#include "myshell.h"

#define TRUE 1
#define MAX_TOKEN 1 << 5
#define MAX_BUFFER 1 << 9

void type_prompt(void) {
    printf("my_shell$: ")
}

static bool validate(char* token){

}

void error_message(void) {
}

int main(int argc, char** argv) {
    char* buffer = malloc(sizeof(char) * MAX_BUFFER);  //store line
    char* token = malloc(sizeof(char) * MAX_TOKEN);    //store command token

    while (TRUE) {
        type_prompt();
        fgets(buffer, MAX_BUFFER, stdin);
        // read_command(command, parameters);
        if (validate(token) & !fork()) {  // fork arguments?
            // parent
            // waitpid(-1, &status, 0);
        } else {
            // child
            // execvp(command, parameters, 0);  // execvp doesn't need absolute path
            //need to hardcode metachars & > < |
        }
    }
}