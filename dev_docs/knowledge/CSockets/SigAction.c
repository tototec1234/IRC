#include <stdio.h>  /* printf()に必要 */ 
#include <signal.h> /* sigaction()に必要 */
#include <unistd.h> /* pause()に必要 */

void DieWithError(char *errorMessage);       /* エラー処理関数 */
void InterruptSignalHandler(int signalType); /* 割り込みシグナル処理関数 */

int main(int argc, char *argv[])
{
    struct sigaction handler;        /* シグナルハンドラを指定する構造体 */

    /* InterruptSignalHandler()をハンドラ関数として設定 */
    handler.sa_handler = InterruptSignalHandler;
    /* 全シグナルをマスクするマスクを作成 */
    if (sigfillset(&handler.sa_mask) < 0)
        DieWithError("sigfillset() failed");
    /* フラグなし */
    handler.sa_flags = 0;

    /* 割り込みシグナルに対する処理を設定 */
    if (sigaction(SIGINT, &handler, 0) < 0)
        DieWithError("sigaction() failed");

    for(;;)
        pause(); /* シグナルを受け取るまでプログラムを一時停止 */

     exit(0);
}

void InterruptSignalHandler(int signalType)
{
      printf("Interrupt Received.  Exiting program.\n");
      exit(1);
}
