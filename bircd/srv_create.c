
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include "bircd.h"

void			srv_create(t_env *e, int port)
{
  int			s;
  struct sockaddr_in	sin;	// p41
  struct protoent	*pe;
  
  /* 	soket	*/
  pe = (struct protoent*)Xv(NULL, getprotobyname("tcp"), "getprotobyname");
  s = X(-1, socket(PF_INET, SOCK_STREAM, pe->p_proto), "socket");
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port);

  /* 	bind	*/
  X(-1, bind(s, (struct sockaddr*)&sin, sizeof(sin)), "bind");

  /* 	listen	*/
  X(-1, listen(s, 42), "listen");
  e->fds[s].type = FD_SERV;
  e->fds[s].fct_read = srv_accept;
}
