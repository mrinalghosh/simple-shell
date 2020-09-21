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

// char* charLoc(char* str, char* ch) {
// }

int pipeHandler(char* tokens[]) {
    int fd1[2], fd2[2];

    pid_t pid;

    int cmd_count = 1;  // pipe count + 1
    char* command[TOKEN_LIMIT];

    size_t i = 0, j = 0, k = 0;

    // check for |
    while (tokens[i] != NULL) {
        // TODO: non-spaced check - if works use same check in cmdhandler
        if (strcmp(tokens[i], "|") == 0) {
            ++cmd_count;
            //TODO: strtok for | - add all to new array (split tokens) OR strchr
        }
        ++i;
    }

    // printf("Command count = %d", cmd_count);
    /* Loop over all commands in array - change to NEWARRAY when strtok for | imp */
    // while (tokens[j] != NULL) {
    //     k = 0;
    //     while (strcmp(tokens[j], "|") != 0) {
    //         command[k] = tokens[j];
    //         ++k;
    //     }
    //     ++j;
    // }

    return 0;
}

int commandParser(char* tokens[]) {
    char* basetokens[TOKEN_LIMIT];
    metachar* metachars[TOKEN_LIMIT];  // array of indexes to metacharacters in order

    size_t i = 0, j = 0;

    while (tokens[i] != NULL) {  // get token count and assign to new string
        if (strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], "<") == 0 || strcmp(tokens[i], "|") == 0 || strcmp(tokens[i], "&") == 0) {
            metachars[j]->index = i;         // index of metacharacter
            metachars[j]->type = tokens[i];  // pointer to metacharacter
            ++j;                             // metacharacter count
        }
        basetokens[i] = tokens[i];
        ++i;  // number of tokens;
    }

    // if (metamask[2]) {        // PIPE
    //     pipeHandler(tokens);  // need to pass entire args - might have multiple pipes
    //     return 2;             // TODO: encode return from cmd handler
    // } // TODO: deal with pipe without masking - maybe have a list of operations?

    return 0;
}

int main(int argc, char** argv) {
    int token_count;
    int suppress = (argc > 1) && !strcmp(argv[1], "-n");  // suppress output

    // pid_t pid;
    // int status;
    // int retcode;
    int i;

    char buffer[MAX_BUFFER];    // DON'T NEED TO MALLOC THESE - MAX SIZE GIVEN
    char* tokens[TOKEN_LIMIT];  // TODO: may not need array - might be able to dynamically allocate only size needed?

    while (true) {
        token_count = 1;
        if (!suppress)
            prompt();

        memset(buffer, '\0', MAX_BUFFER);  // TODO:not working for ctrl-D - but memory IS zeroed

        fgets(buffer, MAX_BUFFER, stdin);

        // printf("buffer before: %s", buffer);

        i = 0;
        while (buffer[i] != '\0') {
            if ((buffer[i - 1] != ' ') && (buffer[i] == '|' || buffer[i] == '<' || buffer[i] == '>' || buffer[i] == '&')) {  // no space before
                memmove((buffer + i + 1), (buffer + i), sizeof(buffer) - i);
                buffer[i] = ' ';
            }
            if ((buffer[i + 1] != ' ') && (buffer[i] == '|' || buffer[i] == '<' || buffer[i] == '>' || buffer[i] == '&')) {  // no space after
                memmove((buffer + i + 2), (buffer + i + 1), sizeof(buffer) - i - 1);
                buffer[i + 1] = ' ';
            }
            ++i;
        }

        // printf("buffer after: %s", buffer);

        if ((tokens[0] = strtok(buffer, " \n\t\v")) == NULL)  // which whitespace characters possible?
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