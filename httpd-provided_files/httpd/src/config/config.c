/*
 * IMPORTANT: Cette macro doit être la TOUTE PREMIÈRE ligne.
 * Elle active les fonctions POSIX comme strdup() qui sont masquées
 * par défaut quand on compile avec -std=c99.
 */
#define _POSIX_C_SOURCE 200809L

/*
 * Inclusions des bibliothèques système EN PREMIER.
 * Cela évite que vos fichiers .h (comme string.h) n'entrent en conflit
 * avec les bibliothèques standard.
 */

#include "config.h"

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Définition des options longues
enum
{
    OPT_PID_FILE = 1,
    OPT_LOG_FILE,
    OPT_LOG,
    OPT_DAEMON,
    OPT_SERVER_NAME,
    OPT_PORT,
    OPT_IP,
    OPT_ROOT_DIR,
    OPT_DEFAULT_FILE
};

static const struct option long_options[] = {
    { "pid_file", required_argument, 0, OPT_PID_FILE },
    { "log_file", required_argument, 0, OPT_LOG_FILE },
    { "log", required_argument, 0, OPT_LOG },
    { "daemon", required_argument, 0, OPT_DAEMON },
    { "server_name", required_argument, 0, OPT_SERVER_NAME },
    { "port", required_argument, 0, OPT_PORT },
    { "ip", required_argument, 0, OPT_IP },
    { "root_dir", required_argument, 0, OPT_ROOT_DIR },
    { "default_file", required_argument, 0, OPT_DEFAULT_FILE },
    { 0, 0, 0, 0 }
};

// Fonction utilitaire pour copier une chaîne (et éviter les fuites)
static char *safe_strdup(const char *src)
{
    if (!src)
        return NULL;
    return strdup(src);
}

// Fonction utilitaire pour initialiser la structure et définir les valeurs par
// défaut.
static struct config *config_create_and_init(void)
{
    struct config *cfg = calloc(1, sizeof(struct config));
    if (!cfg)
        return NULL;

    cfg->vhost = calloc(1, sizeof(struct server_config));
    if (!cfg->vhost)
    {
        free(cfg);
        return NULL;
    }

    // Valeurs par défaut
    cfg->daemon_action = safe_strdup(NO_DAEMON_ACTION);
    cfg->log_enabled = true; // Par défaut: true
    cfg->log_file = NULL; // Par défaut: NULL
    cfg->vhost->default_file =
        safe_strdup("index.html"); // Par défaut: "index.html"

    if (!cfg->daemon_action || !cfg->vhost->default_file)
    {
        config_destroy(cfg);
        return NULL;
    }

    return cfg;
}

// Fonction utilitaire pour vérifier les champs obligatoires
static bool check_mandatory_fields(const struct config *cfg)
{
    return cfg->pid_file != NULL && cfg->vhost->server_name != NULL
        && cfg->vhost->port != NULL && cfg->vhost->ip != NULL
        && cfg->vhost->root_dir != NULL;
}

struct config *parse_configuration(int argc, char **argv)
{
    struct config *cfg = config_create_and_init();
    if (!cfg)
    {
        return NULL;
    }

    // Réinitialiser getopt pour permettre plusieurs appels successifs
    optind = 1;
    opterr = 0;

    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, "", long_options, &option_index)) != -1)
    {
        // Si c est 0, c'est une option longue
        if (c == 0)
        {
            c = long_options[option_index].val;
        }

        switch (c)
        {
        // --- Global Options ---
        case OPT_PID_FILE:
            free(cfg->pid_file);
            cfg->pid_file = safe_strdup(optarg);
            break;

        case OPT_LOG_FILE:
            free(cfg->log_file);
            cfg->log_file = safe_strdup(optarg);
            break;

        case OPT_LOG:
            // Log: "true" ou "false", sinon "false"
            if (strcmp(optarg, "true") == 0)
            {
                cfg->log_enabled = true;
            }
            else
            {
                cfg->log_enabled = false;
            }
            break;

        case OPT_DAEMON:
            // Daemon: "start", "stop", ou "restart"
            if (strcmp(optarg, "start") == 0 || strcmp(optarg, "stop") == 0
                || strcmp(optarg, "restart") == 0)
            {
                free(cfg->daemon_action);
                cfg->daemon_action = safe_strdup(optarg);
            }
            else
            {
                config_destroy(cfg);
                return NULL;
            }
            break;

        // --- Vhost Options ---
        case OPT_SERVER_NAME:
            if (cfg->vhost->server_name)
            {
                string_destroy(cfg->vhost->server_name);
            }
            // strlen est maintenant bien visible grâce à <string.h>
            cfg->vhost->server_name = string_create(optarg, strlen(optarg));
            break;

        case OPT_PORT:
            free(cfg->vhost->port);
            cfg->vhost->port = safe_strdup(optarg);
            break;

        case OPT_IP:
            free(cfg->vhost->ip);
            cfg->vhost->ip = safe_strdup(optarg);
            break;

        case OPT_ROOT_DIR:
            free(cfg->vhost->root_dir);
            cfg->vhost->root_dir = safe_strdup(optarg);
            break;

        case OPT_DEFAULT_FILE:
            free(cfg->vhost->default_file);
            cfg->vhost->default_file = safe_strdup(optarg);
            break;

        case '?':
        default:
            config_destroy(cfg);
            return NULL;
        }
    }

    // Vérifier les champs obligatoires
    if (!check_mandatory_fields(cfg))
    {
        config_destroy(cfg);
        return NULL;
    }

    return cfg;
}

void config_destroy(struct config *cfg)
{
    if (!cfg)
    {
        return;
    }

    // Libérer les champs char*
    free(cfg->pid_file);
    free(cfg->log_file);
    free(cfg->daemon_action);

    // Libérer les champs vhost
    if (cfg->vhost)
    {
        if (cfg->vhost->server_name)
            string_destroy(cfg->vhost->server_name);

        free(cfg->vhost->port);
        free(cfg->vhost->ip);
        free(cfg->vhost->root_dir);
        free(cfg->vhost->default_file);
        free(cfg->vhost);
    }

    free(cfg);
}
