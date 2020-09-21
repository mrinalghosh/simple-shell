#include "myshell.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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
-n flag to sup prompt
basic REPL
2D tokens
*/

bool charCompare(char* str, char* list, int n) {
    int i = 0;
    for (i = 0; i < n; ++i)
        if (strcmp(str, (char[2]){list[i], '\0'}) == 0)
            return true;

    return false;
}

void prompt(void) {
    char buf[] = "my_shell$ ";
    write(STD_OUTPUT, buf, strlen(buf));
    fflush(stdout);
}

void executeOne() {}

/*
void fileHandler(char* tokens[], char* input_file, char* output_file, int io_opt) {
    // general purpose I/O handling - tokens include  

    int fd;  // file descriptor
    pid_t pid;
    int status;
    int wflags = O_WRONLY | O_CREAT | O_TRUNC;
    int rflags = O_RDONLY;
    mode_t mode = S_IRUSR | S_IWUSR;

    if ((pid = fork()) == -1) {
        printf("Error - child could not be created\n");
        return;  // TODO: exit vs return on fork fail?
    }

    if (pid == 0) {
        //  Child 
        if (io_opt == STD_INPUT) {
            fd = open(output_file, rflags, mode);  // TODO: does the environment variable need to be concatenated?
            dup2(fd, STDOUT_FILENO);               // duplicate fd to stdout
            close(fd);
        } else if (io_opt == STD_OUTPUT) {
            fd = open(input_file, rflags, mode);
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // TODO: need to handle case with both I and O

        execvp(tokens[0], tokens);  // TODO: handle signal - kill if errors - so doesn't overwrite file
    }
    waitpid(pid, &status, 0);  // wait for any child process in group
}
*/

// int pipeHandler(){}

int commandHandler(char* tokens[]) {
    int tok_c = 0, meta_c = 0, i, j;

    char* token_array[TOKEN_LIMIT][MAX_TOKEN];  //2D array - row = set of arguments bw metachars - columns = "argument"

    for (i = 0; i < TOKEN_LIMIT; ++i) {
        for (j = 0; j < MAX_TOKEN; ++j) {
            token_array[i][j] = NULL;
        }
    }

    metachar* metachars = malloc(TOKEN_LIMIT * sizeof(metachar));  // array of indexes and type of metacharacters in order - FUNCTIONAL
    size_t row = 0, col = 0;

    while (tokens[tok_c] != NULL) {  // get token count and assign to new string

        if (charCompare(tokens[tok_c], "|<>&", 4)) {
            metachars[meta_c].index = tok_c;         // index of metacharacter
            metachars[meta_c].type = tokens[tok_c];  // pointer to metacharacter - string
            ++meta_c;                                // METACHARACTER COUNT

            col = 0;

            if (tok_c != 0)
                ++row;

            token_array[row][col] = malloc(MAX_TOKEN * sizeof(char));
            memcpy(token_array[row][col], tokens[tok_c], MAX_TOKEN);

            if (tokens[tok_c + 1] != NULL)
                ++row;

        } else {
            token_array[row][col] = malloc(MAX_TOKEN * sizeof(char));
            memcpy(token_array[row][col], tokens[tok_c], MAX_TOKEN);
            ++col;
        }

        ++tok_c;  // TOKEN COUNT;
    }

    ++row;  // ROW index -> count

    /* // test print for array
    printf("rows: %d\n", row);

    for (i = 0; i < row; ++i) {
        j = 0;
        while (token_array[i][j] != NULL) {
            printf("\"%s\"  ", token_array[i][j]);
            ++j;
        }
        printf("\n");
    }
    */

    // pid_t pid;
    // int status;

    // if (j == 0) { /* NO METACHARACTERS */
    //     if ((pid = fork()) > 0) {
    //         /* Parent */
    //         printf("Hello from parent..waiting\n");
    //         pid = waitpid(pid, &status, 0);
    //         printf("child %d exited with status %d\n", pid, WEXITSTATUS(status));
    //     } else {
    //         /* Child */
    //         execvp(tokens[0], tokens);
    //     }
    //     return 0;
    // }

    return 0;
}

int main(int argc, char** argv) {
    int token_c;
    int i;

    char buffer[MAX_BUFFER];    // DON'T NEED TO MALLOC THESE - MAX SIZE GIVEN
    char* tokens[TOKEN_LIMIT];  // TODO: may not need array - might be able to dynamically allocate only size needed?

    bool sup = ((argc > 1) && !strcmp(argv[1], "-n"));  // supress prompt for automated grading

    while (true) {
        token_c = 1;
        if (!sup)
            prompt();

        memset(buffer, '\0', MAX_BUFFER);
        fgets(buffer, MAX_BUFFER, stdin);

        i = 0;
        while (buffer[i] != '\0') {
            if ((buffer[i - 1] != ' ') && charCompare(buffer[i], "|&<>", 4)) {  // no space before
                memmove((buffer + i + 1), (buffer + i), sizeof(buffer) - i);
                buffer[i] = ' ';
            }
            if ((buffer[i + 1] != ' ') && charCompare(buffer[i], "|&<>", 4)) {  // no space after
                memmove((buffer + i + 2), (buffer + i + 1), sizeof(buffer) - i - 1);
                buffer[i + 1] = ' ';
            }
            ++i;
        }

        if ((tokens[0] = strtok(buffer, " \n\t\v")) == NULL)
            continue;

        while ((tokens[token_c] = strtok(NULL, " \n\t\v")) != NULL)
            ++token_c;

        commandHandler(tokens);
    }

    return 0;
}