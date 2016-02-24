#ifndef _SHELL_H
#define _SHELL_H

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct Command
{
    char** tokens;
    int redirection;
    int background;
    char* inputFile;
    char* outputFile;
};

typedef struct Command Command;

void getInput();
void handleRedirection(char**, int*, int, int);
void run();
void parseCommand(char*, char* []);
void redirect(char* []);
void runCommand(char*);
void showShellMessage();
void showPrompt();

#define PROMPT_SYM '>'
#define MAX_ARGUMENTS 21365

#endif
