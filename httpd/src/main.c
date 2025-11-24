#include <stdio.h>
#include <stdlib.h>

#include "config/config.h"
#include "daemon/daemon.h"

int main(int argc, char **argv)
{
    struct config *cfg = parse_configuration(argc, argv);

    if (cfg == NULL)
    {
        return 2;
    }

    int res = daemon_handle_action(cfg);
    config_destroy(cfg);
    return res;
}
