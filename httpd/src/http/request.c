#define _POSIX_C_SOURCE 200809L
#include "request.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static enum http_method parse_method(const char *str)
{
    if (strcmp(str, "GET") == 0)
        return HTTP_GET;
    if (strcmp(str, "HEAD") == 0)
        return HTTP_HEAD;
    return HTTP_UNKNOWN;
}

static char *trim_whitespace(char *str)
{
    char *end;

    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0)
        return str;

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    *(end + 1) = 0;
    return str;
}

void http_request_free(struct http_request *req)
{
    if (!req)
        return;
    free(req->target);
    free(req->version);
    free(req->header_host);
    free(req);
}

struct http_request *http_request_parse(const char *raw_request)
{
    if (!raw_request)
        return NULL;

    char *request_copy = strdup(raw_request);
    if (!request_copy)
        return NULL;

    struct http_request *req = calloc(1, sizeof(struct http_request));
    if (!req)
    {
        free(request_copy);
        return NULL;
    }

    char *saveptr_lines;
    char *line = strtok_r(request_copy, "\r\n", &saveptr_lines);

    if (!line)
    {
        free(request_copy);
        http_request_free(req);
        return NULL;
    }

    char *saveptr_words;
    char *method_str = strtok_r(line, " ", &saveptr_words);
    char *target_str = strtok_r(NULL, " ", &saveptr_words);
    char *version_str = strtok_r(NULL, " ", &saveptr_words);

    if (!method_str || !target_str || !version_str)
    {
        free(request_copy);
        http_request_free(req);
        return NULL;
    }

    req->method = parse_method(method_str);
    req->target = strdup(target_str);
    req->version = strdup(version_str);

    while ((line = strtok_r(NULL, "\r\n", &saveptr_lines)) != NULL)
    {
        char *colon = strchr(line, ':');
        if (colon)
        {
            *colon = '\0';
            char *key = line;
            char *value = colon + 1;

            value = trim_whitespace(value);

            if (strcasecmp(key, "Host") == 0)
            {
                if (req->header_host)
                    free(req->header_host);
                req->header_host = strdup(value);
            }
            else if (strcasecmp(key, "Content-Length") == 0)
            {
                req->content_length = strtoul(value, NULL, 10);
                req->has_content_length = true;
            }
        }
    }

    free(request_copy);
    return req;
}
