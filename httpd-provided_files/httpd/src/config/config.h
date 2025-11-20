#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#include "utils/string/string.h"

// Macro for default daemon value as string
#define NO_DAEMON_ACTION "NO_OPTION"

// Structure pour la configuration du Virtual Host (un seul vhost requis)
struct server_config
{
    struct string *server_name;
    char *port;
    char *ip;
    char *root_dir;
    char *default_file; // "index.html" par défaut
};

// Structure pour la configuration globale du serveur
struct config
{
    char *pid_file;
    char *log_file;
    bool log_enabled; // true/false (parsé de l'argument)
    char *daemon_action; // "start", "stop", "restart", ou "NO_OPTION"

    struct server_config *vhost;
};

/*
 ** @brief Parse les arguments de la ligne de commande pour créer une structure
 *de configuration.
 **
 ** @param argc Le nombre d'arguments.
 ** @param argv Le vecteur d'arguments.
 **
 ** @return La nouvelle structure de configuration allouée dynamiquement, ou
 *NULL en cas d'erreur
 ** (option inconnue, argument --daemon invalide, ou champ obligatoire
 *manquant).
 */
struct config *parse_configuration(int argc, char **argv);

/*
 ** @brief Libère la mémoire allouée pour la structure de configuration.
 **
 ** @param cfg La structure à détruire.
 */
void config_destroy(struct config *cfg);

#endif /* ! CONFIG_H */
