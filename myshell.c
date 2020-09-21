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

bool charCompare(char* str, char* list) {
    int i = 0;
    for (i = 0; i < strlen(str); ++i)
        if (strcmp(str, (char[2]){list[i], '\0'}) == 0)
            return true;

    return false;
}

void prompt(void) {
    char buf[] = "my_shell$ ";
    write(STD_OUTPUT, buf, strlen(buf));
}

void fileHandler(char* tokens[], char* input_file, char* output_file, int io_opt) {
    /* general purpose I/O handling - tokens include  */

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
        /* Child */
        if (io_opt == STD_INPUT) {
            fd = open(output_file, wflags, mode);  // TODO: does the environment variable need to be concatenated?
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

int pipeHandler(char* base[], char* aux[]) {
    // TODO: THIS DOESN'T WORK FUCK
    int fd[2];  // TODO: multiple file descriptors to handle multiple pipes
    pid_t pid;
    int status;

    pipe(fd);

    // perror("pipe") exit(1);
    // exit(1);

    pid = fork();

    char* ls_args[2];
    switch (pid) {
        case 0:  //child
            ls_args[0] = "ls";
            ls_args[1] = "-a";
            execvp(ls_args[0], ls_args);
            exit(0);
        case -1:
            perror("fork");
            exit(1);
        default:  //parent
            while ((pid = wait(&status)) != -1) {
                fprintf(stderr, "process %d exits with %d\n",
                        pid,
                        WEXITSTATUS(status));
                break;
            }
    }
    exit(0);

    // if (fork() == 0) {  // child1
    //     dup2(fd[1], STDOUT_FILENO);
    //     close(fd[0]);
    //     close(fd[1]);

    //     char* ls_args[2] = {"ls", "-a"};
    //     execvp(ls_args[0], ls_args);
    //     perror("Child1 failed");
    //     exit(1);
    // }

    // close(fd[0]);
    // close(fd[1]);
    // wait(0);
    // wait(0);

    // if ((pid = fork()) > 0) {
    //     /* Parent - close fd1 */
    //     printf("Hello from the parent\n");
    //     dup2(STD_OUTPUT, fd[1]);
    //     // execvp(base[0], base);
    //     char* ls_args[2] = {"ls", "."};
    //     if (execvp(ls_args[0], ls_args) == -1) printf("LS FAILED\n");

    // } else {
    //     /* Child - close fd0 */
    //     printf("Hello from the child\n");
    //     dup2(STD_INPUT, fd[0]);
    //     // execvp(aux[0], aux);
    //     char* cat_args[1] = {"cat"};
    //     if (execvp(cat_args[0], cat_args) == -1) printf("CAT FAILED\n");
    // }

    return 0;  // TODO: retcodes?
}

int commandHandler(char* tokens[]) {
    int i = 0, j = 0, k;

    char* base[TOKEN_LIMIT];  // left of metachar i
    char* aux[TOKEN_LIMIT];   // right of metachar i TODO: implement iteratively instead of hardcoding just the first

    for (k = 0; k < TOKEN_LIMIT; ++k) {
        base[k] = malloc(MAX_TOKEN);
        aux[k] = malloc(MAX_TOKEN);
    }

    metachar* metachars = malloc(TOKEN_LIMIT * sizeof(metachar));  // array of indexes and type of metacharacters in order - FUNCTIONAL

    while (tokens[i] != NULL) {  // get token count and assign to new string
        if (charCompare(tokens[i], "|<>&")) {
            metachars[j].index = i;         // index of metacharacter
            metachars[j].type = tokens[i];  // pointer to metacharacter - string
            ++j;                            // METACHARACTER COUNT
        }
        ++i;  // TOKEN COUNT;
    }

    printf("number of tokens: %d, number of metachars: %d", i, j);
    printf("MC type, index: %s, %d", metachars[0].type, metachars[0].index);

    pid_t pid;
    int status;

    if (j == 0) { /* NO METACHARACTERS */
        if ((pid = fork()) > 0) {
            /* Parent */
            printf("Hello from parent..waiting\n");
            pid = waitpid(pid, &status, 0);
            printf("child %d exited with status %d\n", pid, WEXITSTATUS(status));
        } else {
            /* Child */
            execvp(tokens[0], tokens);
        }
        return 0;
    }

    /* break into arrays of strings between metachars - can use to execvp */
    if (charCompare(metachars[0].type, "|<>&")) {
        for (k = 0; k < metachars[0].index; ++k) {
            memcpy(base[k], tokens[k], MAX_TOKEN);
            printf("BASE %d %s\n", k, base[k]);
        }

        for (k = metachars[0].index + 1; k < i; ++k) {
            memcpy(aux[k - metachars[0].index - 1], tokens[k], MAX_TOKEN);
            printf("AUX %d %s\n", k - metachars[0].index - 1, aux[k - metachars[0].index - 1]);
        }

        // printf("size of base: %d", sizeof(base)/sizeof(base[0]));
        // printf("size of aux: %d", sizeof(aux)/sizeof(aux[0]));

        // pipeHandler(base, aux);  // TODO: fix piping

        char* inputfile = "LICENSE";
        char* outputfile = "testout";
        fileHandler(base, inputfile, outputfile, STD_INPUT);  //TODO: move this to appropriate place after testing io
    }

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