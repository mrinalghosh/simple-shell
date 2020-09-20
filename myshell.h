#ifndef MYSHELL_H
#define MYSHELL_H

#define MAX_TOKEN 1 << 5
#define MAX_BUFFER 1 << 9
#define TOKEN_LIMIT 1 << 7  // maximum number of tokens - arbitrary choice of 128
#define STD_INPUT 0
#define STD_OUTPUT 1
#define STD_ERROR 2
#define AMPERSAND "&"
#define GREATER ">"
#define LESSER "<"
#define PIPE "|"

// TODO: add function prototypes
void type_prompt(void);
void error(char* message);

#endif /*MYSHELL_H*/