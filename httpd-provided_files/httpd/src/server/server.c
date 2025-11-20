#define _POSIX_C_SOURCE 200809L // Nécessaire pour getaddrinfo, sigaction

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

// Fonction pour ignorer le signal SIGPIPE (recommandé par le sujet 3.2.1)
static void setup_sigpipe(void)
{
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGPIPE, &sa, NULL);
}

// Crée et configure le socket d'écoute
static int create_and_bind(const char *ip, const char *port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // IPv4 seulement (imposé par le sujet)
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE; // Pour utiliser l'IP locale si ip est NULL

    int s = getaddrinfo(ip, port, &hints, &result);
    if (s != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }

    // Tenter de se lier à l'une des adresses renvoyées
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        // Option pour réutiliser le port immédiatement après arrêt (évite
        // "Address already in use")
        int optval = 1;
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break; // Succès

        close(sfd);
    }

    freeaddrinfo(result);

    if (rp == NULL)
    {
        fprintf(stderr, "Could not bind\n");
        return -1;
    }

    return sfd;
}

static int is_host_valid(const struct http_request *req,
                         const struct config *config)
{
    if (!req->header_host)
    {
        return 0; // Host manquant = Invalide en HTTP/1.1
    }

    // Vérification basique : est-ce que le Host correspond au server_name ?
    // Le sujet dit : match server_name, IP, ou IP:port.

    // 1. Match server_name
    if (strcmp(req->header_host, config->vhost->server_name->data) == 0)
        return 1;

    // 2. Match IP
    if (strcmp(req->header_host, config->vhost->ip) == 0)
        return 1;

    // 3. Match IP:PORT (ex: 127.0.0.1:8080)
    // On construit la chaîne IP:PORT attendue pour comparer
    char ip_port[128];
    snprintf(ip_port, sizeof(ip_port), "%s:%s", config->vhost->ip,
             config->vhost->port);
    if (strcmp(req->header_host, ip_port) == 0)
        return 1;

    return 0; // Aucun match
}

static int is_version_valid(const struct http_request *req)
{
    // On ne supporte QUE "HTTP/1.1"
    if (req->version && strcmp(req->version, "HTTP/1.1") == 0)
    {
        return 1;
    }
    return 0;
}

static void handle_client(int client_fd, const char *client_ip,
                          struct config *config)
{
    char buffer[2048];
    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';

        struct http_request *req = http_request_parse(buffer);

        if (req)
        {
            // 1. Log de la requête REÇUE
            logger_log_request(req, client_ip, config);

            if (!is_host_valid(req, config))
            {
                // Si Host invalide -> 400 Bad Request direct
                logger_log_bad_request(client_ip, config);

                char *err = "HTTP/1.1 400 Bad Request\r\nContent-Length: "
                            "0\r\nConnection: close\r\n\r\n";
                send(client_fd, err, strlen(err), 0);

                http_request_free(req);
                close(client_fd);
                return; // On arrête là
            }

            if (!is_version_valid(req))
            {
                // Erreur 505
                char *err = "HTTP/1.1 505 HTTP Version Not "
                            "Supported\r\nContent-Length: 0\r\nConnection: "
                            "close\r\n\r\n";
                send(client_fd, err, strlen(err), 0);

                // On loggue l'erreur (on peut réutiliser logger_log_response
                // avec un faux objet response si on veut être puriste, ou faire
                // un log manuel ici).
                printf("Responding with 505 to %s\n", client_ip); // Log minimal

                http_request_free(req);
                close(client_fd);
                return;
            }

            struct http_response *res = http_response_create(req, config);

            // 2. Envoi de la réponse
            http_response_send(client_fd, res);

            // 3. Log de la réponse ENVOYÉE
            logger_log_response(req, res, client_ip, config);

            http_response_destroy(res);
            http_request_free(req);
        }
        else
        {
            // Cas 400 Bad Request (Parsing échoué)
            logger_log_bad_request(client_ip, config);

            char *err = "HTTP/1.1 400 Bad Request\r\nContent-Length: "
                        "0\r\nConnection: close\r\n\r\n";
            send(client_fd, err, strlen(err), 0);
        }
    }
    close(client_fd);
}

// Variable globale pour contrôler la boucle principale
static volatile sig_atomic_t server_running = 1;

// Handler pour SIGINT (CTRL+C ou kill -INT)
static void handle_sigint(int sig)
{
    (void)sig;
    server_running = 0; // On demande l'arrêt
}

int server_start(struct config *config)
{
    setup_sigpipe();

    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    // On intercepte SIGINT
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction SIGINT");
        return 1;
    }

    // 1. Création du socket
    int sfd = create_and_bind(config->vhost->ip, config->vhost->port);
    if (sfd == -1)
        return 1;

    // 2. Mise en écoute (Listen)
    // SOMAXCONN est la taille max de la file d'attente
    if (listen(sfd, SOMAXCONN) == -1)
    {
        perror("listen");
        close(sfd);
        return 1;
    }

    printf("Server listening on %s:%s\n", config->vhost->ip,
           config->vhost->port);

    // 3. Boucle d'acceptation
    while (server_running)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd =
            accept(sfd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (client_fd == -1)
        {
            if (errno == EINTR)
                continue;
            perror("accept");
            continue;
        }

        // Récupération propre de l'IP pour le logger
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);

        // Appel mis à jour
        handle_client(client_fd, client_ip, config);
    }

    printf("\nServer shutting down...\n");
    close(sfd);
    return 0;
}
