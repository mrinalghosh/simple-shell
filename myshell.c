#include "myshell.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_TOKEN 1 << 5
#define MAX_BUFFER 1 << 9
#define TOKEN_LIMIT 1 << 7  // maximum number of tokens - arbitrary choice of 128

/*
SOURCES:
https://stackoverflow.com/questions/23456374/why-do-we-use-null-in-strtok
https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
https://stackoverflow.com/questions/12935752/how-to-memset-char-array-with-null-terminating-character 
*/

/*
TODO:
detect ctrl-D = EOF
basic fork execvp REPL -- read - eval - print - loop
need to hardcode handling metachars - & > < |
*/

/*
DONE:
basic tokenizing
-n flag to suppress prompt
*/

void type_prompt(void) {
    printf("my_shell$: ");
}

int main(int argc, char** argv) {
    // suppress output
    bool suppress = (argc > 1) && !strcmp(argv[1], "-n");

    char* buffer = (char*)malloc(sizeof(char) * MAX_BUFFER);  // store line from stdin
    char* tokens[TOKEN_LIMIT];  // TODO: may not need array - might be able to dynamically allocate only size needed?
    int num_tokens;

    pid_t pid;
    int status;

    while (true) {
        if (!suppress)
            type_prompt();

        // populate and tokenize buffer - should eventually be in read_command();
        fgets(buffer, MAX_BUFFER, stdin);

        if ((tokens[0] = strtok(buffer, " \n\t\v")) == NULL)
            continue;  // do nothing if empty input

        num_tokens = 1;  // reset for new input

        while ((tokens[num_tokens] = strtok(NULL, " \n\t\v")) != NULL)
            ++num_tokens;

        char* ls_args[2];
        ls_args[0] = ".";
        ls_args[1] = 0;

        if ((pid = fork()) > 0) {
            // PARENT
            printf("Hello from parent..waiting\n");
            pid = waitpid(pid, &status, 0);
            printf("child %d exited with status %d", pid, WEXITSTATUS(status));
        } else {
            // CHILD
            // execve("/bin/ls", ls_args, 0);
            execvp("ls", ls_args);
        }

        // DONT USE FFLUSH ON STDIN - MEANT FOR OSTREAMs
    }

    // free pointers
    free(buffer);
    // free(command);
    // free(token);

    return 0;
}