#define _POSIX_C_SOURCE 200809L

#include "daemon.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "server/server.h"

// Helper: Lit le PID depuis le fichier
static pid_t get_pid_from_file(const char *pid_file)
{
    FILE *f = fopen(pid_file, "r");
    if (!f)
        return -1;

    pid_t pid;
    if (fscanf(f, "%d", &pid) != 1)
    {
        pid = -1;
    }
    fclose(f);
    return pid;
}

// Helper: Vérifie si un processus est en vie
static int is_process_running(pid_t pid)
{
    if (pid <= 0)
        return 0;
    // kill(pid, 0) n'envoie pas de signal mais vérifie l'existence
    if (kill(pid, 0) == 0)
        return 1;
    return 0;
}

// Helper: Écrit le PID actuel dans le fichier
static int write_pid_file(const char *pid_file)
{
    FILE *f = fopen(pid_file, "w");
    if (!f)
    {
        perror("fopen pid_file");
        return -1;
    }
    fprintf(f, "%d\n", getpid());
    fclose(f);
    return 0;
}

static int daemon_stop(struct config *config)
{
    pid_t pid = get_pid_from_file(config->pid_file);

    if (pid > 0 && is_process_running(pid))
    {
        printf("Stopping daemon (PID %d)...\n", pid);
        if (kill(pid, SIGINT) == -1)
        {
            perror("kill");
            return 1; // Erreur lors de l'envoi du signal
        }
        // On pourrait attendre ici que le processus se termine vraiment,
        // mais pour l'instant on suppose que le signal est envoyé.
    }
    else
    {
        printf("Daemon not running or PID file invalid.\n");
    }

    // "After this step (or if the process is not alive), delete the PID_FILE"
    unlink(config->pid_file);

    return 0; // "Either there is an error or not, your program must return 0"
}

static int daemon_start(struct config *config)
{
    // 1. Vérifier si le fichier PID existe et si le processus tourne
    pid_t pid = get_pid_from_file(config->pid_file);
    if (pid > 0 && is_process_running(pid))
    {
        fprintf(stderr, "Daemon already running (PID %d)\n", pid);
        return 1; // "return 1" si vivant
    }

    // Nettoyage préventif si le fichier existait mais processus mort
    unlink(config->pid_file);

    printf("Starting daemon...\n");

    // 2. Fork
    pid_t f = fork();
    if (f < 0)
    {
        perror("fork");
        return 1;
    }

    if (f > 0)
    {
        // Parent: se termine avec succès
        return 0;
    }

    // --- ENFANT (Le Démon) ---

    // Créer une nouvelle session
    setsid();

    // 3. Écrire le PID
    if (write_pid_file(config->pid_file) == -1)
    {
        exit(1);
    }

    // 4. Redirection des logs
    // "must not print on stdout or stderr... write to log_file"
    int log_fd;
    const char *log_path = config->log_file ? config->log_file : "HTTPd.log";

    if (config->log_enabled)
    {
        log_fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (log_fd == -1)
        {
            perror("open log file");
            exit(1);
        }
        // Rediriger stdout et stderr vers le fichier de log
        dup2(log_fd, STDOUT_FILENO);
        dup2(log_fd, STDERR_FILENO);
        close(log_fd);
    }
    else
    {
        // Si logs désactivés, on redirige vers /dev/null pour ne pas polluer
        int dev_null = open("/dev/null", O_WRONLY);
        if (dev_null != -1)
        {
            dup2(dev_null, STDOUT_FILENO);
            dup2(dev_null, STDERR_FILENO);
            close(dev_null);
        }
    }

    // Fermer l'entrée standard
    close(STDIN_FILENO);

    // Lancer le serveur (bloquant)
    int res = server_start(config);

    // Nettoyage final (au cas où le serveur s'arrête de lui-même)
    unlink(config->pid_file);
    exit(res);
}

int daemon_handle_action(struct config *config)
{
    const char *action = config->daemon_action;

    if (strcmp(action, "NO_OPTION") == 0)
    {
        // Pas de mode démon, on lance le serveur directement
        return server_start(config);
    }

    if (strcmp(action, "start") == 0)
    {
        return daemon_start(config);
    }

    if (strcmp(action, "stop") == 0)
    {
        return daemon_stop(config);
    }

    if (strcmp(action, "restart") == 0)
    {
        // "equivalent to calling HTTPd with --daemon stop, then... start"
        daemon_stop(config);
        // Petit délai pour laisser le temps au socket de se libérer
        sleep(1);
        return daemon_start(config);
    }

    return 2; // Action inconnue (ne devrait pas arriver grâce au parsing
              // config)
}
