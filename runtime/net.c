/*
 * net.c — FreeLang fx2 Raw TCP 소켓 함수
 */
#include "runtime.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <time.h>
#include <limits.h>
#include <pthread.h>

typedef enum { FL_NET_LISTENER, FL_NET_INBOUND, FL_NET_OUTBOUND } FLNetKind;
typedef enum { FL_NET_OPEN, FL_NET_CLOSING, FL_NET_CLOSED } FLNetState;
typedef struct FLNetEntry FLNetEntry;
struct FLNetEntry {
    int fd;
    unsigned long long generation;
    FLNetKind kind;
    FLNetState state;
    unsigned refs;
    int close_claimed;
    FLNetEntry *next;
};
static pthread_mutex_t fl_net_registry_mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t fl_net_registry_cv = PTHREAD_COND_INITIALIZER;
static FLNetEntry *fl_net_registry;
static unsigned long long fl_net_next_generation;
#ifdef FL_NET_TESTING
static unsigned long long fl_net_test_close_claims;
static unsigned long long fl_net_test_os_closes;
#endif

static FLNetEntry *fl_net_find_locked(int fd) {
    for (FLNetEntry *e = fl_net_registry; e; e = e->next)
        if (e->fd == fd && e->state == FL_NET_OPEN) return e;
    return NULL;
}
#ifdef FL_NET_TESTING
static FLNetEntry *fl_net_find_generation_locked(int fd, unsigned long long generation) {
    for (FLNetEntry *e = fl_net_registry; e; e = e->next)
        if (e->fd == fd && e->generation == generation && (e->state == FL_NET_OPEN || e->state == FL_NET_CLOSING)) return e;
    return NULL;
}
#endif
static FLNetEntry *fl_net_find_close_locked(int fd) {
    for (FLNetEntry *e = fl_net_registry; e; e = e->next)
        if (e->fd == fd && (e->state == FL_NET_OPEN || e->state == FL_NET_CLOSING)) return e;
    return NULL;
}
static void fl_net_reap_locked(FLNetEntry *entry) {
    if (entry->state != FL_NET_CLOSED || entry->refs != 0) return;
    FLNetEntry **p = &fl_net_registry;
    while (*p && *p != entry) p = &(*p)->next;
    if (*p == entry) { *p = entry->next; free(entry); }
}
static unsigned long long fl_net_register_fd(int fd, FLNetKind kind) {
    if (fd < 0) return 0;
    FLNetEntry *entry = (FLNetEntry*)calloc(1, sizeof(*entry));
    if (!entry) return 0;
    pthread_mutex_lock(&fl_net_registry_mu);
    entry->fd = fd;
    entry->generation = ++fl_net_next_generation;
    entry->kind = kind;
    entry->state = FL_NET_OPEN;
    entry->refs = 1;
    entry->next = fl_net_registry;
    fl_net_registry = entry;
    pthread_mutex_unlock(&fl_net_registry_mu);
    return entry->generation;
}
static FLNetEntry *fl_net_acquire_fd(int fd) {
    pthread_mutex_lock(&fl_net_registry_mu);
    FLNetEntry *entry = fl_net_find_locked(fd);
    if (entry) entry->refs++;
    pthread_mutex_unlock(&fl_net_registry_mu);
    return entry;
}
static void fl_net_release_fd(FLNetEntry *entry) {
    if (!entry) return;
    pthread_mutex_lock(&fl_net_registry_mu);
    if (entry->refs > 0) entry->refs--;
    pthread_cond_broadcast(&fl_net_registry_cv);
    fl_net_reap_locked(entry);
    pthread_mutex_unlock(&fl_net_registry_mu);
}
static int fl_net_close_once(int fd) {
    pthread_mutex_lock(&fl_net_registry_mu);
    FLNetEntry *entry = fl_net_find_close_locked(fd);
    if (!entry) { pthread_mutex_unlock(&fl_net_registry_mu); return close(fd); }
    if (entry->close_claimed) { pthread_mutex_unlock(&fl_net_registry_mu); return 0; }
    entry->close_claimed = 1;
    entry->state = FL_NET_CLOSING;
#ifdef FL_NET_TESTING
    fl_net_test_close_claims++;
#endif
    while (entry->refs > 1) pthread_cond_wait(&fl_net_registry_cv, &fl_net_registry_mu);
    pthread_mutex_unlock(&fl_net_registry_mu);
    int result = close(fd);
#ifdef FL_NET_TESTING
    pthread_mutex_lock(&fl_net_registry_mu);
    fl_net_test_os_closes++;
    pthread_mutex_unlock(&fl_net_registry_mu);
#endif
    pthread_mutex_lock(&fl_net_registry_mu);
    entry->state = FL_NET_CLOSED;
    if (entry->refs > 0) entry->refs--;
    fl_net_reap_locked(entry);
    pthread_mutex_unlock(&fl_net_registry_mu);
    return result;
}

