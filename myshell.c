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
-n flag to suppress prompt
basic REPL
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

// void fileHandler(char* tokens[], char* input_file, char* output_file, int io_opt) {
//     /* general purpose I/O handling - tokens include  */

//     int fd;  // file descriptor
//     pid_t pid;
//     int status;
//     int wflags = O_WRONLY | O_CREAT | O_TRUNC;
//     int rflags = O_RDONLY;
//     mode_t mode = S_IRUSR | S_IWUSR;

//     if ((pid = fork()) == -1) {
//         printf("Error - child could not be created\n");
//         return;  // TODO: exit vs return on fork fail?
//     }

//     if (pid == 0) {
//         /* Child */
//         if (io_opt == STD_INPUT) {
//             fd = open(output_file, rflags, mode);  // TODO: does the environment variable need to be concatenated?
//             dup2(fd, STDOUT_FILENO);               // duplicate fd to stdout
//             close(fd);
//         } else if (io_opt == STD_OUTPUT) {
//             fd = open(input_file, rflags, mode);
//             dup2(fd, STDIN_FILENO);
//             close(fd);
//         }

//         // TODO: need to handle case with both I and O

//         execvp(tokens[0], tokens);  // TODO: handle signal - kill if errors - so doesn't overwrite file
//     }
//     waitpid(pid, &status, 0);  // wait for any child process in group
// }

// int pipeHandler(char* base[], char* aux[]) {
//     // TODO: THIS DOESN'T WORK
//     int fd[2];  // TODO: multiple file descriptors to handle multiple pipes
//     pid_t pid;
//     int status;

//     pipe(fd);

//     // perror("pipe") exit(1);
//     // exit(1);

//     return 0;  // TODO: retcodes?
// }

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
            
            ++row;

            token_array[row][col] = malloc(MAX_TOKEN * sizeof(char));
            memcpy(token_array[row][col], tokens[tok_c], MAX_TOKEN);

            ++row;  // assuming can never start with a pipe

        } else {
            token_array[row][col] = malloc(MAX_TOKEN * sizeof(char));
            memcpy(token_array[row][col], tokens[tok_c], MAX_TOKEN);
            ++col;
        }

        ++tok_c;  // TOKEN COUNT;
    }

    ++row;
    printf("rows: %d\n", row);

    for (i = 0; i < row; ++i) {
        j = 0;
        while (token_array[i][j] != NULL) {
            printf("\"%s\"  ", token_array[i][j]);
            ++j;
        }
        printf("\n");
    }

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

    /* break into arrays of strings between metachars - can use to execvp */
    // if (charCompare(metachars[0].type, "|<>&", 4)) {
    //     // if(tokens[])
    //     for (k = 0; k < metachars[0].index; ++k) {
    //         base[k] = malloc(MAX_TOKEN);
    //         memcpy(base[k], tokens[k], MAX_TOKEN);
    //         printf("BASE %d %s\n", k, base[k]);
    //     }

    //     for (k = metachars[0].index + 1; k < i; ++k) {
    //         memcpy(aux[k - metachars[0].index - 1], tokens[k], MAX_TOKEN);
    //         printf("AUX %d %s\n", k - metachars[0].index - 1, aux[k - metachars[0].index - 1]);
    //     }

    //     // printf("size of base: %d", sizeof(base)/sizeof(base[0]));
    //     // printf("size of aux: %d", sizeof(aux)/sizeof(aux[0]));

    //     // pipeHandler(base, aux);  // TODO: fix piping

    //     char* inputfile = "LICENSE";
    //     char* outputfile = "testout";
    //     fileHandler(base, inputfile, outputfile, STD_INPUT);  //TODO: move this to appropriate place after testing io
    // }

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