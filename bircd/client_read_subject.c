
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include "bircd.h"

void	client_read(t_env *e, int cs)
{
	t_fd	*f;
  int	r;
//   int	i;
	char	*pos;
	int		msg_len;

	f = &e->fds[cs];

//   r = recv(cs, e->fds[cs].buf_read, BUF_SIZE, 0);
  /* (A) 累積バッファの「末尾」に直接 recv する。
         第2引数: バッファのどの位置から書く？
         第3引数: 残り空きは何バイト？ */
	//  r = recv(cs, /* (A1) */, /* (A2) */, 0);
	r = recv(cs, e->fds[cs].buf_read, BUF_SIZE, 0);

// DEBUG(Lesson3.1):bircd_learning_curriculum_ans.md
//  TCPはバイトストリーム。recvの切れ目はデータに残らない
/*
python3 -c "import sys; sys.stdout.buffer.write(b'A' * 100000)" | nc localhost 6667
*/
// → 分割はサーバ側ログでしか観察できない（受信側LOGは切れ目なし100000B）
// 実験後はこの行と bircd.h の BUF_SIZE=100 を元に戻す
fprintf(stderr, "recv %d bytes\n", r);
  if (r <= 0)
	{
	  close(cs);
	  clean_fd(&e->fds[cs]);
	  printf("client #%d gone away\n", cs);
	}
  else
	{
	  i = 0;
	  while (i < e->maxfd)
	{
	  if ((e->fds[i].type == FD_CLIENT) && (i != cs))
	  {
		send(i, e->fds[cs].buf_read, r, 0);
	  }
	  i++;
	}
	}
}
