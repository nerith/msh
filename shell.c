/* Brandon Fairchild
 * The MUSH Shell
 * 2-11-2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void getInput();
void run();
char** parseCommand(char*);
void runCommand(char*);
void showShellMessage();
void showPrompt();

#define PROMPT_SYM '>'
#define MAX_ARGUMENTS 21365

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
    char command[MAX_ARGUMENTS];

    do
    {
        showPrompt();
        fgets(command, MAX_ARGUMENTS-1, stdin);
        runCommand(command);

    } while(strcmp(command, "exit"));
}

/**
 * Parses a given command into an array of tokens using strtok()
 *
 * @returns A pointer to an array of tokens on the heap from the
 *          given command.
 *
 *          i.e. `ls -la` will be stored as { "ls", "-la", NULL }
 */
char** parseCommand(char* command)
{
    char* tokenArray[strlen(command)];
    char* token = strtok(command, " ");

    int i = 0;

    while(token != NULL)
    {
        char* start = token;

        for(; *token != '\0'; token++)
        {
  	    if(*token == '\n')
	    {
		*token = '\0';
	    }
	}

	// Reset to the start of the string
	token = start;
	
	tokenArray[i] = token;
	
	token = strtok(NULL, " ");
	i++;
    }

    tokenArray[i] = NULL;

    // Allocate the generated tokens onto the heap so that
    // they can be accessed later from other functions
    char** arguments = malloc(i * sizeof(char));

    for(i = 0; tokenArray[i] != NULL; i++)
    {
	// Exclude empty strings from the returned array
	// This could cause commands such as `ls` to be passed nothing if there
	// are spaces after it
	if(strcmp(tokenArray[i], "") != 0)
	{
	    arguments[i] = (char*)malloc(strlen(tokenArray[i]));
	    printf("%d, %d\n", i, strlen(tokenArray[i]));
	    arguments[i] = tokenArray[i];
	}
    }

    arguments[i] = NULL;

    return arguments;
}

void runCommand(char* command)
{
    // Parse the given command before running it
    char** arguments = parseCommand(command);

    if(strcmp(command, "") == 0)
    {
	return;
    }
    else if(strcmp(arguments[0], "cd") == 0)
    {
        puts("cd command");
	printf("Going to %s\n", arguments[1]);
	chdir(arguments[1]);
    }
    else if(strcmp(arguments[0], "help") == 0)
    {
	puts("usage:");
	puts("exit: exit the shell");
    }
    else if(strcmp(arguments[0], "exit") != 0)
    {
        pid_t pid = fork();

        if(pid == 0)
        {
            char** args = arguments;
            char commandLocation[100] = "/bin/";

            strcat(commandLocation, arguments[0]);
	    printf("%s\n", commandLocation);

            if(execvp(commandLocation, args) < 0)
            {
                strcpy(commandLocation, "/usr/bin/");
                strcat(commandLocation, arguments[0]);

                // The command was not found in /bin, so maybe the command will
                // be found in the other location that stores programs, /usr/bin/.
                if(execvp(commandLocation, args) < 0)
                {
                    printf("\x1b[31;1m mush: `%s` - command not found. \x1b[0m\n", command);
                }
            }

            exit(0);
        }
        else
        {
	    // TODO: Implement background processes
            wait(NULL);
        }
    }

    // TODO: Deallocate the command array
}
