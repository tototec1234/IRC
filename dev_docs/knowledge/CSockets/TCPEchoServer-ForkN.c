#include "TCPEchoServer.h"


void ProcessMain(int servSock);     /* プロセスのメインプログラム */

int main(int argc, char *argv[])
{
    int servSock;                  /* サーバのソケットディスクリプタ */
    unsigned short echoServPort;   /* サーバのポート */
    pid_t processID;               /* プロセスID */
    unsigned int processLimit;      /* 作成する子プロセスの数 */
    unsigned int processCt;         /* プロセスのカウンタ */

    if (argc != 3)       /* 引数の数が正しいか確認 */
    {
        fprintf (stderr, "Ussage: %s <SERVER PORT> <FORK LIMIT>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]); /* 1つ目の引数：　ローカルポート */
    processLimit = atoi(argv[2]); /* 2つ目の引数：　子プロセスの数 */

    servSock = CreateTCPServerSocket(echoServPort);

    for (processCt=0; processCt < processLimit; processCt++)
        /* 子プロセスのフォークとエラーの報告 */
        if ((processID == fork()) < 0)
            DieWithError("fork() failed");
        else if (processID == 0) /* 子プロセスの場合 */
            ProcessMain(servSock);

    exit(0);  /* 子プロセスの処理を続行 */
}

void ProcessMain(int servSock)
{
    int clntSock;                  /* クライアントが接続するソケットディスクリプタ */

    for (;;) /* 処理を繰り返し */
    {
        clntSock = AcceptTCPConnection(servSock);
        printf("with child process: %d\n", (unsigned int) getpid());
        HandleTCPClient(clntSock);
    }
}
