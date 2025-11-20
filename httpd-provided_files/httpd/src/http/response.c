#define _POSIX_C_SOURCE 200809L

#include "response.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h> // Pour sendfile (Linux)
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

// Helpers pour les status
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
    // Format RFC 1123: Mon, 07 Nov 1994 08:49:37 GMT
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

    // Headers minimaux pour une erreur
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
    // 1. Vérification de la méthode (Seuls GET et HEAD sont supportés pour
    // l'instant)
    if (req->method != HTTP_GET && req->method != HTTP_HEAD)
    {
        return create_error_response(STATUS_405);
    }

    // 2. Construction du chemin complet (root_dir + target)
    // Attention: ceci est une version simplifiée. En prod, il faut sécuriser
    // contre ".."
    size_t path_len = strlen(conf->vhost->root_dir) + strlen(req->target) + 2;
    char *full_path = malloc(path_len);
    sprintf(full_path, "%s%s", conf->vhost->root_dir, req->target);

    // 3. Vérification du fichier
    struct stat st;
    if (stat(full_path, &st) == -1)
    {
        free(full_path);
        return create_error_response(STATUS_404);
    }

    // 4. Gestion des dossiers (Default File)
    if (S_ISDIR(st.st_mode))
    {
        // Si c'est un dossier, on ajoute le fichier par défaut (ex: index.html)
        size_t new_len =
            strlen(full_path) + strlen(conf->vhost->default_file) + 2;
        char *dir_path = full_path; // pointeur temporaire
        full_path = realloc(dir_path, new_len);

        // Ajout d'un slash si nécessaire
        if (full_path[strlen(full_path) - 1] != '/')
        {
            strcat(full_path, "/");
        }
        strcat(full_path, conf->vhost->default_file);

        // On re-vérifie l'existence du fichier par défaut
        if (stat(full_path, &st) == -1)
        {
            free(full_path);
            return create_error_response(STATUS_404);
        }
    }

    // 5. Ouverture du fichier
    int fd = open(full_path, O_RDONLY);
    free(full_path); // On n'a plus besoin du chemin

    if (fd == -1)
    {
        // Si on ne peut pas ouvrir (permissions), c'est une 403
        return create_error_response(STATUS_403);
    }

    // 6. Succès : Création de la réponse 200 OK
    struct http_response *res = calloc(1, sizeof(struct http_response));
    res->status_line = strdup(STATUS_200);
    res->content_length = st.st_size;

    // Si c'est GET, on envoie le corps. Si HEAD, on garde fd fermé ou on le
    // gère
    if (req->method == HTTP_GET)
    {
        res->body_fd = fd;
    }
    else
    {
        close(fd); // Pour HEAD, on a juste besoin de la taille (stat), pas du
                   // contenu
        res->body_fd = -1;
    }

    // Construction des Headers
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

    // 1. Envoyer Status Line
    dprintf(client_fd, "%s\r\n", res->status_line);

    // 2. Envoyer Headers
    if (res->headers)
    {
        write(client_fd, res->headers, strlen(res->headers));
    }

    // 3. Envoyer Body (si fichier ouvert)
    if (res->body_fd != -1)
    {
        // sendfile est très efficace: copie noyau à noyau sans passer par user
        // space
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
