#include "myshell.h"

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// #define TRUE 1 // using stdbool
// #define FALSE 0

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

    // check for pipes - pass to pipe_handler
    char* metachars[4] = {GREATER, LESSER, PIPE, AMPERSAND};
    bool metamask[4] = {false, false, false, false};  // METACHARACTER mask - ['>', '<', '|', '&']

    char* base_tokens[TOKEN_LIMIT];  // to hold left side arguments - this won't work with more than 1 meta-char - might need more?
    char* aux_tokens[TOKEN_LIMIT];   // right side of meta-delimited args

    size_t i = 0;  // temp indices for iteration

    //only looking for single spaced metachar
    while (tokens[i] != NULL) {
        for (size_t j = 0; j < 4; ++j) {
            metamask[j] = (strcmp(tokens[i], metachars[j]) == 0) ? true : false;
            printf("DETECTED %s - breaking\n", metamask[j]);
            break; // does this break out of outer while loop?
        }
        base_tokens[i] = tokens[i];  // add tokens to base (left side of )
        printf("adding token %s \n", tokens[i]);
        ++i;                         //increment
    }

    // for (size_t j = 0; j < i; ++j) {
    //     printf("%s--", base_tokens[j]);
    // }

    return 0;
}

int main(int argc, char** argv) {
    int token_count;
    int status;
    int suppress = (argc > 1) && !strcmp(argv[1], "-n");  // suppress output
    pid_t pid;
    int retcode;

    char buffer[MAX_BUFFER];    // DON'T NEED TO MALLOC THESE - MAX SIZE GIVEN
    char* tokens[TOKEN_LIMIT];  // TODO: may not need array - might be able to dynamically allocate only size needed?

    while (true) {
        token_count = 1;
        if (!suppress)
            prompt();

        memset(buffer, '\0', MAX_BUFFER);  // TODO:not working for ctrl-D - but memory IS zeroed

        fgets(buffer, MAX_BUFFER, stdin);

        if ((tokens[0] = strtok(buffer, " \n\t\v")) == NULL)  // which whitespace characters can be input?
            continue;                                         // reshow prompt

        while ((tokens[token_count] = strtok(NULL, " \n\t\v")) != NULL)
            ++token_count;

        // HERE ONWARD SHOULD GO INTO command_handler();
        // if ((pid = fork()) > 0) {
        //     // PARENT
        //     printf("Hello from parent..waiting\n");
        //     pid = waitpid(pid, &status, 0);
        //     printf("child %d exited with status %d\n", pid, WEXITSTATUS(status));
        // } else {
        //     // CHILD
        //     execvp(tokens[0], tokens);
        // }

        retcode = command_handler(tokens);  // error checking via encoded return value?
    }

    return 0;
}