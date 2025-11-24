#ifndef REQUEST_H
#define REQUEST_H

#include <stdbool.h>
#include <stddef.h>

enum http_method
{
    HTTP_GET,
    HTTP_HEAD,
    HTTP_UNKNOWN
};

struct http_request
{
    enum http_method method;
    char *target;
    char *version;
    char *header_host;
    size_t content_length;
    bool has_content_length;
};

struct http_request *http_request_parse(const char *raw_request);

void http_request_free(struct http_request *req);

#endif /* ! REQUEST_H */
