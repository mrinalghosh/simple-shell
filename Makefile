CC=gcc

myshell: myshell.c
	$(CC) --std=c99 -Werror -o myshell myshell.c