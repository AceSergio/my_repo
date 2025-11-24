#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http/request.h"

void test_simple_get(void)
{
    printf("[TEST] Simple GET... ");
    const char *raw = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
    struct http_request *req = http_request_parse(raw);

    assert(req != NULL);
    assert(req->method == HTTP_GET);
    assert(strcmp(req->target, "/index.html") == 0);
    assert(strcmp(req->version, "HTTP/1.1") == 0);
    assert(strcmp(req->header_host, "localhost") == 0);

    http_request_free(req);
    printf("OK\n");
}

void test_malformed(void)
{
    printf("[TEST] Malformed Request... ");
    const char *raw = "NOT_HTTP_AT_ALL";
    struct http_request *req = http_request_parse(raw);
    assert(req == NULL); // Doit retourner NULL, pas crash
    printf("OK\n");
}

void test_headers_parsing(void)
{
    printf("[TEST] Header Parsing... ");
    const char *raw =
        "GET / HTTP/1.1\r\nHost: epita.fr\r\nContent-Length: 42\r\n\r\n";
    struct http_request *req = http_request_parse(raw);

    assert(req != NULL);
    assert(strcmp(req->header_host, "epita.fr") == 0);
    assert(req->content_length == 42);

    http_request_free(req);
    printf("OK\n");
}

int main(void)
{
    printf("=== HTTP SUITE ===\n");
    test_simple_get();
    test_malformed();
    test_headers_parsing();
    return 0;
}
