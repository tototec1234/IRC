/* write, close */
#include <unistd.h>

/* exit, malloc, calloc, free, atoi */
#include <stdlib.h>

#include <stdio.h>      /* sprintf */
#include <string.h>      /* strlen, strcpy, strcat, bzero */
#include <sys/socket.h>
#include <netinet/in.h>


int	g_sockfd = -1;	/* fatal() から閉じるため */
fd_set g_active;
int g_maxfd;
int g_ids[65536];
int g_next_id;
char* g_rbuf[65536];
char* g_wbuf[65536];
void	send_all(int except, char *msg);
void	accept_client(void);
void	handle_client(int fd);
void	flush_client(int fd);


char *str_join(char *buf, char *add);
int extract_message(char **buf, char **msg);

void	put_err(char *s)
{
	write(2, s, strlen(s));
}

void	fatal(void)
{
	if (g_sockfd != -1)
		close(g_sockfd);
	put_err("Fatal error\n");
	exit(1);
}

int	main(int argc, char **argv)
{

	// 原文が指定しているのは引数がない場合のみなので引数が 2 つ以上あってもエラーにしない
	if (argc < 2)
	{
		put_err("Wrong number of arguments\n");
		exit(1);
	}
	int portnbr = atoi(argv[1]);
	/* 段階 2: socket / bind / listen。失敗したら fatal() */
	// struct sockaddr_in servaddr, cli;
	struct sockaddr_in servaddr; //, cli;
	g_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (g_sockfd == -1)
		fatal();
	printf("Socket successfully created..\n"); // デバッグ出力、課題提出時にはコメントアウト、以下、Printfは全てその扱い
	bzero(&servaddr, sizeof(servaddr));
	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433);// bcコマンドで 1+2^(31-7)*127で値を出す
	servaddr.sin_port = htons(portnbr);

	// Binding newly created socket to given IP and verification 
	if ((bind(g_sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); // デバッグ出力、課題提出時にはコメントアウト
		fatal();
	} else
	printf("Socket successfully binded..\n"); // デバッグ出力、課題提出時にはコメントアウト
	if (listen(g_sockfd, 10) != 0) // maxpendingの値が10なのは、試験問題に添付されるはずのmain.cの82行目が根拠
	{
		printf("cannot listen\n"); // デバッグ出力、課題提出時にはコメントアウト
		fatal();
	}

	FD_ZERO(&g_active);
	FD_SET(g_sockfd, &g_active);
	g_maxfd = g_sockfd;
	while (1)
	{
		// select() replaces the given descriptor sets with subsets consisting of those descriptors that are ready for		the requested operation. 
		fd_set rfds = g_active;

		fd_set wfds;
		FD_ZERO(&wfds);
		for (int fd = 0; fd <= g_maxfd; fd++)
			if (FD_ISSET(fd, &g_active) && g_wbuf[fd] != NULL) // 送信バッファ g_wbuf[fd] に送信待ちデータが存在するかどうか
				FD_SET(fd, &wfds);

		if (select(g_maxfd + 1, &rfds, &wfds, NULL, NULL) < 0 )
			fatal();
		for (int fd = 0; fd <= g_maxfd; fd++)
		{
			if (FD_ISSET(fd, &rfds))
			{
				if (fd == g_sockfd)
					accept_client();
				else
					handle_client(fd);
			}
			if (FD_ISSET(fd, &wfds) && FD_ISSET(fd, &g_active))
				flush_client(fd);
		}

	}




	return (0);
}





void	send_all(int except, char *msg)
{
	for (int fd = 0; fd <= g_maxfd; fd++)
	{

		if (FD_ISSET(fd, &g_active) && fd != except && fd != g_sockfd)
		{
			g_wbuf[fd] = str_join(g_wbuf[fd], msg);
			if (g_wbuf[fd] == NULL)
				fatal();
		}
			// send(fd, g_wbuf[fd], strlen(msg), 0);   /* 段階 7 で送信待ちに足す形に変える */
	}
		
}

void	accept_client(void)
{
	char	msg[64];
	int		connfd;

	connfd = accept(g_sockfd, NULL, NULL);
	if (connfd < 0)
		return ;
	g_ids[connfd] = g_next_id++;
	FD_SET(connfd, &g_active);
	if ( g_maxfd < connfd )
		g_maxfd = connfd;
	sprintf(msg, "server: client %d just arrived\n", g_ids[connfd]);
	send_all(connfd, msg);
}



void	handle_client(int fd)
{
	char	buf[4097];
	char	msg[64];
	int		n;
	//  These calls return the number of bytes received, or -1 if an error occurred.
	// For TCP sockets, the return value 0 means the peer has closed its half side of the connection.

	n = recv(fd, buf, sizeof(buf) - 1 , 0);      /* 終端の '\0' を置く 1 バイトを残す */
	if (n <= 0)
	{
		sprintf(msg, "server: client %d just left\n", g_ids[fd]);
		// 段階 6 と段階 7 で、g_rbuf[fd] と g_wbuf[fd] をここで free して NULL にする
		free(g_rbuf[fd]);
		g_rbuf[fd] = NULL;
		free(g_wbuf[fd]);
		g_wbuf[fd] = NULL;
		FD_CLR(fd, &g_active);
		close(fd);
		send_all(fd, msg);
		return ;
	}
	buf[n] = '\0';

	char *line;
	char *out;
	int r;

	g_rbuf[fd] = str_join(g_rbuf[fd], buf);
	if (g_rbuf[fd] == 0)
		fatal();
	while ((r = extract_message(&g_rbuf[fd], &line)) == 1)
	{
		out = malloc(strlen(line) + sizeof("client %d: ") + 32);
		if (out == NULL)
			fatal();
		sprintf(out, "client %d: %s", g_ids[fd], line);
		send_all(fd, out);
		free(out);
		free(line);
	}
	if (r == -1)
		fatal();
}



char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}


/*
cd ~/IRC/bircd/exam06_miniserv
gcc -Wall -Wextra -Werror mini_serv.c -o mini_serv
./mini_serv; echo "exit=$?"
./mini_serv 2>&1 >/dev/null | od -c
./mini_serv 2>/dev/null | od -c


lsof -nP -iTCP:8081

*/
int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}


void	flush_client(int fd)
{
	char	*rest;
	int		len;
	int		n;

	len = strlen(g_wbuf[fd]);
	n = send(fd, g_wbuf[fd], len, 0);
	if (n <= 0)
		return ;                 /* 切断は recv 側で検出する */
	if (n == len)
	{
		free(g_wbuf[fd]);
		g_wbuf[fd] = NULL;
		return ;
	}
	rest = malloc(len - n + 1);           /* 送れなかった len - n バイトと '\0' */
	if (rest == NULL)
		fatal();
	strcpy(rest, g_wbuf[fd] + n);
	free(g_wbuf[fd]);
	g_wbuf[fd] = rest;
}