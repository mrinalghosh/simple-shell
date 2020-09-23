#ifndef MYSHELL_H
#define MYSHELL_H

#define MAX_TOKEN (1ULL << 5)    // maximum token size
#define MAX_BUFFER (1ULL << 9)   // maximum buffer size
#define TOKEN_LIMIT (1ULL << 7)  // maximum number of tokens - arbitrary choice of 128
#define STD_INPUT 0
#define STD_OUTPUT 1
#define STD_ERROR 2

void prompt(void);
bool chcmp(char ch, char* list, int n);
bool strsearch(char* str, char* list, int n);
void nanny(int signum);
void command_handler(char* tokens[]);

#endif /*MYSHELL_H*/