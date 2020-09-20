#include "myshell.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define TRUE 1
#define MAX_TOKEN 1 << 5
#define MAX_BUFFER 1 << 9
#define TOKEN_LIMIT 1 << 7  // maximum number of tokens - arbitrary choice of 128
// #define EXIT_SIGNAL '' //detect ctrl-D

/*
SOURCES:
https://stackoverflow.com/questions/23456374/why-do-we-use-null-in-strtok
https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
https://stackoverflow.com/questions/12935752/how-to-memset-char-array-with-null-terminating-character 
*/

void type_prompt(void) {
    printf("my_shell$: ");
}

// static bool validate(char* token){

// }

// void error_message(void) {
// }

// REPL - read - eval - print - loop
// has to handle metachars like > (pass output) & (doesn't wait for forked process) | (pipe - connect stdin stdout)

int main(int argc, char** argv) {
    char* buffer = (char*)malloc(sizeof(char) * MAX_BUFFER);  // store line
    // char* command = (char*)malloc(sizeof(char) * MAX_TOKEN);  // store command token
    // char* token = (char*)malloc(sizeof(char) * MAX_TOKEN);    // TODO: does this need to be smaller?
    char* tokens[TOKEN_LIMIT];  // TODO: may not need array - might be able to dynamically allocate only size needed?
    int num_tokens;

    pid_t pid;
    int wstatus;

    while (TRUE) {
        type_prompt();

        // populate and tokenize buffer - should eventually be in read_command();
        fgets(buffer, MAX_BUFFER, stdin);

        if ((tokens[0] = strtok(buffer, " \n\t\v")) == NULL)
            continue;  // do nothing if empty input

        num_tokens = 1;  // reset for new input

        while ((tokens[num_tokens] = strtok(NULL, " \n\t\v")) != NULL)
            num_tokens++;

        for (int i = 0; i < num_tokens; ++i) {  //test for proper tokenizing
            printf("%s\n", tokens[i]);
        }

        // pid = fork();
        // if (pid == 0) {
        //     // child
        //     printf("Hello from child\n");
        //     exit(42);
        // } else if (pid > 0) {
        //     // parent
        //     printf("Hello from parent..waiting\n");
        //     pid = waitpid(pid, &wstatus, 0);
        //     printf("child %d exited with status %d", pid, WEXITSTATUS(wstatus));
        // } else {
        //     printf("something went wrong\n");
        //     perror("error: ");
        // }

        // execvp(command, parameters, 0);  // execvp doesn't need absolute path
        //need to hardcode metachars & > < |

        // DONT USE FFLUSH ON STDIN - MEANT FOR OSTREAMs
    }

    // free pointers
    free(buffer);
    // free(command);
    // free(token);

    return 0;
}