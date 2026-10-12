/* write, close */
#include <unistd.h>

/* exit, malloc, calloc, free, atoi */
#include <stdlib.h>

#include <stdio.h>      /* sprintf */
#include <string.h>      /* strlen, strcpy, strcat, bzero */
#include <sys/socket.h>
#include <netinet/in.h>


int	g_sockfd = -1;	/* fatal() から閉じるため */

void	put_err(char *s)
{
	write(2, s, strlen(s));
}

void	fatal(void)
{
	if (g_sockfd)
		printf("/* 段階 3 以降: 開いている fd と確保したメモリをここで片づける */"); // ダミーです
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

	while (1);




	return (0);
}

/*
cd ~/IRC/bircd/exam06_miniserv
gcc -Wall -Wextra -Werror mini_serv.c -o mini_serv
./mini_serv; echo "exit=$?"
./mini_serv 2>&1 >/dev/null | od -c
./mini_serv 2>/dev/null | od -c

*/