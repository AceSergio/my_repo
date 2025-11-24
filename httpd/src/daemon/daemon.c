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

static pid_t get_pid_from_file(const char *pid_file)
{
    FILE *f = fopen(pid_file, "r");
    if (!f)
        {
            return -1;
        }

    pid_t pid;
    if (fscanf(f, "%d", &pid) != 1)
    {
        pid = -1;
    }
    fclose(f);
    return pid;
}

static int is_process_running(pid_t pid)
{
    if (pid <= 0)
        {
            return 0;
        }
    if (kill(pid, 0) == 0)
        {
            return 1;
        }
    return 0;
}

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
            return 1;
        }
    }
    else
    {
        printf("Daemon not running or PID file invalid.\n");
    }

    unlink(config->pid_file);
    return 0;
}

static int daemon_start(struct config *config)
{
    pid_t pid = get_pid_from_file(config->pid_file);
    if (pid > 0 && is_process_running(pid))
    {
        fprintf(stderr, "Daemon already running (PID %d)\n", pid);
        return 1;
    }

    unlink(config->pid_file);

    printf("Starting daemon...\n");

    pid_t f = fork();
    if (f < 0)
    {
        perror("fork");
        return 1;
    }

    if (f > 0)
    {
        return 0;
    }

    setsid();

    if (write_pid_file(config->pid_file) == -1)
    {
        exit(1);
    }

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
        dup2(log_fd, STDOUT_FILENO);
        dup2(log_fd, STDERR_FILENO);
        close(log_fd);
    }
    else
    {
        int dev_null = open("/dev/null", O_WRONLY);
        if (dev_null != -1)
        {
            dup2(dev_null, STDOUT_FILENO);
            dup2(dev_null, STDERR_FILENO);
            close(dev_null);
        }
    }

    close(STDIN_FILENO);

    int res = server_start(config);

    unlink(config->pid_file);
    exit(res);
}

int daemon_handle_action(struct config *config)
{
    const char *action = config->daemon_action;

    if (strcmp(action, "NO_OPTION") == 0)
    {
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
        daemon_stop(config);
        sleep(1);
        return daemon_start(config);
    }

    return 2;
}
