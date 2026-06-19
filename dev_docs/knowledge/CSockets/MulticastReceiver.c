#include <stdio.h>      /* printf()とfprintf()に必要 */
#include <sys/socket.h> /* socket()、connect()、sendto()、recvfrom()に必要 */
#include <arpa/inet.h>  /* sockaddr_in、inet_addr()に必要 */
#include <stdlib.h>     /* atoi()に必要 */
#include <string.h>     /* memset()に必要 */
#include <unistd.h>     /* close()に必要 */

#define MAXRECVSTRING 255  /* 受信する文字列の最大長 */

void DieWithError(char *errorMessage); /* 外部エラー処理関数 */


 int main(int argc, char *argv[])
 {
     int sock;                         /* ソケット */
     struct sockaddr_in multicastAddr; /* マルチキャストアドレス */
     char *multicastIP;                /* マルチキャストIPアドレス */
     unsigned int multicastPort;       /* ポート */
     char recvString[MAXRECVSTRING+1]; /* 文字列受信用バッファ */
     int recvStringLen;                /* 受信する文字列の長さ */
     struct ip_mreq multicastRequest;  /* ジョインするマルチキャストアドレスの構造体 */

     if (argc != 3)       /* 引数の数が正しいか確認 */
     {
         fprintf(stderr, "Usage: %s <Multicast IP> <Multicast Port>\n", argv[0]);
         exit(1);
     }

     multicastIP = argv[1];        /* 1つ目の引数：マルチキャストIPアドレス（ドット10進表記） */
     multicastPort = atoi(argv[2]);/* 2つ目の引数：マルチキャストポート */

     /* UDPによるベストエフォート式のデータグラムを作成 */
     if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
         DieWithError("socket() failed");

     /* バインドする構造体の作成 */
     memset(&multicastAddr, 0, sizeof(multicastAddr));   /* 構造体をゼロで埋める */

     multicastAddr.sin_family = AF_INET;                 /* インターネットアドレスファミリ */
     multicastAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* ワイルドカードを使用 */
     multicastAddr.sin_port = htons(multicastPort);      /* マルチキャストポート */

     /* マルチキャストポートへバインド */
     if (bind(sock, (struct sockaddr *) &multicastAddr, sizeof(multicastAddr))< 0)
         DieWithError("bind() failed");

     /* マルチキャストグループの指定 */
     multicastRequest.imr_multiaddr.s_addr = inet_addr(multicastIP);
     /* 任意のインタフェースからマルチキャストを受け付ける */
     multicastRequest.imr_interface. s_addr = htonl(INADDR_ANY);
     /* マルチキャストグループへのジョイン */
     if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &multicastRequest,
           sizeof(multicastRequest)) < 0)
         DieWithError("setsockopt() failed");

     /* サーバからデータグラムを1つ受信 */
     if ((recvStringLen = recvfrom(sock, recvString, MAXRECVSTRING, 0, NULL, 0)) < 0)
         DieWithError("recvfrom() failed");

     recvString[recvStringLen] =
     printf("Received: %s\n", recvString);     /* 受信した文字列の表示 */

     close(sock);
     exit(0);
 }


