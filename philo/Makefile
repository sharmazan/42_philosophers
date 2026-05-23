NAME		= philo

CC			= cc
CFLAGS		= -Wall -Wextra -Werror
RM			= rm -f

SRCS		= philo.c utils.c parsing.c state.c logging.c sim.c worker.c \
			  threads.c monitor.c
OBJS		= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c philo.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
