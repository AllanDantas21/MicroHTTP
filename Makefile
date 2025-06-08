NAME = webserv

CC = cc
CFLAGS = -Wall -Wextra -Werror -I includes/

SRCS_DIR = srcs/
SRCS = $(shell find $(SRCS_DIR) -name '*.c')
OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
