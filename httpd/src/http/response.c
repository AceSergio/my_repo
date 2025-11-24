#define _POSIX_C_SOURCE 200809L

#include "response.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define STATUS_200 "HTTP/1.1 200 OK"
#define STATUS_400 "HTTP/1.1 400 Bad Request"
#define STATUS_403 "HTTP/1.1 403 Forbidden"
#define STATUS_404 "HTTP/1.1 404 Not Found"
#define STATUS_405 "HTTP/1.1 405 Method Not Allowed"

static char *get_current_date(void)
{
    char *buf = malloc(128);
    time_t now = time(NULL);
    struct tm *tm = gmtime(&now);
    strftime(buf, 128, "%a, %d %b %Y %H:%M:%S GMT", tm);
    return buf;
}

static struct http_response *create_error_response(const char *status)
{
    struct http_response *res = calloc(1, sizeof(struct http_response));
    res->status_line = strdup(status);
    res->body_fd = -1;
    res->content_length = 0;

    char *date = get_current_date();

    size_t headers_size = 256;
    res->headers = malloc(headers_size);
    snprintf(res->headers, headers_size,
             "Date: %s\r\n"
             "Content-Length: 0\r\n"
             "Connection: close\r\n"
             "\r\n",
             date);

    free(date);
    return res;
}

struct http_response *http_response_create(const struct http_request *req,
                                           struct config *conf)
{
    if (req->method != HTTP_GET && req->method != HTTP_HEAD)
    {
        return create_error_response(STATUS_405);
    }

    size_t path_len = strlen(conf->vhost->root_dir) + strlen(req->target) + 2;
    char *full_path = malloc(path_len);
    sprintf(full_path, "%s%s", conf->vhost->root_dir, req->target);

    struct stat st;
    if (stat(full_path, &st) == -1)
    {
        free(full_path);
        return create_error_response(STATUS_404);
    }

    if (S_ISDIR(st.st_mode))
    {
        size_t new_len =
            strlen(full_path) + strlen(conf->vhost->default_file) + 2;
        char *dir_path = full_path;
        full_path = realloc(dir_path, new_len);

        if (full_path[strlen(full_path) - 1] != '/')
        {
            strcat(full_path, "/");
        }
        strcat(full_path, conf->vhost->default_file);

        if (stat(full_path, &st) == -1)
        {
            free(full_path);
            return create_error_response(STATUS_404);
        }
    }

    int fd = open(full_path, O_RDONLY);
    free(full_path);

    if (fd == -1)
    {
        return create_error_response(STATUS_403);
    }

    struct http_response *res = calloc(1, sizeof(struct http_response));
    res->status_line = strdup(STATUS_200);
    res->content_length = st.st_size;

    if (req->method == HTTP_GET)
    {
        res->body_fd = fd;
    }
    else
    {
        close(fd);
        res->body_fd = -1;
    }

    char *date = get_current_date();
    char headers_buf[1024];

    snprintf(headers_buf, sizeof(headers_buf),
             "Date: %s\r\n"
             "Server: HTTPd/1.0\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n"
             "\r\n",
             date, res->content_length);

    res->headers = strdup(headers_buf);
    free(date);

    return res;
}

void http_response_send(int client_fd, struct http_response *res)
{
    if (!res)
        return;

    dprintf(client_fd, "%s\r\n", res->status_line);

    if (res->headers)
    {
        write(client_fd, res->headers, strlen(res->headers));
    }

    if (res->body_fd != -1)
    {
        off_t offset = 0;
        sendfile(client_fd, res->body_fd, &offset, res->content_length);
    }
}

void http_response_destroy(struct http_response *res)
{
    if (!res)
        return;
    if (res->status_line)
        free(res->status_line);
    if (res->headers)
        free(res->headers);
    if (res->body_fd != -1)
        close(res->body_fd);
    free(res);
}
