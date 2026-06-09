
#include "bircd.h"

/*
void	check_fd(t_env *e)
{
  int	i;

  i = 0;
  while ((i < e->maxfd) && (e->r > 0))
	{
	  if (FD_ISSET(i, &e->fd_read))
	e->fds[i].fct_read(e, i);
	  if (FD_ISSET(i, &e->fd_write))
	e->fds[i].fct_write(e, i);
	  if (FD_ISSET(i, &e->fd_read) ||
	  FD_ISSET(i, &e->fd_write))
	e->r--;
	  i++;
	}
}
*/

void check_fd(t_env *e)
{
	int i;
	int fd;

	i = 0;
	while (i < e->nfds)
	{
		fd = e->pollfds[i].fd;

		if (e->pollfds[i].revents & POLLIN)
			e->fds[fd].fct_read(e, fd);

		if (e->pollfds[i].revents & POLLOUT)
			e->fds[fd].fct_write(e, fd);

		// POLLERR / POLLHUP は Lesson 3 以降実装？

		i++;
	}
}

/*
pollfds[i].fd と fds 配列の添字の関係は？ → 同じ fd 番号
*/