#ifndef MYSHELL_H
#define MYSHELL_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* NOT NEEDED SINCE ALL BINARY EXE NEED TO BE SUPPORTED
typedef struct {
    char* path;
    int argc;
} binexe;

enum commands {
    CAT,
    LS,
    PS,
    COMMAND_COUNT
};
*/

void type_prompt(void);
void read_command(char* command, char* parameters);
void error(char* message);

#endif /*MYSHELL_H*/