#include "runtime.h"
#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct { int fd; int port; pthread_t thread; } Fixture;
static void* serve_once(void* opaque) {
    Fixture* fixture = opaque; int client = accept(fixture->fd, NULL, NULL); char request[512];
    if (client >= 0) { (void)read(client, request, sizeof(request)); const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 11\r\n\r\nfx-async-ok"; (void)write(client, response, strlen(response)); close(client); }
    return NULL;
}
static const char* error_type(FLValue value) { FLValue type = fl_map_get(value, fl_str_val("errorType")); return type.tag == FL_STRING ? ((FLString*)type.obj)->data : ""; }

int main(void) {
    char root[] = "/tmp/fx-async-test-XXXXXX"; assert(mkdtemp(root));
    char source[512], output[512]; snprintf(source, sizeof(source), "%s/source.txt", root); snprintf(output, sizeof(output), "%s/output.txt", root);
    FILE* file = fopen(source, "wb"); assert(file); fputs("native-async", file); fclose(file);
    assert(fl_truthy(async_set_permissions(fl_str_val(root), fl_str_val(root), fl_str_val("127.0.0.1"))));
    FLValue read_future = async_read_file(fl_str_val(source)); FLValue read_result = async_await(read_future);
    assert(read_result.tag == FL_STRING && strcmp(((FLString*)read_result.obj)->data, "native-async") == 0);
    assert(fl_truthy(async_await(async_write_file(fl_str_val(output), fl_str_val("written")))));
    file = fopen(output, "rb"); assert(file); char buffer[32] = {0}; fread(buffer, 1, sizeof(buffer) - 1, file); fclose(file); assert(strcmp(buffer, "written") == 0);

    async_set_permissions(fl_nil(), fl_nil(), fl_nil());
    assert(strcmp(error_type(async_await(async_read_file(fl_str_val(source)))), "PermissionDeniedError") == 0);
    assert(strcmp(error_type(async_await(async_fetch(fl_str_val("http://127.0.0.1/")))), "PermissionDeniedError") == 0);
    async_set_permissions(fl_str_val(root), fl_str_val(root), fl_str_val("127.0.0.1"));
    assert(strcmp(error_type(async_timeout(async_sleep(fl_int(100)), fl_int(5))), "AsyncTimeoutError") == 0);
    FLValue cancelled = async_sleep(fl_int(100)); assert(fl_truthy(async_cancel(cancelled))); assert(strcmp(error_type(async_await(cancelled)), "AsyncCancelledError") == 0);

    Fixture fixture = {.fd = socket(AF_INET, SOCK_STREAM, 0)}; assert(fixture.fd >= 0); struct sockaddr_in address = {.sin_family = AF_INET, .sin_addr.s_addr = htonl(INADDR_LOOPBACK), .sin_port = 0};
    assert(bind(fixture.fd, (struct sockaddr*)&address, sizeof(address)) == 0); assert(listen(fixture.fd, 1) == 0); socklen_t length = sizeof(address); assert(getsockname(fixture.fd, (struct sockaddr*)&address, &length) == 0); fixture.port = ntohs(address.sin_port); assert(pthread_create(&fixture.thread, NULL, serve_once, &fixture) == 0);
    char url[128]; snprintf(url, sizeof(url), "http://127.0.0.1:%d/", fixture.port); FLValue response = async_await(async_fetch(fl_str_val(url))); assert(fl_map_get(response, fl_str_val("status")).i == 200); assert(strcmp(((FLString*)fl_map_get(response, fl_str_val("body")).obj)->data, "fx-async-ok") == 0); pthread_join(fixture.thread, NULL); close(fixture.fd);

    async_set_concurrency(fl_int(2)); FLValue futures[8]; for (int i = 0; i < 8; i++) futures[i] = async_sleep(fl_int(1)); for (int i = 0; i < 8; i++) (void)async_await(futures[i]);
    printf("NATIVE_ASYNC_TESTS=PASS\n"); return 0;
}
