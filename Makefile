CC=gcc

myshell: myshell.c
	$(CC) -Werror -g -o myshell myshell.c