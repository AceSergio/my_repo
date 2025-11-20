#define _POSIX_C_SOURCE 200809L

#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Helper pour récupérer la date au format log (GMT)
static void get_log_date(char *buf, size_t size)
{
    time_t now = time(NULL);
    struct tm *tm = gmtime(&now);
    // Format exemple : Sun, 06 Nov 1994 08:49:37 GMT
    strftime(buf, size, "%a, %d %b %Y %H:%M:%S GMT", tm);
}

// Helper pour convertir l'enum méthode en chaîne
static const char *method_to_str(enum http_method method)
{
    switch (method)
    {
    case HTTP_GET:
        return "GET";
    case HTTP_HEAD:
        return "HEAD";
    default:
        return "UNKNOWN";
    }
}

void logger_log_request(const struct http_request *req, const char *client_ip,
                        const struct config *config)
{
    char date[128];
    get_log_date(date, sizeof(date));

    // Format: DATE [SERV_NAME] received REQUEST_TYPE on 'TARGET' from CLIENT_IP
    printf("%s [%s] received %s on '%s' from %s\n", date,
           config->vhost->server_name
               ->data, // Attention: server_name est une struct string*
           method_to_str(req->method), req->target, client_ip);

    fflush(stdout); // Important pour que le log s'écrive immédiatement
}

void logger_log_response(const struct http_request *req,
                         const struct http_response *res, const char *client_ip,
                         const struct config *config)
{
    char date[128];
    get_log_date(date, sizeof(date));

    // Extraction du code statut (ex: "200" de "HTTP/1.1 200 OK")
    // On suppose que status_line commence par "HTTP/1.1 "
    char status_code[4] = "000";
    if (res->status_line && strlen(res->status_line) >= 12)
    {
        strncpy(status_code, res->status_line + 9, 3);
    }

    // Format: DATE [SERV_NAME] responding with STATUS_CODE to CLIENT_IP for
    // REQUEST_TYPE on 'TARGET'
    printf("%s [%s] responding with %s to %s for %s on '%s'\n", date,
           config->vhost->server_name->data, status_code, client_ip,
           method_to_str(req->method), req->target);

    fflush(stdout);
}

void logger_log_bad_request(const char *client_ip, const struct config *config)
{
    char date[128];
    get_log_date(date, sizeof(date));

    // Cas spécifique 400 Bad Request
    // Format: DATE [SERV_NAME] received Bad Request from CLIENT_IP
    printf("%s [%s] received Bad Request from %s\n", date,
           config->vhost->server_name->data, client_ip);

    // On log aussi la réponse 400
    printf("%s [%s] responding with 400 to %s\n", date,
           config->vhost->server_name->data, client_ip);

    fflush(stdout);
}
