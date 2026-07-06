#include <stdio.h>	/* printf()、fprintf()に必要 */
#include <sys/socket.h>	/* socket()、connect()、send()、recv()に必要 */
#include <arpa/inet.h>	/* sockaddr_in、inet_addr()に必要 */
#include <stdlib.h>	/* atoi()に必要 */
#include <string.h>	/* memset()に必要 */
#include <unistd.h>	/* close()に必要 */

#define RCVBUFSIZE 32	/* 受信バッファのサイズ */

void DieWithError(char *errorMessage);	/* エラー処理関数 */

int main(int argc, char *argv[])
{
	int sock;			/* ソケットディスクリプタ */
	struct sockaddr_in echoServAddr;/* エコーサーバのアドレス */
	unsigned short echoServPort;	/* エコーサーバのポート番号 */
	char *servIP;			/* サーバのIPアドレス（dotted-quad） */
	char *echoString;	        /* エコーサーバに送信する文字列 */
	char echoBuffer[RCVBUFSIZE];	/* エコー文字列用のバッファ */
	unsigned int echoStringLen;	/* エコーする文字列のサイズ */
	int bytesRcvd, totalBytesRcvd;	/* 一回のrecv()で読み取られる
					   バイト数と全バイト数 */

	if ((argc < 3) || (argc > 4))	/* 引数の数が正しいか確認 */
	{
		fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>}\n",
				argv[0]);
		exit(1);
	}

	servIP = argv[1];		/* 1つ目の引数：サーバのIPアドレス（ドット10進表記） */
	echoString = argv[2];	/* 2つ目の引数：エコー文字列 */

	if (argc == 4)
		echoServPort = atoi(argv[3]);	/* 指定のポート番号があれば使用 */
	else
		echoServPort = 7;	/* 7はエコーサービスのwell-knownポート番号 */

	/* TCPによる信頼性の高いストリームソケットを作成 */
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		DieWithError("socket() failed");

	/* サーバのアドレス構造体を作成 */
	memset(&echoServAddr, 0, sizeof(echoServAddr));		/* 構造体にゼロを埋める */
	echoServAddr.sin_family = AF_INET;			/* インターネットアドレスファミリ */
	echoServAddr.sin_addr.s_addr = inet_addr(servIP);	/* サーバのIPアドレス */
	echoServAddr.sin_port = htons(echoServPort);		/* サーバのポート番号 */

	/* エコーサーバへの接続を確立 */
	if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
		DieWithError("connect() failed");

	echoStringLen = strlen(echoString);	/* 入力データの長さを調べる */

	/* 文字列をサーバに送信 */
	if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
		DieWithError("send() sent a different number of bytes than expected");

	/* 同じ文字列をサーバから受信 */
	totalBytesRcvd = 0;
	printf("Received: ");	/* エコーされた文字列を表示するための準備 */
	while (totalBytesRcvd < echoStringLen)
	{
		/* バッファサイズに達するまで（ヌル文字用の1バイトを除く）
			サーバからのデータを受信する */
		if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
			DieWithError("recv() failed or connection closed prematurely");
		totalBytesRcvd += bytesRcvd;	/* 総バイト数の集計 */
		echoBuffer[bytesRcvd] = '\0' ;	/* 文字列の終了 */
		printf("%s", echoBuffer);	/* エコーバッファの表示 */
	}

	printf("\n");	/* 最後の改行を出力 */

	close(sock);
	exit(0);
}