#ifdef FL_NET_TESTING
int fl_net_test_close_generation(int fd, unsigned long long generation) {
    pthread_mutex_lock(&fl_net_registry_mu);
    FLNetEntry *entry = fl_net_find_generation_locked(fd, generation);
    if (!entry || entry->close_claimed) {
        pthread_mutex_unlock(&fl_net_registry_mu);
        return 0;
    }
    entry->close_claimed = 1;
    entry->state = FL_NET_CLOSING;
    fl_net_test_close_claims++;
    while (entry->refs > 1) pthread_cond_wait(&fl_net_registry_cv, &fl_net_registry_mu);
    pthread_mutex_unlock(&fl_net_registry_mu);
    int result = close(fd);
    pthread_mutex_lock(&fl_net_registry_mu);
    fl_net_test_os_closes++;
    entry->state = FL_NET_CLOSED;
    if (entry->refs > 0) entry->refs--;
    fl_net_reap_locked(entry);
    pthread_mutex_unlock(&fl_net_registry_mu);
    return result == 0 ? 1 : -1;
}

int fl_net_test_generation(int fd, unsigned long long *generation) {
    pthread_mutex_lock(&fl_net_registry_mu);
    FLNetEntry *entry = fl_net_find_locked(fd);
    if (!entry) { pthread_mutex_unlock(&fl_net_registry_mu); return 0; }
    if (generation) *generation = entry->generation;
    pthread_mutex_unlock(&fl_net_registry_mu);
    return 1;
}

int fl_net_test_lookup(int fd, unsigned long long generation) {
    pthread_mutex_lock(&fl_net_registry_mu);
    int found = fl_net_find_generation_locked(fd, generation) != NULL;
    pthread_mutex_unlock(&fl_net_registry_mu);
    return found;
}

int fl_net_test_snapshot(int fd, unsigned long long generation,
                         int *kind, int *state, unsigned *refs) {
    pthread_mutex_lock(&fl_net_registry_mu);
    FLNetEntry *entry = fl_net_find_generation_locked(fd, generation);
    if (!entry) { pthread_mutex_unlock(&fl_net_registry_mu); return 0; }
    if (kind) *kind = (int)entry->kind;
    if (state) *state = (int)entry->state;
    if (refs) *refs = entry->refs;
    pthread_mutex_unlock(&fl_net_registry_mu);
    return 1;
}

void fl_net_test_counts(int *entries, int *listeners, int *inbound, int *outbound,
                        unsigned long long *claims, unsigned long long *closes) {
    int e=0,l=0,i=0,o=0;
    pthread_mutex_lock(&fl_net_registry_mu);
    for (FLNetEntry *p=fl_net_registry; p; p=p->next) {
        e++;
        if (p->kind == FL_NET_LISTENER) l++;
        else if (p->kind == FL_NET_INBOUND) i++;
        else if (p->kind == FL_NET_OUTBOUND) o++;
    }
    if (entries) *entries=e;
    if (listeners) *listeners=l;
    if (inbound) *inbound=i;
    if (outbound) *outbound=o;
    if (claims) *claims=fl_net_test_close_claims;
    if (closes) *closes=fl_net_test_os_closes;
    pthread_mutex_unlock(&fl_net_registry_mu);
}
#endif

static int fl_net_register_or_close(int fd, FLNetKind kind) {
    if (fl_net_register_fd(fd, kind) != 0) return 0;
    close(fd);
    return -1;
}

