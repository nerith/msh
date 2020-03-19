#include <string.h>

#include "builtins.h"

#define builtin_handler(x) void builtin_##x(char *cmd, char **args, env_t *env)

struct builtin {
  char *name;
  void (*handler) (char*, char**, env_t*);
};

builtin_handler(cd) {
  char *dir = args[1];

  if(dir == NULL || !strcmp(dir, "~"))
    dir = getenv("HOME");

  if(chdir(dir) < 0)
    printf("cd: No such file or directory\n");
}

builtin_handler(exit) {
  exit(0);
}

builtin_handler(help) {
  puts("Usage:\n\nexit: exit the shell\nhelp: get a listing of the builtin cmds\n");
}

struct builtin builtins[] = {
  { "cd",   &builtin_cd   },
  { "exit", &builtin_exit },
  { "help", &builtin_help }
};

int exec_builtin(char *cmd, char **args, env_t *env) {
  int i;

  for(i = 0; i < sizeof(builtins) / sizeof(struct builtin); i++) {
    if(strcmp(builtins[i].name, cmd))
      continue;

    builtins[i].handler(cmd, args, env);
    return 0;
  }

  return -1;
}
