#define _POSIX_C_SOURCE 200809L
#include "runtime.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

FLValue fxb_net_listen_address(FLValue, FLValue);
FLValue fxb_net_accept_timeout(FLValue, FLValue);
FLValue fxb_net_connect_timeout(FLValue, FLValue, FLValue);
FLValue fxb_net_read_timeout(FLValue, FLValue, FLValue);
FLValue fxb_net_write_timeout(FLValue, FLValue, FLValue);
FLValue fxb_net_close(FLValue);
int fl_net_test_generation(int, unsigned long long *);
int fl_net_test_lookup(int, unsigned long long);
int fl_net_test_close_generation(int, unsigned long long);
void fl_net_test_counts(int *, int *, int *, int *, unsigned long long *, unsigned long long *);
FLValue fl_sleep_ms(FLValue v) { struct timespec t = {(time_t)(v.i / 1000), (long)(v.i % 1000) * 1000000L}; nanosleep(&t, NULL); return fl_nil(); }

static int listener_fd, server_fd;
static unsigned long long listener_gen, server_gen, client_gen;
static void *server_main(void *unused) {
    (void)unused;
    FLValue r = fxb_net_accept_timeout(fl_int(listener_fd), fl_int(3000));
    assert(fl_map_get(r, fl_str_val("status")).obj != NULL);
    assert(fl_map_get(r, fl_str_val("status")).tag == FL_STRING);
    server_fd = (int)fl_map_get(r, fl_str_val("fd")).i;
    assert(fl_net_test_generation(server_fd, &server_gen));
    FLValue x = fxb_net_read_timeout(fl_int(server_fd), fl_int(32), fl_int(3000));
    assert(fl_map_get(x, fl_str_val("status")).tag == FL_STRING);
    assert(((FLString*)fl_map_get(x, fl_str_val("data")).obj)->len == 4);
    FLValue y = fxb_net_write_timeout(fl_int(server_fd), fl_str_val("pong"), fl_int(3000));
    assert(fl_map_get(y, fl_str_val("bytes")).i == 4);
    int e,l,i,o; unsigned long long c,z;
    fl_net_test_counts(&e,&l,&i,&o,&c,&z);
    assert(e == 3 && l == 1 && i == 1 && o == 1);
    puts("LISTENER_INBOUND_OUTBOUND=PASS");
    fxb_net_close(fl_int(server_fd));
    return NULL;
}
struct close_arg { int fd; unsigned long long gen; int result; };
static void *close_worker(void *p) {
    struct close_arg *a = (struct close_arg*)p;
    a->result = fl_net_test_close_generation(a->fd, a->gen);
    return NULL;
}
int main(void) {
    listener_fd = (int)fxb_net_listen_address(fl_str_val("127.0.0.1"), fl_int(19253)).i;
    assert(listener_fd > 0);
    assert(fl_net_test_generation(listener_fd, &listener_gen));
    pthread_t st; assert(!pthread_create(&st, NULL, server_main, NULL));
    struct timespec pause_time = {0, 50000000}; nanosleep(&pause_time, NULL);
    FLValue c = fxb_net_connect_timeout(fl_str_val("127.0.0.1"), fl_int(19253), fl_int(3000));
    int client_fd = (int)fl_map_get(c, fl_str_val("fd")).i;
    assert(client_fd > 0 && fl_net_test_generation(client_fd, &client_gen));
    FLValue w = fxb_net_write_timeout(fl_int(client_fd), fl_str_val("ping"), fl_int(3000));
    assert(fl_map_get(w, fl_str_val("bytes")).i == 4);
    FLValue r = fxb_net_read_timeout(fl_int(client_fd), fl_int(32), fl_int(3000));
    assert(((FLString*)fl_map_get(r, fl_str_val("data")).obj)->len == 4);
    pthread_join(st, NULL);
    int before_entries,before_l,before_i,before_o; unsigned long long before_claims,before_closes;
    fl_net_test_counts(&before_entries,&before_l,&before_i,&before_o,&before_claims,&before_closes);
    pthread_t ts[100]; struct close_arg args[100];
    for (int i=0;i<100;i++) { args[i]=(struct close_arg){client_fd,client_gen,0}; assert(!pthread_create(&ts[i],NULL,close_worker,&args[i])); }
    for (int i=0;i<100;i++) pthread_join(ts[i],NULL);
    int claims=0; for(int i=0;i<100;i++) if(args[i].result==1) claims++;
    int after_entries,after_l,after_i,after_o; unsigned long long after_claims,after_closes;
    fl_net_test_counts(&after_entries,&after_l,&after_i,&after_o,&after_claims,&after_closes);
    assert(claims == 1 && after_claims == before_claims + 1 && after_closes == before_closes + 1 && !fl_net_test_lookup(client_fd, client_gen));
    puts("CONCURRENT_CLOSE_100=PASS");
    puts("OS_CLOSE_EXACTLY_ONCE=PASS");
    puts("STALE_GENERATION_REJECT=PASS");
    assert(fl_net_test_close_generation(listener_fd, listener_gen) == 1);
    int replacement=(int)fxb_net_listen_address(fl_str_val("127.0.0.1"), fl_int(19253)).i;
    unsigned long long replacement_gen; assert(replacement > 0 && fl_net_test_generation(replacement,&replacement_gen));
    assert(replacement_gen > listener_gen && !fl_net_test_lookup(listener_fd,listener_gen));
    assert(fl_net_test_close_generation(replacement,replacement_gen) == 1);
    int e,l,i,o; unsigned long long ccount,zcount;
    fl_net_test_counts(&e,&l,&i,&o,&ccount,&zcount);
    assert(e == 0 && l == 0 && i == 0 && o == 0);
    puts("GENERATION_INCREMENT=PASS");
    puts("REGISTRY_EMPTY=PASS");
    puts("PRODUCTION_HARNESS=PASS");
    return 0;
}
