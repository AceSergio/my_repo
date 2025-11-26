#define _POSIX_C_SOURCE 200809L

#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void date_log(char *tampon, size_t taille) {
  time_t mtn = time(NULL);
  struct tm *tm = gmtime(&mtn);
  strftime(tampon, taille, "%a, %d %b %Y %H:%M:%S GMT", tm);
}

static const char *methode_vers_str(enum http_method methode) {
  switch (methode) {
  case HTTP_GET:
    return "GET";
  case HTTP_HEAD:
    return "HEAD";
  default:
    return "UNKNOWN";
  }
}

void logger_log_request(const struct http_request *req, const char *ip_cli,
                        const struct config *cfg) {
  char date[128];
  date_log(date, sizeof(date));

  printf("%s [%s] received %s on '%s' from %s\n", date,
         cfg->vhost->server_name->data, methode_vers_str(req->method),
         req->target, ip_cli);

  fflush(stdout);
}

void logger_log_response(const struct http_request *req,
                         const struct http_response *rep, const char *ip_cli,
                         const struct config *cfg) {
  char date[128];
  date_log(date, sizeof(date));

  char code_statut[4] = "000";
  if (rep->status_line && strlen(rep->status_line) >= 12) {
    strncpy(code_statut, rep->status_line + 9, 3);
  }

  printf("%s [%s] responding with %s to %s for %s on '%s'\n", date,
         cfg->vhost->server_name->data, code_statut, ip_cli,
         methode_vers_str(req->method), req->target);

  fflush(stdout);
}

void logger_log_bad_request(const char *ip_cli,
                            const struct config *cfg) {
  char date[128];
  date_log(date, sizeof(date));

  printf("%s [%s] received Bad Request from %s\n", date,
         cfg->vhost->server_name->data, ip_cli);

  printf("%s [%s] responding with 400 to %s\n", date,
         cfg->vhost->server_name->data, ip_cli);

  fflush(stdout);
}
