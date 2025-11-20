#ifndef SERVER_H
#define SERVER_H

#include "config/config.h"

/*
** @brief Lance le serveur HTTP.
** Cette fonction initialise les sockets, configure les signaux (SIGPIPE)
** et entre dans la boucle d'acceptation des clients.
**
** @param config La configuration globale du serveur.
** @return 0 en cas de succès (arrêt propre), 1 en cas d'erreur.
*/
int server_start(struct config *config);

#endif /* ! SERVER_H */
