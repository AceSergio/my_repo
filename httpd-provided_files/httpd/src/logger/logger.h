#ifndef LOGGER_H
#define LOGGER_H

#include "config/config.h"
#include "http/request.h"
#include "http/response.h"

/*
** @brief Log une requête reçue selon le format imposé :
** DATE [SERV_NAME] received REQUEST_TYPE on 'TARGET' from CLIENT_IP
*/
void logger_log_request(const struct http_request *req, const char *client_ip,
                        const struct config *config);

/*
** @brief Log une réponse envoyée selon le format imposé :
** DATE [SERV_NAME] responding with STATUS_CODE to CLIENT_IP for REQUEST_TYPE on
*'TARGET'
*/
void logger_log_response(const struct http_request *req,
                         const struct http_response *res, const char *client_ip,
                         const struct config *config);

/*
** @brief Log spécial pour les erreurs 400 (Bad Request) où la requête n'a pas
*pu être parsée.
*/
void logger_log_bad_request(const char *client_ip, const struct config *config);

#endif /* ! LOGGER_H */