static const char* fl_to_cstring(FLValue v) {
    if (v.tag == FL_STRING && v.obj) return ((FLString*)v.obj)->data;
    return "";
}

static double fl_to_double(FLValue v) {
    if (v.tag == FL_INT)   return (double)v.i;
    if (v.tag == FL_FLOAT) return v.f;
    return 0.0;
}

/* fxb-net-listen port → fd */
FLValue fxb_net_listen(FLValue port_v) {
    int port = (int)fl_to_double(port_v);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return fl_int(-1);
    if (fl_net_register_or_close(fd, FL_NET_LISTENER) < 0) return fl_int(-1);
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { fl_net_close_once(fd); return fl_int(-1); }
    if (listen(fd, 64) < 0) { fl_net_close_once(fd); return fl_int(-1); }
    return fl_int(fd);
}

/* fxb-net-accept server-fd → {fd ip port} */
FLValue fxb_net_accept(FLValue server_fd_v) {
    int server_fd = (int)fl_to_double(server_fd_v);
    struct sockaddr_in cli = {0};
    socklen_t len = sizeof(cli);
    int client_fd = accept(server_fd, (struct sockaddr*)&cli, &len);
    if (client_fd < 0) return fl_nil();
    if (fl_net_register_or_close(client_fd, FL_NET_INBOUND) < 0) return fl_nil();
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cli.sin_addr, ip, sizeof(ip));
    FLValue map = fl_map_new();
    map = fl_map_set(map, fl_str_val("fd"),   fl_int(client_fd));
    map = fl_map_set(map, fl_str_val("ip"),   fl_str_val(ip));
    map = fl_map_set(map, fl_str_val("port"), fl_int(ntohs(cli.sin_port)));
    return map;
}

/* fxb-net-readline fd → string (한 줄, \r\n 제거) */
FLValue fxb_net_readline(FLValue fd_v) {
    int fd = (int)fl_to_double(fd_v);
    FLNetEntry *guard = fl_net_acquire_fd(fd);
    char buf[4096];
    int i = 0;
    while (i < (int)sizeof(buf) - 1) {
        char c;
        int n = recv(fd, &c, 1, 0);
        if (n <= 0) { if (i == 0) { fl_net_release_fd(guard); return fl_nil(); } break; }
        if (c == '\n') break;
        if (c == '\r') continue;
        buf[i++] = c;
    }
    buf[i] = '\0';
    fl_net_release_fd(guard);
    return fl_str_val(buf);
}

/* fxb-net-read-bytes fd n → string */
FLValue fxb_net_read_bytes(FLValue fd_v, FLValue n_v) {
    int fd = (int)fl_to_double(fd_v);
    int n  = (int)fl_to_double(n_v);
    if (n <= 0 || n > 65536) return fl_str_val("");
    FLNetEntry *guard = fl_net_acquire_fd(fd);
    char* buf = malloc(n + 1);
    if (!buf) { fl_net_release_fd(guard); return fl_nil(); }
    int total = 0;
    while (total < n) {
        int r = recv(fd, buf + total, n - total, 0);
        if (r <= 0) break;
        total += r;
    }
    buf[total] = '\0';
    FLValue result = fl_str_val(buf);
    free(buf);
    fl_net_release_fd(guard);
    return result;
}

/* fxb-net-write fd str → nil */
FLValue fxb_net_write(FLValue fd_v, FLValue str_v) {
    int fd = (int)fl_to_double(fd_v);
    const char* str = fl_to_cstring(str_v);
    if (!str) return fl_nil();
    FLNetEntry *guard = fl_net_acquire_fd(fd);
    size_t len = strlen(str), sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, str + sent, len - sent, MSG_NOSIGNAL);
        if (n < 0) break;
        sent += (size_t)n;
    }
    fl_net_release_fd(guard);
    return fl_nil();
}

/* fxb-net-writeline fd str → nil  (str + \r\n 전송) */
FLValue fxb_net_writeline(FLValue fd_v, FLValue str_v) {
    int fd = (int)fl_to_double(fd_v);
    const char* str = fl_to_cstring(str_v);
    if (!str) return fl_nil();
    FLNetEntry *guard = fl_net_acquire_fd(fd);
    char buf[4096];
    snprintf(buf, sizeof(buf), "%s\r\n", str);
    size_t len = strlen(buf), sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, buf + sent, len - sent, MSG_NOSIGNAL);
        if (n < 0) break;
        sent += (size_t)n;
    }
    fl_net_release_fd(guard);
    return fl_nil();
}

