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
compound metachars
cat < x > y
error messages
*/

void nanny(int signum) {                            // child handler function
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

void command_handler(char* tokens[]) {
    /*
    ###################################
    ########----TOKEN PARSER----#######
    ###################################
    */

    int tok_c = 0, meta_c = 0, i, j, k;
    size_t row = 0, col = 0;

    char* token_array[TOKEN_LIMIT][MAX_TOKEN];  //2D array - row = {arguments...} alternated w metachars, columns = argument string

    for (i = 0; i < TOKEN_LIMIT; ++i) {
        for (j = 0; j < MAX_TOKEN; ++j) {
            token_array[i][j] = NULL;
        }
    }

    while (tokens[tok_c] != NULL) {  // get token count and parse into 2D token_array

        if (strsearch(tokens[tok_c], "|<>&", 4)) {
            ++meta_c;  // metacharacter count
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

    /* 
         _____________________________________________
        |#############################################|
        |####----MULTI-METACHARACTER EXECUTION----####|
        |#############################################|

        ASSUMPTIONS:
        -   Metacharacters can only be of the form (<) {||...||} (>) (&),
            where they are interleaved by arguments (commands or files).
        -   The 2-D array therefore has the order of rows [A M A ... A M A (&)]
        -   command </> file

    */

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

    if (row > 2 && strcmp(token_array[row - 2][0], ">") == 0)  // must have at least 3 rows (A>A)
        o_redirect = true;                                     // only one position at eol ... cmd > file (&)

    if (row > 2 && strcmp(token_array[1][0], "<") == 0)  // must have at least 3 rows (A<A)
        i_redirect = true;

    pipe_c = 0;
    for (i = 0; i < tok_c; ++i)
        if (strcmp(tokens[i], "|") == 0)
            ++pipe_c;  // pipe count

    i = 0;  // row counter (even->A, odd->M)

    while (token_array[i][0] != NULL) {  // loop over every A(args) row of token_array of form AMAMA---AMAMA
        pipe(fd);
        if ((pid = fork()) == -1) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            /* ####--Child--#### */
            if (i_redirect && i == 0) {  // i = [command] (< file) must be first command
                printf("Running input redirection\n");

                if ((filefd = open(token_array[i + 2][0], rflags, mode)) == -1) {
                    perror("open");  // assuming only one filename
                }

                if (dup2(filefd, STDIN_FILENO) == -1) {
                    perror("dup2");
                    exit(1);
                }

                if (token_array[i + 3][0] != NULL && strcmp(token_array[i + 3][0], "|") == 0) {  // next mc is a pipe after ( command < file | ... )?
                    printf("pipe next\n");
                    if (dup2(fd[1], STDOUT_FILENO) == -1) {  // TODO: not working with pipes at all
                        perror("dup2");
                        exit(1);
                    }
                }

                if (execvp(token_array[i][0], token_array[i]) < 0) {
                    perror("execvp");
                    exit(1);
                }

                close(filefd);
                close(fd[0]);
                exit(1);
            } else if (o_redirect && i == row - 3) {  // i = [command] (> file) which may or may not have a pipe before - ampersand removed
                printf("Running output redirection\n");

                if ((filefd = open(token_array[row - 1][0], wflags, mode)) == -1) {  // single file name only
                    perror("open");
                    exit(1);
                }

                if (dup2(filefd, STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(1);
                }

                if (i > 0 && strcmp(token_array[i - 1][0], "|") == 0) {  // look for pipe before on its own row
                    printf("pipe before\n");
                    dup2(fd[0], STDIN_FILENO);
                }

                if (execvp(token_array[i][0], token_array[i]) == -1) {
                    perror("execvp");
                    exit(1);
                }

                close(filefd);
                close(fd[1]);
                exit(1);
            } else {  // multiple consecutive pipes
                if (dup2(tfd, 0) == -1)
                    perror("dup2");
                if (token_array[i + 2][0] != NULL)
                    if (dup2(fd[1], 1) == -1)
                        perror("dup2");

                close(fd[0]);
                execvp(token_array[i][0], token_array[i]);
                exit(1);
            }

        } else {
            /* ####--Parent--#### */
            if (!bg) {
                if (waitpid(pid, &status, 0) == -1)
                    perror("waitpid");
            } else {
                signal(SIGCHLD, nanny);
            }
            close(fd[1]);
            tfd = fd[0];
            i += 2;
        }
    }
    return;
}

int main(int argc, char** argv) {
    int token_c, i;

    char buffer[MAX_BUFFER];
    char* tokens[TOKEN_LIMIT];

    bool silence = ((argc > 1) && !strcmp(argv[1], "-n"));  // suppress prompt for automated grading

    while (true) {
        token_c = 1;
        if (!silence)
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

        if ((tokens[0] = strtok(buffer, " \n\t\v")) == NULL)  // skip to next loop if no input
            continue;

        while ((tokens[token_c] = strtok(NULL, " \n\t\v")) != NULL)  // tokenize to 2D array
            ++token_c;

        command_handler(tokens);
    }

    return 0;
}