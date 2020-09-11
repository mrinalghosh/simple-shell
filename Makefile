CC=gcc

myshell: myshell.c
	$(CC) -Werror -o myshell myshell.c -I