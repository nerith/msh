#ifndef _BUILTINS_H
#define _BUILTINS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "include/shellFunctions.h"

int executeBuiltin(char* command, char** arguments, shellInternal* env);

#endif
