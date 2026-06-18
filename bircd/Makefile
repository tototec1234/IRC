SRCS =	main.c init_env.c clean_fd.c get_opt.c x.c main_loop.c \
	init_fd.c do_select.c check_fd.c \
	srv_create.c srv_accept.c \
	client_read.c client_write.c broadcast_message.c

OBJS = ${SRCS:.c=.o}

NAME = bircd

# MINITALK/Makefile 11-13行目と同じ方式
LIBFT_DIR = libft
LIBFT_A = ${LIBFT_DIR}/libft.a

CFLAGS = -I. -I${LIBFT_DIR} -g3 -Wall -Werror
LDFLAGS = -L${LIBFT_DIR} -lft

CC = gcc
RM = rm -f

${NAME}:	${LIBFT_A} ${OBJS}
		${CC} -o ${NAME} ${OBJS} ${LDFLAGS}

${LIBFT_A}:
		@make -C ${LIBFT_DIR}

all:		${NAME}

clean:
		make -C ${LIBFT_DIR} clean
		${RM} ${OBJS} *~ #*#

fclean:		clean
		make -C ${LIBFT_DIR} fclean
		${RM} ${NAME}

re:		fclean all

.PHONY:		all clean fclean re