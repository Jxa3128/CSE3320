/*  Jorge Avila 
    UT Arlington
    1001543128
*
*
Description: In this assignment you will write your own shell program, Mav shell (msh), 
similar to bourne shell (bash), c-shell (csh), or korn shell (ksh). It will accept commands, 
fork a child process and execute those commands. The shell, like csh or bash, will run and 
accept commands until the user exits the shell. Your file must be named msh.c

*
*
*
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n" // We want to split our command line up into tokens // 
                           // so we need to define what delimits our tokens.   // 
                           // In this case  white space                        // 
                           // will separate the tokens on our command line //

#define MAX_COMMAND_SIZE 255 // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5 // Mav shell only supports five arguments

char *CommandHistory[15];

int main(int argc, char **argv)
{

    int full = 0;
    int PIDCOUNTER = 0;
    //dynamically allocate memory for user input
    char *cmd_str = (char *)malloc(MAX_COMMAND_SIZE);
    //position initilizers
    int counter = 0;
    int it = 0;
    int arrayPids[15];
    int lleno = 0;
    //for loop will iterate on the CommandHistory positions
    //and put zeros in them
    for (it = 0; it < 100; it++)
    {
        CommandHistory[it] = (char *)malloc(255);
        memset(CommandHistory[it], 0, 255);
        //printf("This is command history(%d) = %s.\n",it,CommandHistory[it]);
    }
    //while the person does not exit out
    while (1)
    {
        // Print out the msh prompt
        printf("msh> ");

        // Read the command from the commandline.  The
        // maximum command that will be read is MAX_COMMAND_SIZE
        // This while command will wait here until the user
        // inputs something since fgets returns NULL when there
        // is no input
        while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin))
            ;

    USERINPUT:

        if (strcmp(cmd_str, "\n") == 0)
        {
            //printf("NUEVA LINEA\n");
            continue;
        }

        //strcpy(CommandHistory[counter++], cmd_str); //original

        //added by bakker
        strncpy(CommandHistory[counter++], cmd_str, strlen(cmd_str));
        if (counter > 14)
        {
            counter = 0;
            full = 1;
        }
        if (PIDCOUNTER > 14)
        {
            PIDCOUNTER = 0;
            lleno = 1;
        }
        /* Parse input */
        char *token[MAX_NUM_ARGUMENTS];

        int token_count = 0;

        // Pointer to point to the token
        // parsed by strsep
        char *arg_ptr;

        char *working_str = strdup(cmd_str);

        // we are going to move the working_str pointer so
        // keep track of its original value so we can deallocate
        // the correct amount at the end
        char *working_root = working_str;

        // Tokenize the input stringswith whitespace used as the delimiter
        while (((arg_ptr = strsep(&working_str, WHITESPACE)) != NULL) &&
               (token_count < MAX_NUM_ARGUMENTS))
        {
            token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
            if (strlen(token[token_count]) == 0)
            {

                token[token_count] = NULL;
            }

            token_count++;
        }

        //if the user inputs either quit or exit then send 0 to main
        if (strcmp(token[0], "exit") == 0 || strcmp(token[0], "quit") == 0)
        {
            printf("Exiting...\n");
            exit(0);
        }
        //if user enters a new line then fak yo bit
        //pero no me sirve, que tristeza
        if (strcmp(token[0], "\n") == 0)
        {
            printf("This is inside the new line\n");
            continue;
        }
        //if user inputs cd then change directory
        if (strcmp(token[0], "cd") == 0)
        {

            if (strcmp(token[1], "..") == 0)
            {
                chdir("..");
            }
            chdir(token[1]);
            continue;
        }

        //if user enters showpids then iterates over
        //the pids entered (last 15)
        // if (strcmp(token[0], "showpids") == 0)
        // {
        //     if (!lleno)
        //     {
        //         for (it = 0; it < PIDCOUNTER; it++)
        //         {
        //             printf("%d: pid number %d\n", it, arrayPids[it]);
        //             if (PIDCOUNTER > 14)
        //             {
        //                 PIDCOUNTER = 0;
        //             }
        //         }
        //     }
        //     else
        //     {
        //         int contador = PIDCOUNTER;
        //         for (it = 0; it < 15; it++)
        //         {
        //             printf("%d: pid number %d\n", it, arrayPids[contador++]);
        //         }
        //         if(contador > 15) contador =0;
        //     }

        //     continue;
        // }
        //interates over the history array when user inputs history
        int historyIndex = 0;
        if (strcmp(token[0], "showpids") == 0)
        {
            //if the CommandHistory is not filled then
            //print up to the counter since it
            if (!full)
            {
                for (historyIndex = 0; historyIndex < counter; historyIndex++)
                {
                    printf("%d: %d\n", historyIndex, arrayPids[historyIndex]);
                }
            }

            else
            {
                int count = counter;
                for (historyIndex = 0; historyIndex < 15; historyIndex++)
                {
                    printf("%d: %d\n", historyIndex, arrayPids[count++]);
                    if (count > 14)
                        count = 0;
                }
            }
            // Jorge esta aqui
            continue;
        }
        //when the history command is entered
        if (strcmp(token[0], "history") == 0)
        {
            //if the CommandHistory is not filled then
            //print up to the counter since it
            if (!full)
            {
                for (historyIndex = 0; historyIndex < counter; historyIndex++)
                {
                    printf("%d: %s\n", historyIndex, CommandHistory[historyIndex]);
                }
            }

            else
            {
                int count = counter;
                for (historyIndex = 0; historyIndex < 15; historyIndex++)
                {
                    printf("%d: %s\n", historyIndex, CommandHistory[count++]);
                    if (count > 14)
                        count = 0;
                }
            }
            // Jorge esta aqui
            continue;
        }
        if (*token[0] == '!')
        {
            //tokenize the area that was used for an exclamation mark.
            //printf("The ! was used.\n");
            char *TokenPtr;
            const char EXCLAMATION[2] = "!";
            TokenPtr = strtok(token[0], EXCLAMATION);
            int numero = atoi(TokenPtr);
            cmd_str = CommandHistory[numero];
            if (strcmp(cmd_str, "") == 0)
            {
                printf("Error, no command.\n");
                continue;
            }
            else
            {
                goto USERINPUT;
            }
            if (numero < 0 || numero > counter)
            {
                printf("COMMAND NOT IN HISTORY!\n");
                continue;
            }
            //printf("THE NUMBER WAS  %d\n", numero);
        }
        //fork a child
        pid_t pid = fork();

        //iterates a pid for each new process

        //catches an error if pid is invalid
        if (pid == -1)
        {
            printf("fork failed!\n");
            exit(0);
        }
        else if (pid == 0)
        {
            int ret;

            //enters arguments in execvp to see if command is there
            //if the command is not there then it will catch the execption

            ret = execvp(token[0], token);
            if (ret == -1)
            {
                printf("%s: Command not found.\n", token[0]);
            }

            exit(0);
        }
        else
        {
            //pids are getting added to the array
            if (PIDCOUNTER < 15)
            {
                arrayPids[PIDCOUNTER] = pid;
                PIDCOUNTER++;
            }
            // else
            // {
            //     PIDCOUNTER = 0;
            // }

            int status;
            waitpid(pid, &status, 0);
            fflush(NULL);
        }

        free(working_root);
    }

    //end of program
    return 0;
}
