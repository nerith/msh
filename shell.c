/* Brandon Fairchild
 * The MUSH Shell
 * 2-11-2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void getInput();
void run();
void showShellMessage();
void showPrompt();

#define PROMPT_SYM '>'

int main(int argc, char** argv)
{
    run();

    return 0;
}


/**
 * Displays information about the shell
 *
 * @returns nothing
 */
void showShellMessage()
{
    puts("\t   __  ___ __  __ ____ __ __");
    puts("\t  /  |/  // / / // __// // /");
    puts("\t / /|_/ // /_/ /_\\ \\ / _  /");
    puts("\t/_/  /_/ \\____//___//_//_/\n");
    puts("\tTHE MODULAR UNIVERSAL SHELL\n");
    puts("Welcome to the MUSH shell!");
    puts("Created by Brandon Fairchild\n");
    puts("Enter `help` at any time to get extra information.");
    puts("Type `exit` to close the shell.\n");
}

/**
 * Displays the prompt symbol to let the user know they can enter
 * a new command.
 *
 * By default, the prompt symbol is provided by PROMPT_SYM.
 *
 * @returns nothing
 */
void showPrompt()
{
    printf("%c ", PROMPT_SYM);
}

/**
 * Starts the execution of the shell.
 *
 * @returns nothing
 */
void run()
{
    showShellMessage();
    getInput();
}

/**
 * Gets input from the user until the command `exit` is entered.
 *
 * @returns nothing
 */
void getInput()
{
    char* command = "";
    size_t len = 0;

    do
    {
        showPrompt();
        getline(&command, &len, stdin);

    } while(strcmp(command, "exit\n"));

    free(command);
}
