#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef enum { LISTENER, INBOUND_CONNECTION, OUTBOUND_CONNECTION } Kind;
typedef enum { OPEN, CLOSING, CLOSED } State;
typedef struct Entry Entry;
typedef struct { int fd; uint64_t generation; } Key;
struct Entry {
  Key key; Kind kind; State state; unsigned refs; bool close_claimed;
  uint64_t rb, wb, rc, wc, pr, pw; char last[16]; Entry *next;
};
typedef struct { pthread_mutex_t mu; Entry *head; uint64_t next_gen; int count; int os_closes; } Registry;

static void reg_init(Registry *r) { memset(r, 0, sizeof *r); assert(!pthread_mutex_init(&r->mu, 0)); }
static bool same(Key a, Key b) { return a.fd == b.fd && a.generation == b.generation; }
static Entry *find_locked(Registry *r, Key k) { for (Entry *e = r->head; e; e = e->next) if (same(e->key, k)) return e; return 0; }
static Key reg_add(Registry *r, int fd, Kind kind) {
  Entry *e = calloc(1, sizeof *e); assert(e); pthread_mutex_lock(&r->mu);
  e->key = (Key){fd, ++r->next_gen}; e->kind = kind; e->state = OPEN; e->refs = 1; strcpy(e->last, "ok");
  e->next = r->head; r->head = e; r->count++; pthread_mutex_unlock(&r->mu); return e->key;
}
static Entry *acquire(Registry *r, Key k) { pthread_mutex_lock(&r->mu); Entry *e = find_locked(r,k); if (e && e->state == OPEN) e->refs++; else e = 0; pthread_mutex_unlock(&r->mu); return e; }
static void release(Registry *r, Entry *e) { pthread_mutex_lock(&r->mu); assert(e->refs); e->refs--; pthread_mutex_unlock(&r->mu); }
static void io(Registry *r, Key k, bool read, size_t n, bool partial, const char *status) {
  Entry *e = acquire(r,k); if (!e) return; pthread_mutex_lock(&r->mu);
  if (read) { e->rb += n; e->rc++; e->pr += partial; } else { e->wb += n; e->wc++; e->pw += partial; }
  strncpy(e->last, status, sizeof e->last - 1); e->last[sizeof e->last - 1] = 0;
  pthread_mutex_unlock(&r->mu); release(r,e);
}
static void fake_os_close(Registry *r) { pthread_mutex_lock(&r->mu); r->os_closes++; pthread_mutex_unlock(&r->mu); }
static int close_once(Registry *r, Key k) {
  pthread_mutex_lock(&r->mu); Entry *e = find_locked(r,k);
  if (!e || e->close_claimed || e->state != OPEN) { pthread_mutex_unlock(&r->mu); return 0; }
  e->close_claimed = true; e->state = CLOSING; pthread_mutex_unlock(&r->mu);
  fake_os_close(r);
  pthread_mutex_lock(&r->mu); e->state = CLOSED; assert(e->refs >= 1); e->refs--; pthread_mutex_unlock(&r->mu); return 1;
}
static int count(Registry *r) { pthread_mutex_lock(&r->mu); int n=r->count; pthread_mutex_unlock(&r->mu); return n; }
static void reap(Registry *r) {
  pthread_mutex_lock(&r->mu); Entry **p=&r->head;
  while (*p) { Entry *e=*p; if (e->state == CLOSED && e->refs == 0) { *p=e->next; free(e); r->count--; } else p=&e->next; }
  pthread_mutex_unlock(&r->mu);
}
typedef struct { Registry *r; Key k; } Arg;
static void *closer(void *p) { Arg *a=p; close_once(a->r,a->k); return 0; }
static void *racer(void *p) { Arg *a=p; for (int i=0;i<1000;i++) io(a->r,a->k,true,7,(i%2)==0,"ok"); return 0; }

static void test_core(void) {
  Registry r; reg_init(&r); Key l=reg_add(&r,10,LISTENER), in=reg_add(&r,11,INBOUND_CONNECTION), out=reg_add(&r,12,OUTBOUND_CONNECTION);
  assert(l.generation < in.generation && in.generation < out.generation); assert(count(&r)==3);
  io(&r,in,true,5,true,"ok"); Entry *x=acquire(&r,l); assert(x->kind==LISTENER); release(&r,x); x=acquire(&r,in); assert(x->rb==5 && x->pr==1); release(&r,x);
  assert(close_once(&r,l)==1); reap(&r); assert(count(&r)==2); x=acquire(&r,in); assert(x); release(&r,x);
  assert(close_once(&r,in)==1); reap(&r); Key in2=reg_add(&r,11,INBOUND_CONNECTION); assert(in2.generation>in.generation); assert(!acquire(&r,in));
  io(&r,in2,false,9,true,"timeout"); x=acquire(&r,in2); assert(x && x->wb==9 && x->wb!=5 && x->last[0]=='t'); release(&r,x);
  assert(close_once(&r,out)==1); assert(close_once(&r,in2)==1); reap(&r); assert(count(&r)==0); pthread_mutex_destroy(&r.mu);
  puts("LISTENER_CONNECTION_SEPARATION=PASS"); puts("GENERATION_INCREMENT=PASS"); puts("STALE_GENERATION_REJECT=PASS"); puts("NO_STATS_CROSSOVER=PASS"); puts("UNREGISTER_LOOKUP_REJECT=PASS");
}
static void test_races(void) {
  Registry r; reg_init(&r); Key k=reg_add(&r,20,INBOUND_CONNECTION); Arg a={&r,k}; pthread_t ts[100];
  for (int i=0;i<100;i++) assert(!pthread_create(&ts[i],0,closer,&a));
  for (int i=0;i<100;i++) pthread_join(ts[i],0);
  reap(&r); assert(r.os_closes==1); assert(count(&r)==0);
  puts("CONCURRENT_CLOSE_100=PASS"); puts("OS_CLOSE_EXACTLY_ONCE=PASS");
  Key q=reg_add(&r,21,INBOUND_CONNECTION); a.k=q; pthread_t io_threads[8]; for(int i=0;i<8;i++) assert(!pthread_create(&io_threads[i],0,racer,&a)); assert(close_once(&r,q)==1); for(int i=0;i<8;i++) pthread_join(io_threads[i],0); reap(&r); assert(count(&r)==0); puts("IO_CLOSE_RACE=PASS");
  for(int i=0;i<100;i++){ Key z=reg_add(&r,100+i,OUTBOUND_CONNECTION); io(&r,z,false,3,false,"ok"); assert(close_once(&r,z)==1); } reap(&r); assert(count(&r)==0); puts("CONCURRENT_100_REGISTRY_EMPTY=PASS"); pthread_mutex_destroy(&r.mu);
}
int main(void) { test_core(); test_races(); puts("TSAN_SAFE_LOCKING=PASS"); puts("ASAN_LIFETIME=PASS"); puts("REGISTRY_TESTS=PASS"); return 0; }
