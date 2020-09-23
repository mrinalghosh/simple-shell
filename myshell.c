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

PIPES
cat < x > y
MULTIPLE PIPES
error message
*/

/*
DONE:
ctrl-D to quit
prompt
all tokenizing
-n flag to sup prompt
basic REPL
2D tokens
> redirection
single pipe
< - redirection of input
*/

void child_handler(int signum) {
    while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {  // -1 - for any child process
    }                                               // spin while pid != 0 (children exist but no state change) and pid != -1 (error)
    return;
}

bool strsearch(char* str, char* list, int n) {
    int i = 0;
    for (i = 0; i < n; ++i)
        if (strcmp(str, (char[2]){list[i], '\0'}) == 0) return true;
    return false;
}

bool chcmp(char ch, char* list, int n) {
    int i = 0;
    for (i = 0; i < n; ++i)
        if (ch == list[i]) return true;
    return false;
}

void prompt(void) {
    char buf[] = "my_shell$ ";
    write(STD_OUTPUT, buf, strlen(buf));
    fflush(stdout);
    return;
}

void error_message(char* message) {
    printf("ERROR: %s", message);
    return;
}

void pipe_handler(char* args1[], char* args2[], int fd[]) {
    pid_t pid[2];

    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }

    if ((pid[0] = fork()) == 0) {
        close(STDOUT_FILENO);  // explicit close stdout of first child
        dup(fd[1]);            // duplicate first child stdout to pipe stdin
        close(fd[0]);          // TODO: WHY DO WE close both sides of pipe in child
        close(fd[1]);
        execvp(args1[0], args1);
        perror("execvp");
        exit(1);
    }

    if ((pid[1] = fork()) == 0) {
        close(STDIN_FILENO);
        dup(fd[0]);
        close(fd[0]);
        close(fd[1]);
        execvp(args2[0], args2);
        perror("execvp");
        exit(1);
    }

    // PROPERLY close pipe fd in parent
    close(fd[0]);
    close(fd[1]);

    // wait for both child processes under group to terminate
    wait(0);
    wait(0);

    return;
}