/* fxb-net-close fd → nil */
FLValue fxb_net_close(FLValue fd_v) {
    int fd = (int)fl_to_double(fd_v);
    if (fd >= 0) fl_net_close_once(fd);
    return fl_nil();
}

/* fxb-net-pasv-open → {fd port} (50000-50100 범위) */
FLValue fxb_net_pasv_open(void) {
    for (int port = 50000; port <= 50100; port++) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) continue;
        int opt = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons((uint16_t)port);
        if (fl_net_register_or_close(fd, FL_NET_LISTENER) < 0) continue;
        if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0 && listen(fd, 4) == 0) {
            FLValue map = fl_map_new();
            map = fl_map_set(map, fl_str_val("fd"),   fl_int(fd));
            map = fl_map_set(map, fl_str_val("port"), fl_int(port));
            return map;
        }
        fl_net_close_once(fd);
    }
    return fl_nil();
}

/* fxb-net-pasv-accept pasv-fd → data-fd */
FLValue fxb_net_pasv_accept(FLValue pasv_fd_v) {
    int pasv_fd = (int)fl_to_double(pasv_fd_v);
    struct timeval tv = {10, 0};
    setsockopt(pasv_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    struct sockaddr_in cli = {0};
    socklen_t len = sizeof(cli);
    int data_fd = accept(pasv_fd, (struct sockaddr*)&cli, &len);
    if (data_fd >= 0) fl_net_register_or_close(data_fd, FL_NET_INBOUND);
    fl_net_close_once(pasv_fd);
    return fl_int(data_fd);
}

/* fxb-net-connect host port → fd */
FLValue fxb_net_connect(FLValue host_v, FLValue port_v) {
    const char* host = fl_to_cstring(host_v);
    int port = (int)fl_to_double(port_v);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return fl_int(-1);
    if (fl_net_register_or_close(fd, FL_NET_OUTBOUND) < 0) return fl_int(-1);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    inet_pton(AF_INET, host, &addr.sin_addr);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { fl_net_close_once(fd); return fl_int(-1); }
    return fl_int(fd);
}

/* fxb-net-send-file data-fd path → bytes-sent */
FLValue fxb_net_send_file(FLValue fd_v, FLValue path_v) {
    int data_fd = (int)fl_to_double(fd_v);
    const char* path = fl_to_cstring(path_v);
    FILE* f = fopen(path, "rb");
    if (!f) return fl_int(-1);
    char buf[8192];
    size_t total = 0;
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        size_t sent = 0;
        while (sent < n) {
            ssize_t r = send(data_fd, buf + sent, n - sent, MSG_NOSIGNAL);
            if (r < 0) { fclose(f); return fl_int((int)total); }
            sent += (size_t)r;
            total += (size_t)r;
        }
    }
    fclose(f);
    return fl_int((int)total);
}

/* fxb-net-recv-file data-fd path → bytes-received */
FLValue fxb_net_recv_file(FLValue fd_v, FLValue path_v) {
    int data_fd = (int)fl_to_double(fd_v);
    const char* path = fl_to_cstring(path_v);
    FILE* f = fopen(path, "wb");
    if (!f) return fl_int(-1);
    char buf[8192];
    size_t total = 0;
    ssize_t n;
    while ((n = recv(data_fd, buf, sizeof(buf), 0)) > 0) {
        fwrite(buf, 1, (size_t)n, f);
        total += (size_t)n;
    }
    fclose(f);
    return fl_int((int)total);
}

/* fxb-net-local-ip → string */
FLValue fxb_net_local_ip(void) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return fl_str_val("127.0.0.1");
    if (fl_net_register_or_close(fd, FL_NET_OUTBOUND) < 0) return fl_str_val("127.0.0.1");
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "8.8.8.8", &addr.sin_addr);
    connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    struct sockaddr_in local = {0};
    socklen_t len = sizeof(local);
    getsockname(fd, (struct sockaddr*)&local, &len);
    fl_net_close_once(fd);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &local.sin_addr, ip, sizeof(ip));
    return fl_str_val(ip);
}

