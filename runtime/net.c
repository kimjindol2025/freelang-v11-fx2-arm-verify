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
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(fd); return fl_int(-1); }
    if (listen(fd, 64) < 0) { close(fd); return fl_int(-1); }
    return fl_int(fd);
}

/* fxb-net-accept server-fd → {fd ip port} */
FLValue fxb_net_accept(FLValue server_fd_v) {
    int server_fd = (int)fl_to_double(server_fd_v);
    struct sockaddr_in cli = {0};
    socklen_t len = sizeof(cli);
    int client_fd = accept(server_fd, (struct sockaddr*)&cli, &len);
    if (client_fd < 0) return fl_nil();
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
    char buf[4096];
    int i = 0;
    while (i < (int)sizeof(buf) - 1) {
        char c;
        int n = recv(fd, &c, 1, 0);
        if (n <= 0) { if (i == 0) return fl_nil(); break; }
        if (c == '\n') break;
        if (c == '\r') continue;
        buf[i++] = c;
    }
    buf[i] = '\0';
    return fl_str_val(buf);
}

/* fxb-net-read-bytes fd n → string */
FLValue fxb_net_read_bytes(FLValue fd_v, FLValue n_v) {
    int fd = (int)fl_to_double(fd_v);
    int n  = (int)fl_to_double(n_v);
    if (n <= 0 || n > 65536) return fl_str_val("");
    char* buf = malloc(n + 1);
    if (!buf) return fl_nil();
    int total = 0;
    while (total < n) {
        int r = recv(fd, buf + total, n - total, 0);
        if (r <= 0) break;
        total += r;
    }
    buf[total] = '\0';
    FLValue result = fl_str_val(buf);
    free(buf);
    return result;
}

/* fxb-net-write fd str → nil */
FLValue fxb_net_write(FLValue fd_v, FLValue str_v) {
    int fd = (int)fl_to_double(fd_v);
    const char* str = fl_to_cstring(str_v);
    if (!str) return fl_nil();
    size_t len = strlen(str), sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, str + sent, len - sent, MSG_NOSIGNAL);
        if (n < 0) break;
        sent += (size_t)n;
    }
    return fl_nil();
}

/* fxb-net-writeline fd str → nil  (str + \r\n 전송) */
FLValue fxb_net_writeline(FLValue fd_v, FLValue str_v) {
    int fd = (int)fl_to_double(fd_v);
    const char* str = fl_to_cstring(str_v);
    if (!str) return fl_nil();
    char buf[4096];
    snprintf(buf, sizeof(buf), "%s\r\n", str);
    size_t len = strlen(buf), sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, buf + sent, len - sent, MSG_NOSIGNAL);
        if (n < 0) break;
        sent += (size_t)n;
    }
    return fl_nil();
}

/* fxb-net-close fd → nil */
FLValue fxb_net_close(FLValue fd_v) {
    int fd = (int)fl_to_double(fd_v);
    if (fd >= 0) close(fd);
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
        if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0 && listen(fd, 4) == 0) {
            FLValue map = fl_map_new();
            map = fl_map_set(map, fl_str_val("fd"),   fl_int(fd));
            map = fl_map_set(map, fl_str_val("port"), fl_int(port));
            return map;
        }
        close(fd);
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
    close(pasv_fd);
    return fl_int(data_fd);
}

/* fxb-net-connect host port → fd */
FLValue fxb_net_connect(FLValue host_v, FLValue port_v) {
    const char* host = fl_to_cstring(host_v);
    int port = (int)fl_to_double(port_v);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return fl_int(-1);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    inet_pton(AF_INET, host, &addr.sin_addr);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(fd); return fl_int(-1); }
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
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "8.8.8.8", &addr.sin_addr);
    connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    struct sockaddr_in local = {0};
    socklen_t len = sizeof(local);
    getsockname(fd, (struct sockaddr*)&local, &len);
    close(fd);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &local.sin_addr, ip, sizeof(ip));
    return fl_str_val(ip);
}
