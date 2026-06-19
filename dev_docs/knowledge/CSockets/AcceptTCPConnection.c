#include <stdio.h>      /* printf()に必要 */
#include <sys/socket.h> /* accept()に必要 */
#include <arpa/inet.h>  /* sockaddr_inとinet_ntoa()に必要 */

void DieWithError(char *errorMessage); /* エラー処理関数 */

int AcceptTCPConnection(int servSock)
{
    int clntSock;                   /* クライアントのソケットディスクリプタ */
    struct sockaddr_in echoClntAddr; /* クライアントのアドレス */
    unsigned int clntLen;           /* クライアントのアドレス構造体の長さ */

    /* 入出力パラメータのサイズをセット */
    clntLen = sizeof(echoClntAddr);

    /* クライアントからの接続要求を待つ */
    if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr,
           &clntLen)) < 0)
        DieWithError("accept() failed");

     /* clntSockはクライアントに接続済み */

     printf ("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

     return clntSock;
}
