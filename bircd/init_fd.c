#include <stdio.h>
// #include <cstdio>
#include <string.h>
// # include <sys/select.h>
#include "bircd.h"

/*
void	init_fd(t_env *e)
{
  int	i;

  i = 0;
  e->max = 0;
  FD_ZERO(&e->fd_read);
  FD_ZERO(&e->fd_write);
  while (i < e->maxfd)
	{
	  if (e->fds[i].type != FD_FREE)
	{
	  FD_SET(i, &e->fd_read);			// 全員読み待ち　 listen fd も client fd も 読めるようになったら反応したい:
	  if (strlen(e->fds[i].buf_write) > 0)	// 送信バッファ buf_write が空なら 送るものがない → write 監視するのはムダ！
		{
		  FD_SET(i, &e->fd_write);
		}
	  e->max = MAX(e->max, i);
	}
	  i++;
	}
}
*/

/*
init_fd(e)     ← fd_set構築  →  pollfds[] 構築 + nfds セット に変更

 bircd は fds[i] の i = fd番号。poll 配列は 使用中 fd だけ詰める。pollfds[j].fd に fd 番号を入れる。
*/

void init_fd(t_env *e)
{
	int i;

	e->nfds = 0;                    // 配列を毎回作り直す
	i = 0;
	while (i < e->maxfd)
	{
		if (e->fds[i].type != FD_FREE)
		{
			if (e->nfds >= MAX_CLIENTS + 1)
			{
				fprintf(stderr, "init_fd: pollfds[] full (nfds=%d, limit=%d); fd %d not monitored\n",
					e->nfds, MAX_CLIENTS + 1, i);
				continue;
			}	

			e->pollfds[e->nfds].fd = i;           // ← fd番号
			e->pollfds[e->nfds].events = POLLIN;   // 全員読み待ち　 FD_SET(i, &e->fd_write);　相当　listen fd も client fd も 読めるようになったら反応したい:
			e->pollfds[e->nfds].revents = 0;

			if (strlen(e->fds[i].buf_write) > 0)		// 送信バッファ buf_write が空なら 送るものがない → write 監視するのはムダ！
				e->pollfds[e->nfds].events |= POLLOUT;  // 送信待ちも  FD_SET(i, &e->fd_write); 相当

			e->nfds++;   // 使用中 fd を1件追加
		}
		i++;
	}
}