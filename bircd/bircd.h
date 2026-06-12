#ifndef BIRCD_H_
# define BIRCD_H_

# include <sys/select.h>

#include <poll.h> //add

# define FD_FREE	0
# define FD_SERV	1
# define FD_CLIENT	2

#define MAX_CLIENTS 42  //とりあえず最大同時接続42名　	ft_irc（提出）ではvector<pollfd> 動的　で上限は　実質 OS の fd 上限の予定？

// # define BUF_SIZE	4096
# define BUF_SIZE	100 //Lesson3で分割送信を体感するため

# define Xv(err,res,str)	(x_void(err,res,str,__FILE__,__LINE__))
# define X(err,res,str)		(x_int(err,res,str,__FILE__,__LINE__))
# define MAX(a,b)	((a > b) ? a : b)

# define USAGE		"Usage: %s port\n"

struct	s_env;	//追加

typedef struct	s_fd
{
  int	type;
//   void	(*fct_read)();
//   void	(*fct_write)();
	void	(*fct_read)(struct s_env *, int);	//差し替え
	void	(*fct_write)(struct s_env *, int);	//差し替え

	int buf_read_len; // C++のstd::stringは長さを別持ち（メンバ変数に格納？）してるのでこれ不要
  char	buf_read[BUF_SIZE + 1];
  char	buf_write[BUF_SIZE + 1];
}		t_fd;

typedef struct	s_env
{
  t_fd		*fds;
  struct pollfd pollfds[MAX_CLIENTS + 1];	//add 	ft_irc（提出）ではvector<pollfd> 動的　
  int		nfds;					//add
  int		port;
  int		maxfd;
  int		max;
//  int		r;	// select版 check_fd で使用。poll版未使用
//   fd_set	fd_read;
//   fd_set	fd_write;
}		t_env;

void	init_env(t_env *e);
void	get_opt(t_env *e, int ac, char **av);
void	main_loop(t_env *e);
void	srv_create(t_env *e, int port);
void	srv_accept(t_env *e, int s);
void	client_read(t_env *e, int cs);
void	client_write(t_env *e, int cs);
void	broadcast_message(t_env *e, int cs, char *msg); //　クライアント転送なし、stdoutするだけのスタブ
void	clean_fd(t_fd *fd);
int	x_int(int err, int res, char *str, char *file, int line);
void	*x_void(void *err, void *res, char *str, char *file, int line);
void	init_fd(t_env *e);
void	do_select(t_env *e);
void	check_fd(t_env *e);

#endif /* !BIRCD_H_ */
