#ifndef MYSHELL_H
#define MYSHELL_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>

void type_prompt(void);
// void read_command(char* command, char* parameters);
void error(char* message);

#endif /*MYSHELL_H*/