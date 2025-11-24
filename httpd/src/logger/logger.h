#ifndef LOGGER_H
#define LOGGER_H

#include "config/config.h"
#include "http/request.h"
#include "http/response.h"

void logger_log_request(const struct http_request *req, const char *client_ip,
                        const struct config *config);
void logger_log_response(const struct http_request *req,
                         const struct http_response *res, const char *client_ip,
                         const struct config *config);

void logger_log_bad_request(const char *client_ip, const struct config *config);

#endif /* ! LOGGER_H */
