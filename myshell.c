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
// #define EXIT_SIGNAL '' //detect ctrl-D

void type_prompt(void) {
    printf("my_shell$: ");
}

// static bool validate(char* token){

// }

// void error_message(void) {
// }

int main(int argc, char** argv) {
    char* buffer = (char*)malloc(sizeof(char) * MAX_BUFFER);  //store line
    char* command = (char*)malloc(sizeof(char) * MAX_TOKEN);  //store command token
    char* token = (char*)malloc(sizeof(char) * MAX_TOKEN);    // TODO: does this need to be smaller?

    pid_t pid;
    int wstatus;

    while (TRUE) {
        type_prompt();

        // populate and tokenize buffer - should eventually be in read_command();
        fgets(buffer, MAX_BUFFER, stdin);

        token = strtok(buffer, " ");
        while (token != NULL) {
            printf("%s__TEST\n", token);
            token = strtok(NULL, " ");  // returns
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
    }

    // free pointers
    free(buffer);
    free(command);
    free(token);

    return 0;
}