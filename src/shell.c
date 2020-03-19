/*
 * The mSH Shell
 */

#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <editline.h>

#include "builtins.h"
#include "shell.h"

/* TODO: stop assuming term supports color */

static void motd() {
  puts("\t            _____ _    _");
  puts("\t           / ____| |  | |");
  puts("\t _ __ ___ | (___ | |__| |");
  puts("\t| '_ ` _ \\ \\___ \\|  __  |");
  puts("\t| | | | | |____) | |  | |");
  puts("\t|_| |_| |_|_____/|_|  |_|");
  puts("\n\t   THE MINIATURE SHELL\n");
  puts("Welcome to mSH!\n");
  puts("Enter \x1b[31;1mhelp\x1b[0m at any time for extra information.");
  puts("Enter \x1b[31;1mexit\x1b[0m to close the shell.\n");
}

static void parse_cmd(char *cmd, char **args, env_t *env) {
  char *token_array[strlen(cmd)];
  char *token = strtok(cmd, " ");

  int i = 0;

  while(token) {
    char *start = token;

    while(*(token++))
      if(*token == '\n')
        *token = '\0';

    token = start;

    token_array[i] = token;

    token = strtok(NULL, " ");
    i++;
  }

  token_array[i] = NULL;

  env->npipes = 0;

  for(i = 0; token_array[i] != NULL; i++) {
    if(strcmp(token_array[i], "")) {
      if(strcmp(token_array[i], "&") && strcmp(token_array[i], "|")) {
        if(token_array[i][0] == '$') {
          if(!strcmp(token_array[i], "$0"))
            token_array[i] = "msh"; else {
              token_array[i] = "";
            }
        }

        args[i] = token_array[i];
      } else {
        if(!strcmp(token_array[i], "|")) {
          env->piping = 1;
          env->npipes++;
        } else if(!strcmp(token_array[i], "&"))
          env->background = 1;

        args[i] = NULL;
      }
    }
  }

  args[i] = NULL;
}

static void handle_redirection(char **tokens, int *index, int fid, int attr) {
  if(!*index || tokens[(*index)+1] == NULL)
    return;

  tokens[*index] = NULL;

  int file_id = open(tokens[*(index)+1], attr, 0666);
  close(fid);
  dup(file_id);
  close(file_id);
  (*index)++;
}

static void redirect(char **tokens) {
  int i;

  for(i = 0; tokens[i]; i++) {
    if(!strcmp(tokens[i], ">"))
      handle_redirection(tokens, &i, 1, O_CREAT | O_WRONLY | O_TRUNC);

    if(!strcmp(tokens[i], ">>"))
      handle_redirection(tokens, &i, 1, O_WRONLY | O_APPEND);

    if(!strcmp(tokens[i], "<"))
      handle_redirection(tokens, &i, 0, O_RDONLY);
  }
}

static void execute(char **args) {
  if(!*args)
    return;

  redirect(args);

  if(execvp(*args, args) < 0)
    printf("\x1b[31;1m msh: %s: command not found\x1b[0m\n", args[0]);
}

static void send_data_to_pipe(char **args, env_t *env) {
  pid_t pid;

  if(!env->npipes)
    execute(args);

  int pipe_id[2];
  pipe(pipe_id);

  pid = fork();
  if(!pid) {
    close(1);
    dup(pipe_id[1]);
    close(pipe_id[1]);
    execute(args);
  } else {
    close(pipe_id[1]);
    close(0);
    dup(pipe_id[0]);
    close(pipe_id[0]);

    wait(NULL);

    env->npipes--;

    while(*(++args));

    send_data_to_pipe(++args, env);
  }
}

static void run_pipeline(char **cmd_list, env_t *env) {
  pid_t pid = fork();

  if(!pid) {
    send_data_to_pipe(cmd_list, env);
  } else {
    wait(NULL);
  }
}

static void run_cmd(char *cmd_string, env_t *env) {
  pid_t pid;
  char *args[MAX_ARGUMENTS];

  env->piping = 0;
  env->background = 0;

  parse_cmd(cmd_string, args, env);

  char *cmd = args[0];

  if(!exec_builtin(cmd, args, env))
    return;

  if(env->piping) {
    run_pipeline(args, env);
    return;
  }

  pid = fork();
  if(!pid) {
    execute(args);
    exit(0);
  } else {
    if(!env->background) {
      waitpid(pid, NULL, 0);
    } else {
      sleep(1);
      printf("%d\n", pid);
    }
  }
}

int main(int argc, char **argv) {
  static env_t env;

  static char pr[256];
  char host[256];

  gethostname(host, sizeof(host));
  host[sizeof(host) - 1] = 0;

  snprintf(pr, sizeof(pr), "%s@%s%c ", getlogin(), host, getuid() ? '$' : '#');

  motd();

  for(;;) {
    /* TODO: make readline optional */
    char *cmd = readline(pr);

    if(!cmd)
      break;

    add_history(cmd);
    run_cmd(cmd, &env);

    free(cmd);
  }
}
