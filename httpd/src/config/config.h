#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#include "utils/string/string.h"
#define NO_DAEMON_ACTION "NO_OPTION"

struct server_config
{
    struct string *server_name;
    char *port;
    char *ip;
    char *root_dir;
    char *default_file;
};

struct config
{
    char *pid_file;
    char *log_file;
    bool log_enabled;
    char *daemon_action;

    struct server_config *vhost;
};

struct config *parse_configuration(int argc, char **argv);
void config_destroy(struct config *cfg);

#endif /* ! CONFIG_H */
