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

/*
TODO:
detect ctrl-D = SIGQUIT - MAY NEED A SIGNAL HANDLER

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
prompt
all tokenizing
-n flag to suppress prompt
basic REPL
*/

bool charCompare(char* str, char* list) {
    int i = 0;
    for (i = 0; i < strlen(str); ++i)
        if (strcmp(str, list[i]) == 0)
            return true;

    return false;
}

void prompt(void) {
    char buf[] = "my_shell$ ";
    write(STD_OUTPUT, buf, strlen(buf));
}

int pipeHandler(char* base[], char* aux[]) {
    int fd[2];  // file descriptors
    pid_t pid;

    pipe(fd);

    if ((pid = fork()) == -1) {
        // perror("fork error");
        // exit(1);
    }

    // int command_count = 1;  // pipe count + 1
    // char* command_args[TOKEN_LIMIT];

    // size_t i = 0;

    // while (tokens[i] != NULL) {
    //     if (strcmp(tokens[i], "|") == 0) {
    //         ++command_count;
    //     }
    //     ++i;
    // }

    return 0;
}

int commandHandler(char* tokens[]) {
    int i = 0, j = 0, k;

    char* base[TOKEN_LIMIT];  // left to metachars
    char* aux[TOKEN_LIMIT];   // right to metachar

    for (k = 0; k < TOKEN_LIMIT; ++k) {
        base[k] = malloc(MAX_TOKEN);
        aux[k] = malloc(MAX_TOKEN);
    }

    metachar* metachars = malloc(TOKEN_LIMIT * sizeof(metachar));  // array of indexes and type of metacharacters in order - FUNCTIONAL

    while (tokens[i] != NULL) {  // get token count and assign to new string
        if (strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], "<") == 0 || strcmp(tokens[i], "|") == 0 || strcmp(tokens[i], "&") == 0) {
            metachars[j].index = i;         // index of metacharacter
            metachars[j].type = tokens[i];  // pointer to metacharacter - string
            ++j;                            // METACHARACTER COUNT
        }
        ++i;  // TOKEN COUNT;
    }

    // printf("number of tokens: %d, number of metachars: %d", i, j);
    // printf("MC type, index: %s, %d", metachars[0].type, metachars[0].index);

    /* break into arrays of strings between metachars - can use to execvp */
    if (charComp(metachars[0].type, "|<>&")) {
        for (k = 0; k < metachars[0].index; ++k) {
            memcpy(base[k], tokens[k], MAX_TOKEN);
            printf("BASE %d %s\n", k, base[k]);
        }

        for (k = metachars[0].index + 1; k < i; ++k) {
            memcpy(aux[k - metachars[0].index - 1], tokens[k], MAX_TOKEN);
            printf("AUX %d %s\n", k - metachars[0].index - 1, aux[k - metachars[0].index - 1]);
        }
    }

    pid_t pid;
    int status;

    if (j == 0) {
        /* NO METACHARACTERS */
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

    // if (metamask[2]) {        // PIPE
    //     pipeHandler(tokens);  // need to pass entire args - might have multiple pipes
    //     return 2;             // TODO: encode return from cmd handler
    // } // TODO: deal with pipe without masking - maybe have a list of operations?

    // HERE ONWARD SHOULD GO INTO command_handler();

    return 0;
}

int main(int argc, char** argv) {
    int token_c;
    int i;

    char buffer[MAX_BUFFER];    // DON'T NEED TO MALLOC THESE - MAX SIZE GIVEN
    char* tokens[TOKEN_LIMIT];  // TODO: may not need array - might be able to dynamically allocate only size needed?

    bool suppress = ((argc > 1) && !strcmp(argv[1], "-n"));  // suppress output

    while (true) {
        token_c = 1;
        if (!suppress)
            prompt();

        memset(buffer, '\0', MAX_BUFFER);  // TODO:not working for ctrl-D - but memory IS zeroed
        fgets(buffer, MAX_BUFFER, stdin);

        // printf("buffer before: %s", buffer);

        /* correct spacing around metachars before tokenizing */
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
            continue;                                         // reshow prompt at next loop

        while ((tokens[token_c] = strtok(NULL, " \n\t\v")) != NULL)
            ++token_c;

        commandHandler(tokens);  // error checking via encoded return value? - do I need retcode?
    }

    return 0;
}