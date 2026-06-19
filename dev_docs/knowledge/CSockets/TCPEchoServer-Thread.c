#include "TCPEchoServer.h" /* TCPエコーサーバのヘッダファイルをインクルード */
#include <pthread.h>      /* POSIXスレッドに必要 */

void *ThreadMain(void *arg);          /* スレッドのメインプログラム */

/* クライアントスレッドに渡す引数の構造体 */
struct ThreadArgs
{
int clntSock;                          /* クライアントのソケットディスクリプタ */
};

int main(int argc, char *argv[])
{
    int servSock;                    /* サーバのソケットディスクリプタ */
    int clntSock;                    /* クライアントのソケットディスクリプタ */
    unsigned short echoServPort;     /* サーバのポート */
    pthread_t threadID;              /* pthread_create()が返すスレッドID */
    struct ThreadArgs *threadArgs;   /* スレッドの引数構造体へのポインタ */

    if (argc != 2)    /* 引数の数が正しいか確認 */
    {
        fprintf(stderr,"Usage: %s <SERVER PORT>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]); /* 1つ目の引数： ローカルポート */

    servSock = CreateTCPServerSocket(echoServPort);

    for (;;) /* プログラムが終了するまで繰り返し実行 */
    {
        clntSock = AcceptTCPConnection(servSock);

        /* クライアント引数用にメモリを新しく確保 */
        if ((threadArgs = (struct ThreadArgs *) malloc(sizeof(struct ThreadArgs)))
               == NULL)
             DieWithError("malloc() failed");
        threadArgs -> clntSock = clntSock;

        /* クライアントスレッドを生成 */
        if (pthread_create(&threadID, NULL, ThreadMain, (void *) threadArgs) != 0)
            DieWithError("pthread_create() failed");
        printf("with thread %ld\n", (long int) threadID);
    }
    /* この部分には到達しない */
}

void *ThreadMain(void *threadArgs)
{
    int clntSock; /* クライアント接続用ソケットディスクリプタ */

    /* 戻り時に、スレッドのリソースを割り当て解除 */
    pthread_detach(pthread_self());

    /* ソケットのファイルディスクリプタを引数から取り出す */
    clntSock = ((struct ThreadArgs *) threadArgs) -> clntSock;
    free(threadArgs);              /* 引数に割り当てられていたメモリを解放 */

    HandleTCPClient(clntSock);

    return (NULL);
}
