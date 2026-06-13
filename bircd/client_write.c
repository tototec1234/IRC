
#include <sys/socket.h>
#include <unistd.h>
#include "bircd.h"
#include "libft/libft.h"

void	client_write(t_env *e, int cs)
{
	int	sent;

	if (e->fds[cs].buf_write_len == 0)
		return;

	sent = send(cs, e->fds[cs].buf_write, e->fds[cs].buf_write_len, 0);

	if (sent <= 0)
	{
		close(cs);
		clean_fd(&e->fds[cs]);
		return;
	}

	// 送信ぶみ部分削除
	ft_memmove(e->fds[cs].buf_write,
		e->fds[cs].buf_write + sent,
		e->fds[cs].buf_write_len - sent + 1);
	e->fds[cs].buf_write_len -= sent;
}
