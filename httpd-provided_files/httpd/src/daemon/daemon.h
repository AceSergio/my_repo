#ifndef DAEMON_H
#define DAEMON_H

#include "config/config.h"

/*
** @brief Point d'entrée principal pour la gestion du démon.
** Analyse l'action demandée (start, stop, restart ou aucune) et agit en
*conséquence.
**
** @param config La configuration globale.
** @return 0 en cas de succès, une valeur non nulle en cas d'erreur.
*/
int daemon_handle_action(struct config *config);

#endif /* ! DAEMON_H */
