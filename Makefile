CC=gcc
NAME=test
$(NAME): $(NAME).c
	$(CC) $(NAME).c my_string.c scanner.c -o $(NAME)