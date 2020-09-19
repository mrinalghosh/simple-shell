#include "myshell.h"

#define TRUE 1
#define MAX_TOKEN 1 << 5
#define MAX_BUFFER 1 << 9

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
    // char* tokens = (char*)malloc(sizeof(char) * MAX_BUFFER);  // TODO: does this need to be smaller?
    char* tokens;

    pid_t pid;

    while (TRUE) {
        type_prompt();
        fgets(buffer, MAX_BUFFER, stdin);
        printf("%s\n", buffer);

        // tokenize buffer - should eventually be in read_command(command, parameters);
        tokens = strtok(buffer, " ");
        while (tokens != NULL) {
            printf(" %s\n", tokens);

            tokens = strtok(NULL, s);
        }

        // if ((pid = fork()) > 0) {  // might not need to validate - fork arguments?
        //     // parent
        //     // waitpid(-1, &status, 0);
        //     printf("hello parent\n");
        // } else {
        //     // child
        //     // execvp(command, parameters, 0);  // execvp doesn't need absolute path
        //     //need to hardcode metachars & > < |

        //     // printf("%s hello child\n", buffer);
        //     // printf("%s \n", buffer)
        // }
    }

    // free pointers
    free(buffer);
    free(token);

    return 0;
}