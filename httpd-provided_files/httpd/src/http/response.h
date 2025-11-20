#ifndef RESPONSE_H
#define RESPONSE_H

#include "config/config.h"
#include "request.h"

struct http_response
{
    char *status_line; // Ex: "HTTP/1.1 200 OK"
    char *headers; // Ex: "Content-Length: 12\r\n..."

    // Pour l'optimisation sendfile (bonus/conseil sujet)
    int body_fd; // File descriptor du fichier à envoyer (-1 si pas de fichier)
    size_t content_length; // Taille du contenu
};

/*
** @brief Construit une réponse HTTP en fonction de la requête et de la config.
** Gère la recherche de fichier, les erreurs 404/403, et les headers
*obligatoires.
*/
struct http_response *http_response_create(const struct http_request *req,
                                           struct config *conf);

/*
** @brief Envoie la réponse au client (headers + fichier via sendfile).
*/
void http_response_send(int client_fd, struct http_response *res);

/*
** @brief Libère la mémoire de la réponse.
*/
void http_response_destroy(struct http_response *res);

#endif /* ! RESPONSE_H */
