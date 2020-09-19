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
    char* token = (char*)malloc(sizeof(char) * MAX_TOKEN);    //store command token

    pid_t pid;

    while (TRUE) {
        type_prompt();
        fgets(buffer, MAX_BUFFER, stdin);
        // read_command(command, parameters);
        if ((pid = fork()) > 0) {  // might not need to validate - fork arguments?
            // parent
            // waitpid(-1, &status, 0);
            printf("hello parent\n");
        } else {
            // child
            // execvp(command, parameters, 0);  // execvp doesn't need absolute path
            //need to hardcode metachars & > < |
            printf("hello child\n");
        }
    }
}