#include <stdio.h>
#include <stdlib.h>

#include "config/config.h"
#include "daemon/daemon.h"

int main(int argc, char **argv)
{
    struct config *cfg = parse_configuration(argc, argv);

    if (cfg == NULL)
    {
        return 2; // Erreur de parsing
    }

    // Si l'option daemon n'est pas activée (ou pour l'instant, on ignore la
    // daemonization) On lance le serveur directement.

    // Note: Le sujet demande de gérer le démon (fork) avant de lancer le
    // serveur si l'option est présente. Pour l'instant, on lance en mode
    // "foreground" pour tester le réseau facilement.

    int res = daemon_handle_action(cfg);

    config_destroy(cfg);

    return res;
}
