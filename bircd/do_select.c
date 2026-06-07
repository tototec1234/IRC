
#include <stdlib.h>
#include "bircd.h"

/*
void	do_select(t_env *e)
{
  e->r = select(e->max + 1, &e->fd_read, &e->fd_write, NULL, NULL);
}
*/

void do_select(t_env *e)   // 名前はそのままでも可。後で do_poll にリネームしてもよい
{
	poll(e->pollfds, e->nfds, -1);	// -1（無限待ち。select 版と同じ）
//	e->r = poll(e->pollfds, e->nfds, -1);	// select版: e->r = select(...)
}


/*
POLL(2)                                                  System Calls Manual                                                 POLL(2)

NAME
	 poll – synchronous I/O multiplexing

SYNOPSIS
	 #include <poll.h>

	 int
	 poll(struct pollfd fds[], nfds_t nfds, int timeout);

*/