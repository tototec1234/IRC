#include <stdio.h>      /* fprintf()に必要 */
#include <sys/socket.h> /* socket()、connect()、send()、recv()に必要 */
#include <arpa/inet.h>  /* sockaddr_in、inet_addr()に必要 */
#include <stdlib.h>     /* atoi()に必要 */
#include <string.h>     /* memset()に必要 */
#include <unistd.h>     /* sleep()に必要 */

void DieWithError(char *errorMessage);  /* 外部エラー処理関数 */


int main(int argc, char *argv[])
 {
     int sock;                        /* ソケット */
     struct sockaddr_in multicastAddr; /* マルチキャストアドレス */
     char *multicastIP;               /* IPマルチキャストアドレス */
     unsigned short multicastPort;     /* サーバのポート */
     char *sendString;                /* マルチキャストする文字列 */
     unsigned char multicastTTL;      /* マルチキャストパケットのTTL */
     unsigned int sendStringLen;      /* マルチキャストする文字列の長さ */

     if ((argc < 4) || (argc > 5))         /* パラメータの数が正しいか確認 */
     {
         fprintf(stderr, "Usage: %s <Multicast Address> <Port> <Send String> [<TTL>]\n",
                  argv[0]);
         exit(1);
     }

     multicastIP = argv[1];              /* 1つ目の引数：マルチキャストするIPアドレス */
     multicastPort = atoi(argv[2]);      /* 2つ目の引数：マルチキャスト用ポート */
     sendString = argv[3];             /* 2つ目の引数：マルチキャストする文字列 */

     if (argc == 5)                    /* TTLがコマンドラインから指定されているか */
         multicastTTL = atoi(argv[4]); /* コマンドラインから指定されたTTL */
     else
     multicastTTL = 1;                 /* デフォルトのTTL = 1 */

     /* データグラム送受信用ソケットの作成 */
     if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
     DieWithError("socket() failed");

     /* マルチキャストパケットのTTLを設定 */
     if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &multicastTTL,
           sizeof(multicastTTL)) < 0)
         DieWithError("setsockopt() failed");

     /* ローカルのアドレス構造体を作成 */
     memset(&multicastAddr, 0, sizeof(multicastAddr)); /* 構造体をゼロで埋める */
     multicastAddr.sin_family = AF_INET;               /* インターネットアドレスファミリ */
     multicastAddr.sin_addr.s_addr = inet_addr(multicastIP);/* マルチキャストIPアドレス */
     multicastAddr.sin_port = htons(multicastPort);       /* マルチキャストするポート */

     sendStringLen = strlen(sendString); /* sendStringの長さを調べる */
     for (;;) /* プログラムが終了するまで繰り返し実行 */
     {
         /* sendStringのデータグラムを3秒ごとにクライアントへマルチキャストする */
         if (sendto(sock, sendString, sendStringLen, 0, (struct sockaddr *)
               &multicastAddr, sizeof(multicastAddr)) != sendStringLen)
             DieWithError("sendto() sent a different number of bytes than expected");
         sleep(3);
     }
     /* この部分には到達しない */
 }

