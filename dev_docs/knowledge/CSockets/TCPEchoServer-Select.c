#include "TCPEchoServer.h"  /* TCPエコーサーバのインクルードファイル */
#include <sys/time.h>        /* timeval{}に必要 */
#include <fcntl.h>          /* fcntl()に必要 */

int main(int argc, char *argv[])
{
     int *servSock;                   /* サーバのソケットディスクリプタ */
     int maxDescriptor;               /* ソケットディスクリプタの最大値 */
     fd_set sockSet;                  /* select()の対象となるソケットディスクリプタのセット */
     long timeout;                    /* コマンドラインから指定されたタイムアウトの値 */
     struct timeval selTimeout;       /* select()のタイムアウト */
     int running = 1;                 /* サーバが動作中であれば1、そうでなければ0 */
     int noPorts;                     /* コマンドラインから指定したポートの数 */
     int port;                        /* ポートを処理するループの変数 */
     unsigned short portNo;           /* 実際のポート番号 */

     if (argc < 3)      /* 引数の数が正しいか調べる */
     {
         fprintf (stderr, "Usage:  %s <Timeout (secs.)> <Port 1> ... \n", argv[0]);
         exit(1);
     }

     timeout = atol(argv[1]);        /* 1つ目の引数：タイムアウト（秒） */
     noPorts = argc - 2;             /* ポートの数は、引数のカウント【監訳注 argcの値です】から2を引く */

     /* 着信接続要求を入れるソケットリストの割り当て */
     servSock = (int *) malloc(noPorts * sizeof(int));

     /* select()で使用するmaxDescriptorの初期化 */
     maxDescriptor = -1;

     /* ポート、およびポートを処理するソケットのリストを作成 */
     for (port = 0; port < noPorts; port++)
     {
         /* ポートをリストに追加 */
         portNo = atoi(argv[port + 2]); /*最初の2つの引数をスキップ */

         /* ポートのソケットを作成 */
         servSock[port] = CreateTCPServerSocket(portNo);

         /* 新しいディスクリプタの値が最大か調べる */
         if (servSock[port] > maxDescriptor)
            maxDescriptor = servSock[port];
     }

     printf("Starting server: Hit return to shutdown\n");
     while (running)
     {
         /* ソケットディスクリプタのベクタをゼロ初期化してサーバのソケットをセットする */
         /* select()を実行するたびにリセットが必要 */
         FD_ZERO(&sockSet);
         /* ディスクリプタのベクタにキーボードを追加 */
         FD_SET(STDIN_FILENO, &sockSet);
         for (port = 0; port < noPorts; port++)
             FD_SET(servSock[port], &sockSet);

         /* タイムアウトの指定 */
         /* select()を実行するたびにリセットが必要 */
         selTimeout.tv_sec = timeout;       /* タイムアウト（秒） */
         selTimeout.tv_usec = 0;            /* 0マイクロ秒*/

         /* ディスクリプタが利用可能、またはタイムアウトになるまでプログラムを一時中断 */
         if (select(maxDescriptor + 1, &sockSet, NULL, NULL, &selTimeout) == 0)
             printf("No echo requests for %ld secs... Server still alive\n", timeout);
         else
         {
             if (FD_ISSET(STDIN_FILENO, &sockSet)) /* キーボードのチェック */
             {
                 printf("Shutting down server\n");
                 getchar();
                 running = 0;
             }

         for (port = 0; port < noPorts; port++)
             if (FD_ISSET(servSock[port], &sockSet))
             {
                 printf("Request on port %d: ", port);
                 HandleTCPClient(AcceptTCPConnection(servSock[port]));
             }
       }
     }

     /* ソケットのクローズ */
     for (port = 0; port < noPorts; port++)
         close(servSock[port]);

     /* ソケットのリストをメモリから解放 */
     free(servSock);

     exit(0);
 }

