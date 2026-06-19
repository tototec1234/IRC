#include <stdio.h>      /* printf()、fprintf()に必要 */
#include <sys/socket.h> /* socket()、bind()、connect()に必要 */
#include <arpa/inet.h>  /* sockaddr_in、inet_ntoa()に必要 */
#include <stdlib.h>     /* atoi()に必要 */
#include <string.h>      /* memset()に必要 */
#include <unistd.h>     /* close()に必要 */

void DieWithError(char *errorMessage);  /* エラー処理関数 */
void HandleTCPClient(int clntSocket);   /* TCPクライアントの処理関数 */
int CreateTCPServerSocket(unsigned short port); /* TCPサーバソケットを作成 */
int AcceptTCPConnection(int servSock);  /* TCP接続要求を受け付ける */
