#include <stdio.h>       /* printf()とfprintf()に必要 */
#include <sys/socket.h>  /* socket()、connect()、sendto()、recvfrom()に必要 */
#include <arpa/inet.h>   /* sockaddrinとinet_addr()に必要 */
#include <stdlib.h>      /* atoi()に必要 */
#include <string.h>      /* memset()に必要 */
#include <unistd.h>      /* close()に必要 */

#define MAXRECVSTRING 255 /* 受信する文字列の最大長 */

void DieWithError(char *errorMessage); /* 外部エラー処理関数 */

 int main(int argc, char *argv[])
 {
     int sock;                         /* ソケット */
     struct sockaddr_in broadcastAddr; /* ブロードキャストアドレス */
     unsigned int broadcastPort;     /* ポート */
     char recvString[MAXRECVSTRING+1]; /* 文字列の受信用バッファ */
     int recvStringLen;                /* 受信する文字列の長さ */

     if (argc != 2)    /* 引数の数が正しいか確認 */
     {
         fprintf(stderr,"Usage: %s <Broadcast Port>\n", argv[0]);
         exit(1);
     }

     broadcastPort = atoi(argv[1]);  /* 1つ目の引数：ブロードキャストポート */

     /* UDPによるベストエフォート式のデータグラムソケットを作成 */
     if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

     /* バインドする構造体の作成 */
     memset(&broadcastAddr, 0, sizeof(broadcastAddr));  /* 構造体をゼロで埋める */
     broadcastAddr.sin_family = AF_INET;                /* インターネットアドレスファミリ */
     broadcastAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* ワイルドカードを使用 */
     broadcastAddr.sin_port = htons(broadcastPort);     /* ブロードキャスト用ポート */

     /* ブロードキャストするポートへのバインド */
     if (bind(sock, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr)) < 0)
         DieWithError("bind() failed");

     /* データグラムをサーバから1つ受信 */
     if ((recvStringLen = recvfrom(sock, recvString, MAXRECVSTRING, 0, NULL, 0)) < 0)
         DieWithError("recvfrom() failed");

     recvString[recvStringLen] = '\0';
     printf("Received: %s\n", recvString);   /* 受信した文字列を表示 */

     close(sock);
     exit(0);
 }
