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

static pid_t recuperer_pid(const char *fichier_pid) {
  FILE *f = fopen(fichier_pid, "r");
  if (!f) {
    return -1;
  }

  pid_t pid;
  if (fscanf(f, "%d", &pid) != 1) {
    pid = -1;
  }
  fclose(f);
  return pid;
}

static int processus_actif(pid_t pid) {
  if (pid <= 0) {
    return 0;
  }
  if (kill(pid, 0) == 0) {
    return 1;
  }
  return 0;
}

static int ecrire_pid(const char *fichier_pid) {
  FILE *f = fopen(fichier_pid, "w");
  if (!f) {
    perror("fopen pid_file");
    return -1;
  }
  fprintf(f, "%d\n", getpid());
  fclose(f);
  return 0;
}

static int arret_demon(struct config *cfg) {
  pid_t pid = recuperer_pid(cfg->pid_file);

  if (pid > 0 && processus_actif(pid)) {
    printf("Stopping daemon (PID %d)...\n", pid);
    if (kill(pid, SIGINT) == -1) {
      perror("kill");
      return 1;
    }
  } else {
    printf("Daemon not running or PID file invalid.\n");
  }

  unlink(cfg->pid_file);
  return 0;
}

static int lancement_demon(struct config *cfg) {
  pid_t pid = recuperer_pid(cfg->pid_file);
  if (pid > 0 && processus_actif(pid)) {
    fprintf(stderr, "Daemon already running (PID %d)\n", pid);
    return 1;
  }

  unlink(cfg->pid_file);

  printf("Starting daemon...\n");

  pid_t f = fork();
  if (f < 0) {
    perror("fork");
    return 1;
  }

  if (f > 0) {
    return 0;
  }

  setsid();

  if (ecrire_pid(cfg->pid_file) == -1) {
    exit(1);
  }

  int fd_log;
  const char *chemin_log = cfg->log_file ? cfg->log_file : "HTTPd.log";

  if (cfg->log_enabled) {
    fd_log = open(chemin_log, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd_log == -1) {
      perror("open log file");
      exit(1);
    }
    dup2(fd_log, STDOUT_FILENO);
    dup2(fd_log, STDERR_FILENO);
    close(fd_log);
  } else {
    int dev_null = open("/dev/null", O_WRONLY);
    if (dev_null != -1) {
      dup2(dev_null, STDOUT_FILENO);
      dup2(dev_null, STDERR_FILENO);
      close(dev_null);
    }
  }

  close(STDIN_FILENO);

  int res = server_start(cfg);

  unlink(cfg->pid_file);
  exit(res);
}

int daemon_handle_action(struct config *cfg) {
  const char *action = cfg->daemon_action;

  if (strcmp(action, "NO_OPTION") == 0) {
    return server_start(cfg);
  }

  if (strcmp(action, "start") == 0) {
    return lancement_demon(cfg);
  }

  if (strcmp(action, "stop") == 0) {
    return arret_demon(cfg);
  }

  if (strcmp(action, "restart") == 0) {
    arret_demon(cfg);
    sleep(1);
    return lancement_demon(cfg);
  }

  return 2;
}
