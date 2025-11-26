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

static char *date_actuelle(void) {
  char *tampon = malloc(128);
  time_t mtn = time(NULL);
  struct tm *tm = gmtime(&mtn);
  strftime(tampon, 128, "%a, %d %b %Y %H:%M:%S GMT", tm);
  return tampon;
}

static struct http_response *creer_rep_erreur(const char *statut) {
  struct http_response *rep = calloc(1, sizeof(struct http_response));
  rep->status_line = strdup(statut);
  rep->body_fd = -1;
  rep->content_length = 0;

  char *date = date_actuelle();

  size_t taille_entetes = 256;
  rep->headers = malloc(taille_entetes);
  snprintf(rep->headers, taille_entetes,
           "Date: %s\n"
           "Content-Length: 0\n"
           "Connection: close\n"
           "\n",
           date);

  free(date);
  return rep;
}

struct http_response *http_response_create(const struct http_request *req,
                                           struct config *cfg) {
  if (req->method != HTTP_GET && req->method != HTTP_HEAD) {
    return creer_rep_erreur(STATUS_405);
  }

  size_t len_chemin = strlen(cfg->vhost->root_dir) + strlen(req->target) + 2;
  char *chemin_comp = malloc(len_chemin);
  sprintf(chemin_comp, "%s%s", cfg->vhost->root_dir, req->target);

  struct stat st;
  if (stat(chemin_comp, &st) == -1) {
    free(chemin_comp);
    return creer_rep_erreur(STATUS_404);
  }

  if (S_ISDIR(st.st_mode)) {
    size_t nouv_len = strlen(chemin_comp) + strlen(cfg->vhost->default_file) + 2;
    char *chemin_dossier = chemin_comp;
    chemin_comp = realloc(chemin_dossier, nouv_len);

    if (chemin_comp[strlen(chemin_comp) - 1] != '/') {
      strcat(chemin_comp, "/");
    }
    strcat(chemin_comp, cfg->vhost->default_file);

    if (stat(chemin_comp, &st) == -1) {
      free(chemin_comp);
      return creer_rep_erreur(STATUS_404);
    }
  }

  int fd = open(chemin_comp, O_RDONLY);
  free(chemin_comp);

  if (fd == -1) {
    return creer_rep_erreur(STATUS_403);
  }

  struct http_response *rep = calloc(1, sizeof(struct http_response));
  rep->status_line = strdup(STATUS_200);
  rep->content_length = st.st_size;

  if (req->method == HTTP_GET) {
    rep->body_fd = fd;
  } else {
    close(fd);
    rep->body_fd = -1;
  }

  char *date = date_actuelle();
  char buf_entetes[1024];

  snprintf(buf_entetes, sizeof(buf_entetes),
           "Date: %s\n"
           "Server: HTTPd/1.0\n"
           "Content-Length: %ld\n"
           "Connection: close\n"
           "\n",
           date, rep->content_length);

  rep->headers = strdup(buf_entetes);
  free(date);

  return rep;
}

void http_response_send(int fd_cli, struct http_response *rep) {
  if (!rep)
    return;

  dprintf(fd_cli, "%s\n", rep->status_line);

  if (rep->headers) {
    write(fd_cli, rep->headers, strlen(rep->headers));
  }

  if (rep->body_fd != -1) {
    off_t offset = 0;
    sendfile(fd_cli, rep->body_fd, &offset, rep->content_length);
  }
}

void http_response_destroy(struct http_response *rep) {
  if (!rep)
    return;
  if (rep->status_line)
    free(rep->status_line);
  if (rep->headers)
    free(rep->headers);
  if (rep->body_fd != -1)
    close(rep->body_fd);
  free(rep);
}
