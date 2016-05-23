#ifndef _SHELL_H
#define _SHELL_H

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

typedef struct shellInternal {
    int piping;
    int background;
    int numberOfPipes;
} shellInternal;

void execute(char**);
void getInput();
void handleRedirection(char**, int*, int, int);
void parseCommand(char*, char**, shellInternal*);
void readDataFromPipe(int[], char**, shellInternal*);
void redirect(char**);
void run();
void runCommand(char*, shellInternal*);
void sendDataToPipe( char**, shellInternal*);
void showShellMessage();
void showPrompt();
void clearData(void*, int);

void runPipeline(char**, shellInternal*);

#define PROMPT_SYM '>'
#define MAX_ARGUMENTS 21365

#endif
