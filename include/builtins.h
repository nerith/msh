#ifndef _BUILTINS_H
#define _BUILTINS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "shell.h"

int exec_builtin(char* command, char** arguments, env_t* env);

#endif
