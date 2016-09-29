#include "include/shellFunctions.h"
#include "include/builtins.h"
#include <editline/readline.h>
#include <editline/history.h>

/**
 * Displays information about the shell
 *
 * @returns nothing
 */
void showShellMessage()
{
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

/**
 * Generates the prompt that lets the user know they can enter
 * a new command.
 *
 * By default, the prompt symbol is provided by PROMPT_SYM.
 *
 * @returns nothing
 */
char* generatePrompt()
{
    char user[100];
    char hostname[31];
    char* promptString = "%s@%s%c ";
    char* prompt = malloc(sizeof(char*));

    getlogin_r(user, 99);
    gethostname(hostname, 30);

    sprintf(prompt, promptString, user, hostname, PROMPT_SYM);

    return prompt;
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
    shellInternal* env = malloc(sizeof(shellInternal));

    do
    {
        char* prompt = generatePrompt();
        char* command = readline(prompt);

        add_history(command);
        runCommand(command, env);

        free(prompt);
        free(command);

    } while(1);

    free(env);
}

/**
 * Parses a given command into an array of tokens using strtok()
 *
 * @param command   The command that was entered at the prompt
 * @param arguments An array in which to store the results of parsing
 * @param env       The shell's internal environnment
 *
 * @returns A pointer to an array of tokens obtained from the given command.
 *
 *          i.e. `ls -la` will be stored as { "ls", "-la", NULL }
 */
void parseCommand(char* command, char** arguments, shellInternal* env)
{
    char* tokenArray[strlen(command)];
    char* token = strtok(command, " ");

    int i = 0;

    while(token)
    {
        char* start = token;

        while(*(token++))
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

    env->numberOfPipes = 0;

    for(i = 0; tokenArray[i] != NULL; i++)
    {
        // Exclude empty strings from the returned array
        // This could cause commands such as `ls` to be passed nothing if there
        // are spaces after it
        if(!strCompare(tokenArray[i], ""))
        {
            if(!strCompare(tokenArray[i], "&") && !strCompare(tokenArray[i], "|"))
            {
		if(tokenArray[i][0] == '$')
		{
		    if(strCompare(tokenArray[i], "$0"))
		    {
		        tokenArray[i] = "msh";
		    }
		    else
		    {
			tokenArray[i] = "";
		    }
		}

                arguments[i] = tokenArray[i];
            }
            else
            {
                if(strCompare(tokenArray[i], "|"))
                {
                    env->piping = 1;
		    env->numberOfPipes++;
                }
                else if(strCompare(tokenArray[i], "&"))
                {
                    env->background = 1;
                }

                arguments[i] = NULL;
            }
        }
    }

    arguments[i] = NULL;
}

/**
 * Handles redirection setup for the shell.
 *
 * @param tokens     A character string array containing the tokens from parsing
 * @param index      The current index into the tokens array
 * @param fid        The file descriptor to modify
 * @param attributes Flags to open the file with (i.e O_CREAT | O_TRUNC, etc.)
 *
 * @returns nothing
 */
void handleRedirection(char** tokens, int* index, int fid, int attributes)
{
    // Check that this isn't the first or last token in the command since it does
    // not make sense to have redirection as the first or last command token.
    if(*index == 0 || tokens[(*index)+1] == NULL)
    {
        return;
    }

    tokens[*index] = NULL;

    int fileId = open(tokens[*(index)+1], attributes, 0666);
    close(fid);
    dup(fileId);
    close(fileId);
    (*index)++;
}

/**
 * Chooses which I/O redirection to perform and sends arguments based on that
 * to handleRedirection(). If there is no redirection specified in the command,
 * nothing will happen.
 *
 * @param tokens  A character string array containing the tokens from parsing
 *
 * @returns nothing
 */
void redirect(char** tokens)
{
    for(int i = 0; tokens[i] != NULL; i++)
    {
        if(strCompare(tokens[i], ">"))
        {
            handleRedirection(tokens, &i, 1, O_CREAT | O_WRONLY | O_TRUNC);
        }

        if(strCompare(tokens[i], ">>"))
        {
            handleRedirection(tokens, &i, 1, O_WRONLY | O_APPEND);
        }

        if(strCompare(tokens[i], "<"))
        {
            handleRedirection(tokens, &i, 0, O_RDONLY);
        }
    }
}

/**
 * Executes a command based on the arguments specified. If the command cannot be
 * found, the shell prints out an error message.
 *
 * @param arguments A character string array containing the tokens from parsing
 *
 * @returns nothing
 */
void execute(char** arguments)
{
    if(!*arguments)
    {
        return;
    }

    redirect(arguments);

    if(execvp(*arguments, arguments) < 0)
    {
        printf("\x1b[31;1m msh: `%s` - command not found. \x1b[0m\n", arguments[0]);
    }
}

/**
 * Recursively sends data to a pipe to be read by another process.
 *
 * @param arguments A character array containing the command to execute
 * @param env       The shell's internal environment
 *
 * @returns nothing
 */
void sendDataToPipe(char** arguments, shellInternal* env)
{
    if(env->numberOfPipes == 0)
    {
        execute(arguments);
    }

    int pipeId[2];
    pipe(pipeId);

    pid_t pid = fork();

    if(pid == 0)
    {
        close(1);
        dup(pipeId[1]);
        close(pipeId[1]);

        execute(arguments);
    }
    else
    {
        close(pipeId[1]);
	close(0);
	dup(pipeId[0]);
	close(pipeId[0]);

	wait(NULL);

	env->numberOfPipes--;

	while(*(++arguments));

	sendDataToPipe(++arguments, env);
    }
}

/**
 * Initiates pipelining between commands.
 *
 * i.e. `ls | wc -l` will need this function to pipe output
 * and input between them.
 *
 * @param commandList The parsed command to be executed
 * @param env         The shell's internal environment
 *
 * @returns nothing
 */
void runPipeline(char** commandList, shellInternal* env)
{
    pid_t pid = fork();

    if(pid == 0)
    {
	sendDataToPipe(commandList, env);
    }
    else
    {
	wait(NULL);
    }
}

/**
 * Runs the command specified by the user. This is the main starting point
 * for command execution.
 *
 * @param commandString  The command entered into the prompt
 *
 * @returns nothing
 */
void runCommand(char* commandString, shellInternal* env)
{
    env->piping = 0;
    env->background = 0;

    // Parse the given command before running it
    char* arguments[MAX_ARGUMENTS];
    parseCommand(commandString, arguments, env);

    char* command = arguments[0];

    if(!executeBuiltin(command, arguments, env))
    {
        if(env->piping)
        {
            runPipeline(arguments, env);
        }
        else
        {
            pid_t pid = fork();

            if(pid == 0)
            {
                execute(arguments);
                exit(0);
            }
            else
            {
                // Check for running a background process
                if(!env->background)
                {
                    waitpid(pid, NULL, 0);
                }
                else
                {
                    sleep(1);
                    printf("%d\n", pid);
                }
            }
        }
    }

    clearData(arguments, MAX_ARGUMENTS);
}

/**
 * Checks for the equality of two strings
 *
 * @param str     The expected value of the string
 * @param testStr The string that is to be tested against the expected value
 *
 * @returns 1 if the strings are equal. 0 if the strings are not equal.
 */
int strCompare(char* str, char* testStr)
{
    return strcmp(str, testStr) == 0 ? 1 : 0;
}

/**
 * Clear the stored data from a command token array
 *
 * @param data   An array of command tokens
 * @param length The length of the array
 *
 * @returns nothing
 */
void clearData(void* data, int length)
{
    char* d = (char*) data;

    for(int i = 0; i < length && d[i] != 0; i++)
    {
	d[i] = 0;
    }
}
