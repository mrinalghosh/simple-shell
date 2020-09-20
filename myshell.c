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
    char* metachars[4] = {">", "<", "|", "&"};
    bool metamask[4] = {false, false, false, false};

    char* base_tokens[TOKEN_LIMIT];   // to hold left side arguments - this won't work with more than 1 meta-char - might need more?
    char* clean_tokens[TOKEN_LIMIT];  // right side of meta-delimited args
    // char* subtokens[TOKEN_LIMIT];     // doesn't need to be this large
    char* temp;

    // size_t subcount = 0;

    size_t i = 0, j = 0, k = 0;

    while (tokens[i] != NULL) {
        // MIGHT NOT NEED THE METACHARS IF PROPERLY TOKENIZED
        for (k = 0; k < 4; ++k)
            if (strcmp(tokens[i], metachars[k]) == 0)
                metamask[k] = true;

        if (metamask[0] || metamask[1] || metamask[2] || metamask[3])
            break;  // JUST CHECKING FOR FIRST MASK

        

        // if((temp = strchr(tokens[i], '|')) == NULL) // temp points to NULL since to occurance of delimiter
        //     clean_tokens[++j]=tokens[i];
        // else{ // temp points to first occ of delimiter
        //     clean_tokens[++j] =tokens[i]; // fix this
        //     clean_tokens[++j] = "|";
        //     clean_tokens[++j] = temp;
        // }

        base_tokens[i] = tokens[i];
        ++i;

        // ++j;
    }

    //TODO: BREAK UP FOR METACHARS WO SPACES -> DELIMITED

    // for (k = 0; k < i; ++k) {
    //     printf("--%s--\n", base_tokens[k]);
    // }

    if (metamask[2]) {        // PIPE
        pipeHandler(tokens);  // need to pass entire args - might have multiple pipes
        return 2;             // TODO: encode return from cmd handler
    }

    return 0;
}

int main(int argc, char** argv) {
    int token_count;

    int suppress = (argc > 1) && !strcmp(argv[1], "-n");  // suppress output

    pid_t pid;
    int status;
    int retcode;

    char buffer[MAX_BUFFER];    // DON'T NEED TO MALLOC THESE - MAX SIZE GIVEN
    char* tokens[TOKEN_LIMIT];  // TODO: may not need array - might be able to dynamically allocate only size needed?

    while (true) {
        token_count = 1;
        if (!suppress)
            prompt();

        memset(buffer, '\0', MAX_BUFFER);  // TODO:not working for ctrl-D - but memory IS zeroed

        fgets(buffer, MAX_BUFFER, stdin);

        printf("buffer before: %s", buffer);

        // find indices of meta
        int i = 0;
        while (buffer[i] != '\0') {
            if (buffer[i] == '|') {
                memmove(buffer + i + 2, buffer+i+1, MAX_BUFFER - 1);
                buffer[i] = ' ', buffer[i+1]= '|';
            }
            ++i;
        }

        printf("buffer after: %s", buffer);

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