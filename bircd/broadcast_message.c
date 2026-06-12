#include "bircd.h"
#include "libft/libft.h"

/*
** スタブ: Lesson 3 検証用。
** 切り出した1メッセージをサーバの stdout に表示するだけ。
** 本実装（他クライアントへ配送）は Lesson 4 で buf_write + POLLOUT に差し替え。
*/
void	broadcast_message(t_env *e, int cs, char *msg)
{
	(void)e;
	ft_putstr_fd("[fd=", 1);
	ft_putnbr_fd(cs, 1);
	ft_putstr_fd("] msg=[", 1);
	ft_putstr_fd(msg, 1);
	ft_putstr_fd("]\n", 1);
}