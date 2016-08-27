#include "include/builtins.h"

/**
 * Executes a builtin command.
 *
 * @param command    The command to execute
 * @param arguments  The arguments for the command
 * @param env        The shell's environment
 *
 * @return The status of execution (i.e. whether or not the builtin command
 *         was found.
 */
int executeBuiltin(char* command, char** arguments, shellInternal* env)
{
    if(command == 0 || strCompare("", command))
    {
        return 1;
    }

    if(strCompare("cd", command))
    {
        char* dir = arguments[1];

        if(dir == NULL || strCompare(dir, "~"))
        {
	    dir = getenv("HOME");
        }

        if(chdir(dir) < 0)
        {
            printf("cd: No such file or directory\n");
        }
    }
    else if(strCompare("exit", command))
    {
        free(env);
        exit(0);
    }
    else if(strCompare("help", command))
    {
        puts("Usage:\n");
        puts("exit: exit the shell");
        puts("help: get a listing of the builtin commands\n");
    }
    else
    {
        return 0;
    }

    return 1;
}