#define FL_NET_MAX_TIMEOUT_MS 600000
#define FL_NET_MAX_IO_BYTES (16*1024*1024)
static int nint(FLValue v,int d){return v.tag==FL_INT?(int)v.i:v.tag==FL_FLOAT?(int)v.f:d;}
static int ntmo(FLValue v){int n=nint(v,0);return n<0?-1:n>FL_NET_MAX_TIMEOUT_MS?FL_NET_MAX_TIMEOUT_MS:n;}
static FLValue nres(const char*s,int fd,int b,int e){FLValue m=fl_map_new();m=fl_map_set(m,fl_str_val("status"),fl_str_val(s));m=fl_map_set(m,fl_str_val("fd"),fl_int(fd));m=fl_map_set(m,fl_str_val("bytes"),fl_int(b));m=fl_map_set(m,fl_str_val("error"),fl_int(e));return m;}
static int nnb(int fd,int on){int f=fcntl(fd,F_GETFL,0);if(f<0)return -1;return fcntl(fd,F_SETFL,on?f|O_NONBLOCK:f&~O_NONBLOCK);}
static int npoll(int fd,short e,int t,short*r){struct pollfd p={fd,e,0};int x;do{x=poll(&p,1,t);}while(x<0&&errno==EINTR);if(r)*r=p.revents;return x;}
static FLValue nio(const char*s,int fd,const char*d,int n,int e){FLValue m=nres(s,fd,n,e);m=fl_map_set(m,fl_str_val("data"),fl_str_val_n(d,n>0?n:0));return m;}
FLValue fxb_net_listen_address(FLValue av,FLValue pv){const char*a=fl_to_cstring(av);int port=nint(pv,-1);struct in_addr ia;if(port<0||port>65535||inet_pton(AF_INET,a,&ia)!=1)return fl_int(-1);int f=socket(AF_INET,SOCK_STREAM,0);if(f<0)return fl_int(-1);if(fl_net_register_or_close(f,FL_NET_LISTENER)<0)return fl_int(-1);int o=1;setsockopt(f,SOL_SOCKET,SO_REUSEADDR,&o,sizeof(o));struct sockaddr_in x={0};x.sin_family=AF_INET;x.sin_addr=ia;x.sin_port=htons(port);if(bind(f,(struct sockaddr*)&x,sizeof(x))<0||listen(f,64)<0){fl_net_close_once(f);return fl_int(-1);}return fl_int(f);}
FLValue fxb_net_set_nonblocking(FLValue fv,FLValue ev){int f=nint(fv,-1);return fl_bool(f>=0&&nnb(f,fl_truthy(ev))==0);}
FLValue fxb_net_wait(FLValue fv,FLValue ev,FLValue tv){int f=nint(fv,-1),e=nint(ev,1),t=ntmo(tv);short pe=0,rr;if(e&1)pe|=POLLIN;if(e&2)pe|=POLLOUT;if(f<0||!pe||t<0)return nres("error",f,0,EINVAL);int x=npoll(f,pe,t,&rr);if(!x)return nres("timeout",f,0,0);if(x<0)return nres("error",f,0,errno);if(rr&(POLLERR|POLLNVAL))return nres("error",f,0,EIO);return nres("ready",f,rr,0);}
FLValue fxb_net_accept_timeout(FLValue sv,FLValue tv){int s=nint(sv,-1),t=ntmo(tv);if(s<0||t<0)return nres("error",-1,0,EINVAL);short rr;int x=npoll(s,POLLIN,t,&rr);if(!x)return nres("timeout",-1,0,0);if(x<0)return nres("error",-1,0,errno);struct sockaddr_in c={0};socklen_t l=sizeof(c);int f=accept(s,(struct sockaddr*)&c,&l);if(f<0)return nres((errno==EAGAIN||errno==EWOULDBLOCK)?"timeout":"error",-1,0,errno);if(fl_net_register_or_close(f,FL_NET_INBOUND)<0)return nres("error",-1,0,ENOMEM);if(nnb(f,1)<0){int e=errno;fl_net_close_once(f);return nres("error",-1,0,e);}char ip[INET_ADDRSTRLEN]="";inet_ntop(AF_INET,&c.sin_addr,ip,sizeof(ip));FLValue m=nres("ok",f,0,0);m=fl_map_set(m,fl_str_val("ip"),fl_str_val(ip));return m;}
FLValue fxb_net_connect_timeout(FLValue hv,FLValue pv,FLValue tv){const char*h=fl_to_cstring(hv);int port=nint(pv,-1),t=ntmo(tv);struct in_addr ia;if(port<0||port>65535||t<0||inet_pton(AF_INET,h,&ia)!=1)return nres("error",-1,0,EINVAL);int f=socket(AF_INET,SOCK_STREAM,0);if(f<0)return nres("error",-1,0,errno);if(fl_net_register_or_close(f,FL_NET_OUTBOUND)<0)return nres("error",-1,0,ENOMEM);nnb(f,1);struct sockaddr_in a={0};a.sin_family=AF_INET;a.sin_addr=ia;a.sin_port=htons(port);int x=connect(f,(struct sockaddr*)&a,sizeof(a));if(x<0&&errno!=EINPROGRESS){int e=errno;fl_net_close_once(f);return nres("refused",-1,0,e);}if(x<0){short rr;if(!(x=npoll(f,POLLOUT,t,&rr))){fl_net_close_once(f);return nres("timeout",-1,0,0);}if(x<0){int e=errno;fl_net_close_once(f);return nres("error",-1,0,e);}int e=0;socklen_t l=sizeof(e);getsockopt(f,SOL_SOCKET,SO_ERROR,&e,&l);if(e){fl_net_close_once(f);return nres("refused",-1,0,e);}}return nres("ok",f,0,0);}
FLValue fxb_net_read_timeout(FLValue fv,FLValue nv,FLValue tv){int f=nint(fv,-1),n=nint(nv,0),t=ntmo(tv);if(f<0||n<=0||n>FL_NET_MAX_IO_BYTES||t<0)return nio("error",f,"",0,EINVAL);short rr;int x=npoll(f,POLLIN,t,&rr);if(!x)return nio("timeout",f,"",0,0);if(x<0)return nio("error",f,"",0,errno);char*b=malloc(n);if(!b)return nio("error",f,"",0,ENOMEM);ssize_t z=recv(f,b,n,0);if(z==0){free(b);return nio("eof",f,"",0,0);}if(z<0){int e=errno;free(b);return nio((e==EAGAIN||e==EWOULDBLOCK)?"timeout":"error",f,"",0,e);}FLValue m=nio("ok",f,b,z,0);free(b);return m;}
static int64_t nmono_ms(void) { struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000; }
static FLValue nwrite_bytes_common(FLValue fv, FLValue dv, FLValue lv, FLValue tv, int timed) {
    if (fv.tag != FL_INT || dv.tag != FL_STRING || !dv.obj || lv.tag != FL_INT) return nres("error", -1, 0, EINVAL);
    int64_t fd_i = fv.i; int f = (int)fd_i; int64_t want_i = lv.i; int t = timed ? ntmo(tv) : 0; FLString *str = (FLString*)dv.obj;
    if (fd_i < 0 || fd_i > INT_MAX || want_i < 0 || (uint64_t)want_i > str->len || (uint64_t)want_i > FL_NET_MAX_IO_BYTES || (timed && t < 0)) return nres("error", f, 0, EINVAL);
    FLNetEntry *guard = fl_net_acquire_fd(f); if (!guard) return nres("error", f, 0, EBADF);
    size_t want = (size_t)want_i, sent = 0; int64_t deadline = nmono_ms() + (timed ? t : FL_NET_MAX_TIMEOUT_MS);
    while (sent < want) {
        int wait_ms = timed ? t : 1000;
        if (timed || sent < want) { int64_t left = deadline - nmono_ms(); if (left <= 0) { fl_net_release_fd(guard); return nres("timeout", f, (int)sent, 0); } wait_ms = left > INT_MAX ? INT_MAX : (int)left; }
        int ready = npoll(f, POLLOUT, wait_ms, NULL);
        if (!ready) { fl_net_release_fd(guard); return nres("timeout", f, (int)sent, 0); }
        if (ready < 0) { int e = errno; fl_net_release_fd(guard); return nres("error", f, (int)sent, e); }
        ssize_t n = send(f, str->data + sent, want - sent, MSG_NOSIGNAL);
        if (n > 0) { sent += (size_t)n; continue; }
        if (n < 0 && errno == EINTR) continue;
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) continue;
        int e = n < 0 ? errno : EPIPE; fl_net_release_fd(guard); return nres("error", f, (int)sent, e);
    }
    fl_net_release_fd(guard); return nres("ok", f, (int)sent, 0);
}
FLValue fxb_net_write_bytes(FLValue fv, FLValue dv, FLValue lv) { return nwrite_bytes_common(fv, dv, lv, fl_int(0), 0); }
FLValue fxb_net_write_bytes_timeout(FLValue fv, FLValue dv, FLValue lv, FLValue tv) { return nwrite_bytes_common(fv, dv, lv, tv, 1); }
FLValue fxb_net_write_timeout(FLValue fv,FLValue dv,FLValue tv){int f=nint(fv,-1),t=ntmo(tv);const char*d=fl_to_cstring(dv);size_t n=strlen(d);if(f<0||n>FL_NET_MAX_IO_BYTES||t<0)return nres("error",f,0,EINVAL);short rr;int x=npoll(f,POLLOUT,t,&rr);if(!x)return nres("timeout",f,0,0);if(x<0)return nres("error",f,0,errno);ssize_t z=send(f,d,n,MSG_NOSIGNAL);if(z<0){int e=errno;return nres((e==EAGAIN||e==EWOULDBLOCK)?"timeout":"error",f,0,e);}return nres("ok",f,z,0);}
FLValue fxb_net_set_keepalive(FLValue fv,FLValue ev,FLValue iv,FLValue xv,FLValue cv){int f=nint(fv,-1),on=fl_truthy(ev);if(f<0||setsockopt(f,SOL_SOCKET,SO_KEEPALIVE,&on,sizeof(on))<0)return fl_bool(false);if(on){int i=nint(iv,60),x=nint(xv,10),c=nint(cv,5);if(i<1||x<1||c<1)return fl_bool(false);if(setsockopt(f,IPPROTO_TCP,TCP_KEEPIDLE,&i,4)<0||setsockopt(f,IPPROTO_TCP,TCP_KEEPINTVL,&x,4)<0||setsockopt(f,IPPROTO_TCP,TCP_KEEPCNT,&c,4)<0)return fl_bool(false);}return fl_bool(true);}


