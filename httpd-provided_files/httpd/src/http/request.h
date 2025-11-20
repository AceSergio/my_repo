#ifndef REQUEST_H
#define REQUEST_H

#include <stdbool.h>
#include <stddef.h>

// Seuls GET et HEAD sont obligatoires (Section 3.4.6)
enum http_method
{
    HTTP_GET,
    HTTP_HEAD,
    HTTP_UNKNOWN
};

struct http_request
{
    enum http_method method;
    char *target; // Ex: "/index.html"
    char *version; // Ex: "HTTP/1.1"

    // Nous ajouterons les headers et le body plus tard
    char *header_host;
    size_t content_length;
    bool has_content_length;
};

/*
** @brief Parse une requête HTTP brute.
**
** @param raw_request La chaîne contenant la requête reçue.
** @return Une structure http_request allouée, ou NULL si la requête est
*malformée.
*/
struct http_request *http_request_parse(const char *raw_request);

/*
** @brief Libère la mémoire d'une requête.
*/
void http_request_free(struct http_request *req);

#endif /* ! REQUEST_H */
