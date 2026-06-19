#include "TCPEchoServer.h"  /* TCPエコーサーバのヘッダファイルをインクルード */
#include <sys/wait.h>       /* waitpid()に必要 */

int main(int argc, char *argv[])
{
    int servSock;                     /* サーバのソケットディスクリプタ */
    int clntSock;                     /* クライアントのソケットディスクリプタ */
    unsigned short echoServPort;      /* サーバのポート番号 */
    pid_t processID;                  /* fork()が返すプロセスID */
  unsigned int childProcCount = 0;   /* 子プロセスの数 */

  if (argc != 2)    /* 引数の数が正しいか確認 */
  {
      fprintf(stderr, "Usage: %s <Server Port>", argv[0]);
      exit(1);
  }

  echoServPort = atoi(argv[1]); /* 1つ目の引数： ローカルポート */

  servSock = CreateTCPServerSocket(echoServPort);

  for (;;) /* 処理を繰り返し実行 */
  {
      clntSock = AcceptTCPConnection(servSock);
      /* 子プロセスのフォークとエラーを報告 */
     if ((processID = fork()) < 0)
         DieWithError("fork() failed");
     else if (processID == 0) /* 子プロセスの場合 */
    {
         close(servSock);   /* 待ち受け中のソケットを子プロセスがクローズ */
         HandleTCPClient(clntSock);

         exit(0);           /* 子プロセスを終了 */
    }

    printf("with child process: %d\n", (int) processID);
    close(clntSock);       /* 子のソケットディスクリプタを親がクローズ */
    childProcCount++;      /* 未回収の子プロセスの数をインクリメント */

    while (childProcCount) /* 全ゾンビをクリーンアップ */
    {
        processID = waitpid((pid_t) -1, NULL, WNOHANG); /* ノンブロッキングで実行 */
        if (processID < 0) /* waitpid()のエラーを確認 */
            DieWithError("waitpid() failed");
        else if (processID == 0)  /* ゾンビが存在しない */
            break;
        else
            childProcCount--;  /* 子プロセスを回収 */
    }
  }
  /* この部分には到達しない */
}