typedef struct FLNetTrack{int fd;struct FLNetTrack*next;}FLNetTrack;static pthread_mutex_t fl_nm=PTHREAD_MUTEX_INITIALIZER;static FLNetTrack*fl_nt;
FLValue fxb_net_track_connection(FLValue fv){int f=nint(fv,-1);if(f<0)return fl_bool(false);FLNetTrack*n=malloc(sizeof(*n));if(!n)return fl_bool(false);n->fd=f;pthread_mutex_lock(&fl_nm);n->next=fl_nt;fl_nt=n;pthread_mutex_unlock(&fl_nm);return fl_bool(true);}
FLValue fxb_net_untrack_connection(FLValue fv){int f=nint(fv,-1);pthread_mutex_lock(&fl_nm);FLNetTrack**p=&fl_nt;while(*p&&(*p)->fd!=f)p=&(*p)->next;if(!*p){pthread_mutex_unlock(&fl_nm);return fl_bool(false);}FLNetTrack*n=*p;*p=n->next;free(n);pthread_mutex_unlock(&fl_nm);return fl_bool(true);}
FLValue fxb_net_listener_shutdown(FLValue fv,FLValue tv){int f=nint(fv,-1),t=ntmo(tv);if(f<0||t<0)return nres("error",f,0,EINVAL);shutdown(f,SHUT_RDWR);fl_net_close_once(f);int w=0;for(;;){pthread_mutex_lock(&fl_nm);int active=fl_nt!=NULL;pthread_mutex_unlock(&fl_nm);if(!active)return nres("ok",-1,0,0);if(w>=t)return nres("timeout",-1,0,0);fl_sleep_ms(fl_int(10));w+=10;}}
