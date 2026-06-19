#include <stdio.h>      /* printf()とfprintf()に必要 */
#include <sys/socket.h> /* socket()とbind()に必要 */
#include <arpa/inet.h>  /* sockaddr_inに必要 */
#include <stdlib.h>     /* atoi()に必要 */
#include <string.h>     /* memset()に必要 */
#include <unistd.h>     /* close()に必要 */

void DieWithError(char *errorMessage); /* 外部エラー処理関数 */

int main(int argc, char *argv[])
 {
     int sock;                         /* ソケット */
     struct sockaddr_in broadcastAddr; /* ブロードキャストアドレス */
     char *broadcastIP;                /* IPブロードキャストアドレス */
     unsigned short broadcastPort;     /* サーバのポート */
     char *sendString;                 /* ブロードキャストする文字列 */
     int broadcastPermission;          /* ブロードキャストを有効にするソケットオプション */
     unsigned int sendStringLen;       /* ブロードキャストする文字列の長さ */

     if (argc < 4)                     /* パラメータの数が正しいか確認 */
     {
         fprintf(stderr,"Usage: %s <IP Address> <Port> <Send String>\n", argv[0]);
         exit(1);
     }

     broadcastIP = argv[1];           /* 1つ目の引数：ブロードキャストするIPアドレス */
     broadcastPort = atoi(argv[2]);   /* 2つ目の引数：ブロードキャスト用ポート */
     sendString = argv[3] ;           /* 3つ目の引数：ブロードキャストする文字列 */

     /* データグラムを送受信するソケットを作成 */
     if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
         DieWithError("socket() failed");

     /* ソケットをブロードキャスト可能に設定 */
     broadcastPermission = 1;
     if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission,
           sizeof(broadcastPermission)) < 0)
         DieWithError("setsockopt() failed");

     /* ローカルのアドレス構造体を作成 */
     memset(&broadcastAddr, 0, sizeof(broadcastAddr));    /* 構造体をゼロで埋める */
     broadcastAddr.sin_family = AF_INET;                  /* インターネットアドレスファミリ */
     broadcastAddr.sin_addr.s_addr = inet_addr(broadcastIP);/* ブロードキャストIPアドレス */
     broadcastAddr.sin_port = htons(broadcastPort);          /* ブロードキャストポート */

     sendStringLen = strlen(sendString);  /* sendStringの長さを調べる */
     for (;;) /* プログラムが終了するまで繰り返し実行 */
     {
         /* sendStringのデータグラムをクライアントへ3秒ごとにブロードキャストする*/
         if (sendto(sock, sendString, sendStringLen, 0, (struct sockaddr *)
               &broadcastAddr, sizeof(broadcastAddr)) != sendStringLen)
             DieWithError("sendto() sent a different number of bytes than expected");

         sleep(3);  /* ネットワークがいっぱいになるのを防ぐ */
      }
      /* この部分には到達しない */
 }
