
#include "bircd.h"

int	main(int ac, char **av)
{
  t_env	e;

  init_env(&e);					//   ← fd_set構築  →  pollfds[] 構築 + nfds セット　に変更
  get_opt(&e, ac, av);
  srv_create(&e, e.port);
  main_loop(&e);
  return (0);
}

/*

./bircd 6667

nc localhost 6667

nc localhost 6667

*/


/*
MacOSのターミナルでテストする時は下記に注意
macOS BSD nc は LF のみ送る → CRLF 必須実装では生 nc が無反応

printf 'PING :hello\r\n' | nc localhost 6667

printf '...\r\n' や Ctrl-V Ctrl-M（^M）で CR を明示すれば動く

*/