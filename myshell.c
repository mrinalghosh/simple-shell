#include "myshell.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0
#define MAX_TOKEN 1 << 5
#define MAX_BUFFER 1 << 9
#define TOKEN_LIMIT 1 << 7  // maximum number of tokens - arbitrary choice of 128
#define STD_INPUT 0
#define STD_OUTPUT 1
#define STD_ERROR 2

/*
TODO:
detect ctrl-D = EOF = SIGQUIT - MAY NEED A SIGNAL HANDLER
basic fork execvp REPL -- read - eval - print - loop
need to hardcode handling metachars:
    (will probably need to dynamically allocate - see brk, sbrk, mmap)
    & - background task (don't wait)
    > - redirection of output
    < - redirection of input
    | - pipe redirection 
SIGNAL HANDLING - SIGQUIT and SIGCHLD especially
take in environment ? (see project desc)
*/

/*
DONE:
basic tokenizing
-n flag to suppress prompt
*/

void prompt(void) { printf("my_shell$: "); }  //TODO: replace all printf with write - see strace

int pipe_handler(char* tokens[]) {
    // handler for tokens delimited by |
    // while (tokens[1] != NULL)
    return 0;
}

int command_handler(char* tokens[]) {
    /* check for meta-characters (& > < |) */
    // char* secondary_tokens[TOKEN_LIMIT];
    return 0;
}

int main(int argc, char** argv) {
    int num_tokens;
    pid_t pid;
    int status;

    int suppress = (argc > 1) && !strcmp(argv[1], "-n");  // suppress output

    char buffer[MAX_BUFFER]; // DON'T NEED TO MALLOC THESE - MAX SIZE GIVEN
    char* tokens[TOKEN_LIMIT];  // TODO: may not need array - might be able to dynamically allocate only size needed?

    while (TRUE) {
        num_tokens = 1;
        if (!suppress)
            prompt();

        memset(buffer, '\0', MAX_BUFFER);  // TODO:not working for ctrl-D - but memory IS zeroed

        fgets(buffer, MAX_BUFFER, stdin);

        if ((tokens[0] = strtok(buffer, " \n\t\v")) == NULL)  // which whitespace characters can be input?
            continue;

        while ((tokens[num_tokens] = strtok(NULL, " \n\t\v")) != NULL)
            ++num_tokens;

        // HERE ONWARD SHOULD GO INTO command_handler();
        if ((pid = fork()) > 0) {
            // PARENT
            printf("Hello from parent..waiting\n");
            pid = waitpid(pid, &status, 0);
            printf("child %d exited with status %d\n", pid, WEXITSTATUS(status));
        } else {
            // CHILD
            execvp(tokens[0], tokens);
        }
    }

    return 0;
}