#define _POSIX_C_SOURCE 200809L

#include "server.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "http/request.h"
#include "http/response.h"
#include "logger/logger.h"

static void init_sig(void) {
  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGPIPE, &sa, NULL);
}

static int creer_lier(const char *ip, const char *port) {
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int sfd;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int s = getaddrinfo(ip, port, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd == -1) {
      continue;
    }

    int optval = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
      break;
    }
    close(sfd);
  }

  freeaddrinfo(result);

  if (rp == NULL) {
    fprintf(stderr, "Could not bind\n");
    return -1;
  }

  return sfd;
}

static int is_host_valid(const struct http_request *req,
                         const struct config *config) {
  if (!req->header_host) {
    return 0;
  }

  if (strcmp(req->header_host, config->vhost->server_name->data) == 0) {
    return 1;
  }

  if (strcmp(req->header_host, config->vhost->ip) == 0) {
    return 1;
  }

  char ip_port[128];
  snprintf(ip_port, sizeof(ip_port), "%s:%s", config->vhost->ip,
           config->vhost->port);
  if (strcmp(req->header_host, ip_port) == 0)
    return 1;

  return 0;
}

static int is_version_valid(const struct http_request *req) {
  if (req->version && strcmp(req->version, "HTTP/1.1") == 0) {
    return 1;
  }
  return 0;
}

static void gerer_client(int client_fd, const char *client_ip,
                          struct config *config) {
  char buffer[2048];
  ssize_t lu = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

  if (lu > 0) {
    buffer[lu] = '\0';

    struct http_request *req = http_request_parse(buffer);

    if (req) {
      logger_log_request(req, client_ip, config);

      if (!is_host_valid(req, config)) {
        logger_log_bad_request(client_ip, config);

        char *err = "HTTP/1.1 400 Bad Request\nContent-Length: "
                    "0\nConnection: close\n\n";
        send(client_fd, err, strlen(err), 0);

        http_request_free(req);
        close(client_fd);
        return;
      }

      if (!is_version_valid(req)) {
        char *err = "HTTP/1.1 505 HTTP Version Not "
                    "Supported\nContent-Length: 0\nConnection: "
                    "close\n\n";
        send(client_fd, err, strlen(err), 0);

        printf("Responding with 505 to %s\n", client_ip);

        http_request_free(req);
        close(client_fd);
        return;
      }

      struct http_response *res = http_response_create(req, config);
      http_response_send(client_fd, res);
      logger_log_response(req, res, client_ip, config);
      http_response_destroy(res);
      http_request_free(req);
    } else {
      logger_log_bad_request(client_ip, config);

      char *err = "HTTP/1.1 400 Bad Request\nContent-Length: "
                  "0\nConnection: close\n\n";
      send(client_fd, err, strlen(err), 0);
    }
  }
  close(client_fd);
}

static volatile sig_atomic_t actif = 1;

static void handle_sigint(int sig) {
  if(!sig)
  {
    return;
  }
  actif = 0;
}

int server_start(struct config *config) {
  init_sig();

  struct sigaction sa;
  sa.sa_handler = handle_sigint;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction SIGINT");
    return 1;
  }

  int sfd = creer_lier(config->vhost->ip, config->vhost->port);
  if (sfd == -1)
    return 1;

  if (listen(sfd, SOMAXCONN) == -1) {
    perror("listen");
    close(sfd);
    return 1;
  }

  printf("Server listening on %s:%s\n", config->vhost->ip, config->vhost->port);

  while (actif) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd =
        accept(sfd, (struct sockaddr *)&client_addr, &client_addr_len);

    if (client_fd == -1) {
      if (errno == EINTR)
        continue;
      perror("accept");
      continue;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);

    gerer_client(client_fd, client_ip, config);
  }

  printf("\nServer shutting down...\n");
  close(sfd);
  return 0;
}
