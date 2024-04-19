#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "cp.h"
#include "mv.h"
#include "dirname.h"
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_INPUT_LENGTH 100
#define MAX_ARGS 10

// ANSI escape codes for colors
#define COLOR_RESET "\x1B[0m"
#define COLOR_LIGHT_BLUE "\x1B[94m"
#define COLOR_GREEN "\x1B[32m"

void parseInput(char *input, char *args[MAX_ARGS])
{
    int argCount = 0;
    char *token = strtok(input, " ");

    while (token != NULL && argCount < MAX_ARGS - 1)
    {
        args[argCount++] = token;
        token = strtok(NULL, " ");
    }
    args[argCount] = NULL;
}

void executeCommand(int argc, char *args[])
{
    if (strcmp(args[0], "cp") == 0)
    {
        if (args[1] != NULL && args[2] != NULL)
        {
            my_cp(argc, args);
        }
        else
        {
            fprintf(stderr, "Usage: cp [-i] [-r] [-t DIRECTORY] [-v] source destination\n");
            exit(EXIT_FAILURE);
        }
    }
    else if (strcmp(args[0], "mv") == 0)
    {
        if (args[1] != NULL && args[2] != NULL)
        {
            my_mv(argc, args);
        }
        else
        {
            fprintf(stderr, "Usage: mv [-i] [-S SUFFIX] [-b] [-t DIRECTORY] source destination\n");
            exit(EXIT_FAILURE);
        }
    }
    else if (strcmp(args[0], "dirname") == 0)
    {
        if (args[1] != NULL)
        {
            char *result = my_dirname(args[1]);
            if (result != NULL)
            {
                printf("%s\n", result);
                free(result);
            }
            else
            {
                fprintf(stderr, "Error getting dirname\n");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            fprintf(stderr, "Usage: dirname path\n");
            exit(EXIT_FAILURE);
        }
    }
    else if (strcmp(args[0], "cd") == 0)
    {
        if (args[1] != NULL)
        {
            if (chdir(args[1]) != 0)
            {
                perror("Error changing directory");
            }
        }
        else
        {
            fprintf(stderr, "Usage: cd directory\n");
        }
    }
    else if (strcmp(args[0], "exit") == 0)
    {
    	return;
    }
    else if (strcmp(args[0], "help") == 0)
    {
    	printf("The available commands:\n\n");
	printf("help => Brings up this screen on request\n");
	printf("exit => Leaves this console\n");
	printf("clear => Clears the current screen\n");
        printf("LS => List directory contents\n");
	printf("COPY -i,-r(-R),-t,-v => Copies a file from SOURCE to a DESTINATION\n");
	printf("MOVE -i,-t,-S(with -b) => Moves a file from SOURCE to DESTINATION\n");
	printf("DIRNAME => Outputs  each  NAME with its last non-slash component and trailing slashes removed\n");
	return;
    }
    else if (strcmp(args[0], "author") == 0)
    {
	printf("===Project for Operating Systems===\n");
	printf("Author: Potopea Mihaela-Teodora\n");
	printf("Student info:\n");
	printf("-email: mihaela.potopea03@e-uvt.ro\n");
	printf("-group: 3\n");
        printf("-subgoup: 4\n");
        printf("-year: 2nd year\n");
        printf("-specialization: Computer science\n");    
    }
    else
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            execvp(args[0], args);
            perror("Error executing command");
            exit(EXIT_FAILURE);
        }
        else if (pid > 0)
        {
            waitpid(pid, NULL, 0);
        }
        else
        {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
    }
}

int main()
{
    // Clear the terminal screen
    printf("\033[H\033[J");

    char *input;
    rl_bind_keyseq("\\e[A", rl_get_previous_history);
    rl_bind_keyseq("\\e[B", rl_get_next_history);

    while (1)
    {
        // Get the current working directory
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s%s%s%s>", COLOR_LIGHT_BLUE, "MihaelaTerminal", COLOR_RESET, COLOR_GREEN);
            printf("%s%s%s>", cwd, COLOR_RESET, COLOR_GREEN);
        }
        else
        {
            perror("getcwd() error");
            printf("> ");
        }

        input = readline(" ");
        fflush(stdout);
        input[strcspn(input, "\n")] = '\0';
        add_history(input);

        if (!input[0])
        {
            continue;
        }

        if (strcmp(input, "exit") == 0)
        {
            printf("Exiting the terminal.\n");
            break;
        }

        pid_t pid = fork();

        if (pid == 0)
        {
            char *args[MAX_ARGS];
            parseInput(input, args);
            int argc = 0;
            while (args[argc] != NULL)
            {
                argc++;
            }
            executeCommand(argc, args);
        }
        else if (pid > 0)
        {
            waitpid(pid, NULL, 0);
        }
        else
        {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
