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

int pipeHandler(char* tokens[]) {
    int fd1[2], fd2[2];
    pid_t pid;

    int cmd_count; // pipe count + 1
    char* command[MAX_BUFFER];


    return 0;
}

int commandParser(char* tokens[]) {
    char* metachars[4] = {">", "<", "|", "&"};
    bool metamask[4] = {false, false, false, false};

    char* base_tokens[TOKEN_LIMIT];  // to hold left side arguments - this won't work with more than 1 meta-char - might need more?
    char* aux_tokens[TOKEN_LIMIT];   // right side of meta-delimited args

    size_t i = 0;

    while (tokens[i] != NULL) { // only looking for first metacharacter - combinations? - pipe might be only one without
        for (size_t j = 0; j < 4; ++j)
            if (strcmp(tokens[i], metachars[j]) == 0) metamask[j] = true;

        if (metamask[0] || metamask[1] || metamask[2] || metamask[3])
            break;  // can't break out of while within for loop above

        base_tokens[i] = tokens[i];  // left
        ++i;
    }

    // for (size_t j = 0; j < i; ++j) {
    //     printf("%s--", base_tokens[j]);
    // }

    if (metamask[2]) { // PIPE
        pipeHandler(tokens);  // need to pass entire args - might have multiple pipes
        return 2;           // TODO: encode return from cmd handler
    }

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

        retcode = commandParser(tokens);  // error checking via encoded return value?
    }

    return 0;
}