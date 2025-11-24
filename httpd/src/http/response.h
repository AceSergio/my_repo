#ifndef RESPONSE_H
#define RESPONSE_H

#include "config/config.h"
#include "request.h"

struct http_response
{
    char *status_line;
    char *headers;
    int body_fd;
    size_t content_length;
};


struct http_response *http_response_create(const struct http_request *req,
                                           struct config *conf);

void http_response_send(int client_fd, struct http_response *res);
void http_response_destroy(struct http_response *res);

#endif /* ! RESPONSE_H */
