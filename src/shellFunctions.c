#include "include/shellFunctions.h"

int background = 0;
int piping = 0;

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

    } while(1);
}

/**
 * Parses a given command into an array of tokens using strtok()
 *
 * @returns A pointer to an array of tokens on the heap from the
 *          given command.
 *
 *          i.e. `ls -la` will be stored as { "ls", "-la", NULL }
 */
void parseCommand(char* command, char* arguments[])
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

    for(i = 0; tokenArray[i] != NULL; i++)
    {
	// Exclude empty strings from the returned array
	// This could cause commands such as `ls` to be passed nothing if there
	// are spaces after it
	if(strcmp(tokenArray[i], "") != 0)
	{
	    if(strcmp(tokenArray[i],"&") != 0 && strcmp(tokenArray[i], "|") != 0)
	    {
	        arguments[i] = tokenArray[i];
	    }
            else
            {
                if(strcmp(tokenArray[i], "|") == 0)
                {
                    piping = 1;
                    arguments[i] = NULL;
                }
	        else if(strcmp(tokenArray[i], "&") == 0)
	        {
	            background = 1;
                    arguments[i] = NULL;
	        }
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
    // Check that this isn't the first or last token in the command
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
        if(strcmp(tokens[i], ">") == 0)
        {
            handleRedirection(tokens, &i, 1, O_CREAT | O_WRONLY | O_TRUNC);
        }

        if(strcmp(tokens[i], ">>") == 0)
        {
            handleRedirection(tokens, &i, 1, O_WRONLY | O_APPEND);
        }

        if(strcmp(tokens[i], "<") == 0)
        {
            handleRedirection(tokens, &i, 0, O_RDONLY);
        }
    }
}

/**
 * Executes a command based on the arguments specified. It will first look in
 * /bin for the command to execute and if it is not there, it will look in
 * /usr/bin/.
 *
 * Otherwise, the shell prints out an error message.
 *
 * @param arguments A character string array containing the tokens from parsing
 *
 * @returns nothing
 */
void execute(char** arguments)
{
    char commandLocation[100] = "/bin/";
    strncat(commandLocation, arguments[0], 93);
    redirect(arguments);

    if(execvp(commandLocation, arguments) < 0)
    {
        strcpy(commandLocation, "/usr/bin/");
        strncat(commandLocation, arguments[0], 90);

        // The command was not found in /bin, so maybe the command will
        // be found in the other location that stores programs, /usr/bin/.
        if(execvp(commandLocation, arguments) < 0)
        {
            printf("\x1b[31;1m mush: `%s` - command not found. \x1b[0m\n", arguments[0]);
        }
    }
}

/**
 * Sends data to a pipe to be read by another process.
 *
 * @param fd        An integer array to contain a pair of file descriptors
 * @param arguments A character array containing the command to execute
 *
 * @returns nothing
 */
void sendDataToPipe(int fd[], char** arguments)
{
    if(fork() == 0)
    {
        // Setup the pipe output
        close(1);
        dup(fd[1]);
        close(fd[0]);

        char* firstCommand[MAX_ARGUMENTS];
        int i;
        for(i = 0; arguments[i] != NULL; i++)
        {
            firstCommand[i] = arguments[i];
        }

        firstCommand[i++] = NULL;
        execute(firstCommand);
        exit(0);
    }
    else
    {
        wait(NULL);
    }
}

/**
 * Reads data from a pipe that was sent by another process.
 *
 * @param fd        An integer array to contain a pair of file descriptors
 * @param arguments A character array containing the command to execute
 *
 * @returns nothing
 */
void readDataFromPipe(int fd[], char** arguments)
{
    if(fork() == 0)
    {
        close(0);
        dup(fd[0]);
        close(fd[1]);

        int i;
        for(i = 0; arguments[i] != NULL; i++)
        {
        }

        i++;

        // Check for redirecting the output to a file
        // i.e. ps aux | grep ^root > file
        redirect(&arguments[i]);

        char* secondCommand[MAX_ARGUMENTS - i];

        int j;
        for(j= 0; arguments[i] != NULL; i++, j++)
        {
            secondCommand[j] = arguments[i];
        }

        secondCommand[j++] = NULL;
        execute(secondCommand);
        exit(0);
    }
    else
    {
        wait(NULL);
        sleep(1);
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
void runCommand(char* commandString)
{
    // Parse the given command before running it
    char* arguments[MAX_ARGUMENTS];
    parseCommand(commandString, arguments);

    if(strcmp(arguments[0], "") == 0 || arguments[0] == NULL)
    {
        return;
    }
    else if(strcmp(arguments[0], "cd") == 0)
    {
        if(chdir(arguments[1]) < 0)
        {
            printf("%s: No such file or directory\n", arguments[1]);
        }
    }
    else if(strcmp(arguments[0], "exit") == 0)
    {
        exit(0);
    }
    else if(strcmp(arguments[0], "help") == 0)
    {
        puts("usage:");
        puts("exit: exit the shell");
    }
    else if(strcmp(arguments[0], "exit") != 0)
    {
        if(piping)
        {
            int pipeId[2];
            pipe(pipeId);
            sendDataToPipe(pipeId, arguments);
            close(pipeId[1]);
            readDataFromPipe(pipeId, arguments);
            close(pipeId[0]);
            piping = 0;
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
	        if(!background)
	        {
                    waitpid(pid, NULL, 0);
	        }
	        else
	        {
                    background = 0;
                    sleep(1);
                    printf("%d\n", pid);
	        }
            }
        }
    }
}
