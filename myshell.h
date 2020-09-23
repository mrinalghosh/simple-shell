#ifndef MYSHELL_H
#define MYSHELL_H

#define MAX_TOKEN (1ULL << 5)    // maximum token size
#define MAX_BUFFER (1ULL << 9)   // maximum buffer size
#define TOKEN_LIMIT (1ULL << 7)  // maximum number of tokens - arbitrary choice of 128
#define MAX_FILE (1ULL << 12)    // 4k file buffer size
#define STD_INPUT 0
#define STD_OUTPUT 1
#define STD_ERROR 2
#define WRITE 0  // pipe fd[0] corresponds to write to buffer stdin
#define READ 1   // pipe fd[1] corresponds to read from buffer stdout

typedef struct {
    int index;
    char* type;
    int fd[2];
} metachar;

void type_prompt(void);
void error(char* message);

#endif /*MYSHELL_H*/