void command_handler(char* tokens[]) {
    int tok_c = 0, meta_c = 0, i, j, k;
    size_t row = 0, col = 0;

    char* token_array[TOKEN_LIMIT][MAX_TOKEN];  //2D array - row = [arguments...] bw metachars - columns = argument

    metachar* metachars = malloc(TOKEN_LIMIT * sizeof(metachar));  // array of indexes, type and fd[2] of metacharacters in order

    for (i = 0; i < TOKEN_LIMIT; ++i) {
        for (j = 0; j < MAX_TOKEN; ++j) {
            token_array[i][j] = NULL;
        }
    }

    while (tokens[tok_c] != NULL) {  // get token count and put into 2D token_array

        if (strsearch(tokens[tok_c], "|<>&", 4)) {
            metachars[meta_c].index = row + 1;       // index of metacharacter
            metachars[meta_c].type = tokens[tok_c];  // pointer to metacharacter - string
            ++meta_c;                                // METACHARACTER COUNT
            col = 0;

            if (tok_c != 0)
                ++row;

            token_array[row][col] = malloc(MAX_TOKEN * sizeof(char));
            memcpy(token_array[row][col], tokens[tok_c], MAX_TOKEN);

            if (tokens[tok_c + 1] != NULL && !strsearch(tokens[tok_c + 1], "|<>&", 4))
                ++row;

        } else {
            token_array[row][col] = malloc(MAX_TOKEN * sizeof(char));
            memcpy(token_array[row][col], tokens[tok_c], MAX_TOKEN);
            ++col;
        }
        ++tok_c;  // token count;
    }
    ++row;  // row index -> row count

    /* ####----MULTI-METACHARACTER EXECUTION----#### */

    // ASSUMPTIONS:     metacharacters can only be in the valid form (<) (||...||) (>) (&)
    //                  the resulting token_array is of the form amama....amamama (&) where a = args
    //                  command { <, > } filename

    bool bg = false, i_redirect = false, o_redirect = false;  // background, input redirect, output redirect
    int filefd, status, pipe_c, tfd = 0;                      // file fd, status, pipe count, temp fd=0 initially for stdin
    int fd[2];                                                // pipe file descriptors
    pid_t pid;                                                // only one pid/fork at a time

    int wflags = O_WRONLY | O_CREAT | O_TRUNC;            // write flags
    int rflags = O_RDONLY;                                // read flag
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;  // user permissions flags

    // background tasks
    if (row > 1 && strcmp(token_array[row - 1][0], "&") == 0) {  // must have at least 2 rows (A&)
        bg = true;
        token_array[row - 1][0] = NULL;  // remove "&" from last row of token_array - now of form AMA....MAMA
        --row;
        --meta_c;
        --tok_c;
    }

    if (row > 2 && strcmp(token_array[row - 2][0], ">") == 0) {  // must have at least 3 rows (A>A)
        o_redirect = true;                                       // only one position at eol ... cmd > file (&)
    }

    if (row > 2 && strcmp(token_array[1][0], "<") == 0) {  // must have at least 3 rows (A<A)
        i_redirect = true;
    }

    // initialize pipes - DO NOT DO THIS OUTSIDE THE WHILE LOOP
    pipe_c = 0;
    for (i = 0; i < meta_c; ++i) {
        if (strcmp(metachars[i].type, "|") == 0) {
            // if (pipe(metachars[i].fd) < 0) {
            //     perror("ERROR: ");
            // }
            ++pipe_c;  // pipe count
        }
    }

    i = 0;  // row counter (even->A, odd->M)
    j = 0;  // metacharacter counter
    k = 0;  // counter to print tokens

    while (token_array[i][0] != NULL) {  // loop over rows of token_array of form AMAMA---AMAMA - just the A=arguments

        // print all tokens
        // while (token_array[i][k] != NULL) {
        //     printf("\"%s\"  ", token_array[i][k]);
        //     ++k;
        // }
        // printf("\n-----\n");
        // k = 0;

        // printf("pipe_c: %d, row_c: %d\n", pipe_c, row);  // print number of pipes and rows

        pipe(fd);
        if ((pid = fork()) == -1) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            /* Child */
            if (i_redirect && i == 0) {  // i = [command] (< file) must be first command
                printf("Running input redirection\n");

                filefd = open(token_array[i + 2][0], rflags, mode);  // assuming only one filename
                dup2(filefd, STDIN_FILENO);

                if (token_array[i + 3][0] != NULL && strcmp(token_array[i + 3][0], "|") == 0) {  // next mc is a pipe after ( command < file | ... )
                    printf("pipe next\n");
                    if (dup2(fd[1], STDOUT_FILENO) == -1)
                        perror("dup2");
                }

                if (execvp(token_array[i][0], token_array[i]) < 0)
                    perror("execvp");

                close(filefd);
                close(fd[0]);
                exit(1);
            } else if (o_redirect && i == tok_c - 3) {  // i = [command] (> file) which may or may not have a pipe before - ampersand removed
                printf("Running output redirection\n");

                if (filefd = open(token_array[i + 2][0], wflags) == -1)
                    perror("open");

                if (dup2(filefd, STDOUT_FILENO))
                    perror("dup2");

                if (i > 0 && strcmp(token_array[i - 1][0], "|") == 0) {  // look for pipe before
                    printf("pipe before\n");
                    dup2(fd[0], STDIN_FILENO);
                }

                if (execvp(token_array[i][0], token_array[i]) == -1)
                    perror("execvp");

                close(filefd);
                close(fd[1]);
                exit(1);
            } else {  // multiple consecutive pipes
                if (dup2(tfd, 0) == -1)
                    perror("ERROR: ");
                if (token_array[i + 2][0] != NULL)
                    dup2(fd[1], 1);
                close(fd[0]);
                execvp(token_array[i][0], token_array[i]);
                exit(1);
            }

        } else {
            /* Parent */
            if (!bg) {
                waitpid(pid, &status, 0);
            } else {
                signal(SIGCHLD, child_handler);
            }
            close(fd[1]);
            tfd = fd[0];  // TODO: what does this line do??? - from quora
            i += 2;
        }

        // if ((pid = fork()) == -1) {
        //     perror("ERROR: ");
        //     exit(1);
        // } else if (pid > 0) {
        //     /* ---Parent--- */

        //     // printf("henlo from papa\n");

        //     if (!bg) {
        //         pid = waitpid(pid, &status, 0);
        //     } else {
        //         signal(SIGCHLD, child_handler);
        //     }

        // } else {
        //     /* ---Child--- */

        //     // printf("i am litle baby from i:%d j:%d\n", i, j);

        //     if (row == 1) {  // exception for single command without metacharacters
        //         // printf("Running single command\n");
        //         if (execvp(token_array[0][0], token_array[0]) < 0) {
        //             perror("ERROR: ");
        //         }
        //         exit(0);
        //     }

        //     if (strcmp(token_array[i][0], "<") == 0) {  // command < file
        //                                                 // TODO: special case of cmd < file | cmd and cmd < file > file

        //         // printf("Running input redirection\n");
        //         ++j;                                        // increment metacharacter array
        //         filefd = open(token_array[i + 1][0], rflags);  // assuming only one file can be redirected
        //         dup2(filefd, STDIN_FILENO);

        //         if (execvp(token_array[i - 1][0], token_array[i - 1]) < 0)
        //             perror("ERROR: ");

        //         close(filefd);
        //         exit(0);
        //     }

        //     if (strcmp(token_array[i][0], ">") == 0) {  // command > file

        //         // printf("Running output redirection\n");
        //         ++j;  // increment metacharacter array
        //         filefd = open(token_array[i + 1][0], wflags);
        //         dup2(filefd, STDOUT_FILENO);

        //         if (execvp(token_array[i - 1][0], token_array[i - 1]) < 0)
        //             perror("ERROR: ");

        //         close(filefd);
        //         exit(0);
        //     }

        //     if ((i - 1 >= 0) && (strcmp(token_array[i - 1][0], "|") == 0)) {  // LEFT PIPE (| args)
        //         // printf("Pipe to the left of command (index %d)... reading from pipe", i);

        //         // if (dup2(metachars[j].fd[0], STDIN_FILENO) == -1)
        //         //     perror("ERROR: ");

        //         // if (execvp(token_array[i + 1][0], token_array[i + 1]) < 0)
        //         //     perror("ERROR: ");

        //         // close(metachars[j].fd[0]);
        //         // close(metachars[j].fd[1]);
        //         // // ++j;  //increment metachars

        //         exit(0);
        //     }

        //     if ((i + 1 < tok_c) && strcmp(token_array[i + 1][0], "|") == 0) {  // RIGHT PIPE (args |)
        //         // execute args after setting output
        //         // printf("Pipe on the right of command (index %d)... writing to pipe", i);

        //         // if (dup2(metachars[j].fd[1], STDOUT_FILENO) == -1)
        //         //     perror("ERROR: ");

        //         // if (execvp(token_array[i - 1][0], token_array[i - 1]) < 0)
        //         //     perror("ERROR: ");

        //         // close(metachars[0].fd[0]);
        //         // close(metachars[0].fd[1]);
        //         // ++j;  // increment metachars only when it's next

        //         // exit(0);
        //         // pipe_handler(token_array[i], token_array[i + 2], metachars[0].fd);
        //         exit(0);
        //     }

        //     // TODO: DO | cmd |

        //     // printf("Tokens starting with %s run through loop", token_array[i][0]);
        //     exit(0);
        // }
        // printf("Read tokens starting with: \"%s\"", token_array[i][0]);

        // i += 2;  // increment to two rows down (args->(meta)->args...)
    }
    return;
}

