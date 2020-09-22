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
    < - redirection of input !!!not working
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
> redirection
*/

bool strCompare(char* str, char* list, int n) {
    int i = 0;
    for (i = 0; i < n; ++i)
        if (strcmp(str, (char[2]){list[i], '\0'}) == 0) return true;
    return false;
}

bool charCompare(char ch, char* list, int n) {
    int i = 0;
    for (i = 0; i < n; ++i)
        if (ch == list[i]) return true;
    return false;
}

void prompt(void) {
    char buf[] = "my_shell$ ";
    write(STD_OUTPUT, buf, strlen(buf));
    fflush(stdout);
}

void execute(char* args[], char* filename, int options, int pfd[]) {
    /* general and meta execution function
    options: no file=0, input from file(<)=1, output to file(>)=2 
    */

    printf("fd: %d %d\n", pfd[0], pfd[1]);

    // int wflags = O_WRONLY | O_CREAT | O_TRUNC;
    // int rflags = O_RDONLY;
    // mode_t mode = S_IRUSR | S_IWUSR;

    // int ffd, status, nread;
    // pid_t pid;
    // char fbuf[MAX_FILE];

    // if ((pid = fork()) == -1) {
    //     perror("fork");
    // } else if (pid > 0) {
    //     /* Parent */
    //     // printf("Parental guidance.. waiting\n");
    //     pid = waitpid(pid, &status, 0);
    //     // printf("Child %d exited with status %d\n", pid, WEXITSTATUS(status));
    //     return;
    // } else {
    //     /* Child */
    //     switch (options) {
    //         case 0: {  // no file - single commands
    //             execvp(args[0], args);
    //             break;
    //         }
    //         case 1: {  // command < file
    //             printf("Child command < file\n");
    //             ffd = open(filename, rflags);
    //             nread = read(ffd, fbuf, MAX_FILE);
    //             dup2(ffd, STDIN_FILENO);
    //             write(ffd, fbuf, MAX_FILE);

    //             execvp(args[0], args);

    //             close(ffd);
    //             break;
    //         }
    //         case 2: {  //command > file
    //             printf("Child command > file\n");
    //             ffd = open(filename, wflags);
    //             dup2(ffd, STDOUT_FILENO);

    //             execvp(args[0], args);

    //             close(ffd);
    //             break;
    //         }
    //         default: {
    //             break;
    //         }
    //     }

    //     exit(0);
    // }
    return;
}

int commandHandler(char* tokens[]) {
    int tok_c = 0, meta_c = 0, i, j;
    size_t row = 0, col = 0;

    char* token_array[TOKEN_LIMIT][MAX_TOKEN];  //2D array - row = [arguments...] bw metachars - columns = argument

    metachar* metachars = malloc(TOKEN_LIMIT * sizeof(metachar));  // array of indexes, type and fd[2] of metacharacters in order

    for (i = 0; i < TOKEN_LIMIT; ++i) {
        for (j = 0; j < MAX_TOKEN; ++j) {
            token_array[i][j] = NULL;
        }
    }

    while (tokens[tok_c] != NULL) {  // get token count and assign to new string

        if (strCompare(tokens[tok_c], "|<>&", 4)) {
            metachars[meta_c].index = row + 1;       // index of metacharacter
            metachars[meta_c].type = tokens[tok_c];  // pointer to metacharacter - string
            ++meta_c;                                // METACHARACTER COUNT

            col = 0;

            if (tok_c != 0)
                ++row;

            token_array[row][col] = malloc(MAX_TOKEN * sizeof(char));
            memcpy(token_array[row][col], tokens[tok_c], MAX_TOKEN);

            if (tokens[tok_c + 1] != NULL && !strCompare(tokens[tok_c + 1], "|<>&", 4))
                ++row;

        } else {
            token_array[row][col] = malloc(MAX_TOKEN * sizeof(char));
            memcpy(token_array[row][col], tokens[tok_c], MAX_TOKEN);
            ++col;
        }

        ++tok_c;  // token count;
    }

    ++row;  // row index -> count

    /* DEBUG OUTPUT FOR TOKENIZING
    // print 2D array
    printf("rows: %d\n", row);
    for (i = 0; i < row; ++i) {
        j = 0;
        while (token_array[i][j] != NULL) {
            printf("\"%s\"  ", token_array[i][j]);
            ++j;
        }
        printf("\n");
    }

    // print metacharacters
    for (i = 0; i < meta_c; ++i)
        printf("metacharacter has index: %d, type: \"%s\"\n", metachars[i].index, metachars[i].type);
    */

    /* ####----COMMAND EXECUTION----#### */

    pid_t pid;  // only making one fork at a time
    int fd, nread, status, pipe_c = 0;
    char* filename = malloc(MAX_TOKEN * sizeof(char));
    char fbuf[MAX_FILE];

    for (i = 0; i < meta_c; ++i)  // init pipes
        if (strcmp(metachars[i].type, "|") == 0) {
            pipe(metachars[i].fd);  // metachar struct contains fd[2]
            ++pipe_c;               // pipe count
        }

    i = 0;  // row counter
    j = 0;  // metacharacter counter

    while (token_array[i][0] != NULL) {
        // ASSUMPTIONS:
        // - every special character besides & has args on the right and left -> can index ahead or behind
        // - command { <, > } filename is only form of redirection

        /* SINGLE COMMANDS */
        if (row == 1)
            execute(token_array[0], NULL, 0, NULL);

        /* METACHARACTER HANDLING */
        if (strcmp(token_array[i][0], "|") == 0) {
            printf("mc fd: %d %d\n", metachars[i].fd[0], metachars[i].fd[1]);
            execute(token_array[i], NULL, 3, metachars[i].fd);
        }

        if (strcmp(token_array[i][0], "<") == 0) {
            strcpy(filename, token_array[i + 1][0]);
            execute(token_array[i - 1], filename, 1, NULL);
        }

        if (strcmp(token_array[i][0], ">") == 0) {
            strcpy(filename, token_array[i + 1][0]);
            execute(token_array[i - 1], filename, 2, NULL);
        }
        if (strcmp(token_array[i][0], "&") == 0) {
        }

        ++i;
    }

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