int main(int argc, char** argv) {
    int token_c;
    int i;

    char buffer[MAX_BUFFER];
    char* tokens[TOKEN_LIMIT];

    bool sup = ((argc > 1) && !strcmp(argv[1], "-n"));  // supress prompt for automated grading

    while (true) {
        token_c = 1;
        if (!sup)
            prompt();

        memset(buffer, '\0', MAX_BUFFER);
        fgets(buffer, MAX_BUFFER, stdin);

        if (buffer[0] == '\0') {  // ctrl-D
            printf("\n");
            exit(0);
        }

        i = 0;
        while (buffer[i] != '\0') {
            if ((buffer[i - 1] != ' ') && chcmp(buffer[i], "|&<>", 4)) {  // no space before
                memmove((buffer + i + 1), (buffer + i), sizeof(buffer) - i);
                buffer[i] = ' ';
            }
            if ((buffer[i + 1] != ' ') && chcmp(buffer[i], "|&<>", 4)) {  // no space after
                memmove((buffer + i + 2), (buffer + i + 1), sizeof(buffer) - i - 1);
                buffer[i + 1] = ' ';
            }
            ++i;
        }

        if ((tokens[0] = strtok(buffer, " \n\t\v")) == NULL) {  // skip to next loop if no input
            continue;
        }

        while ((tokens[token_c] = strtok(NULL, " \n\t\v")) != NULL)  // tokenize to 2D array
            ++token_c;

        command_handler(tokens);
    }

    return 0;
}