#include <stdio.h>  /* printf()、fprintf()に必要 */
#include <sys/socket.h> /* recv()、send()に必要 */
#include <unistd.h> /* close()に必要 */

#define RCVBUFSIZE 32 /* 受信バッファのサイズ */

void DieWithError(char *errorMessage);  /* エラー処理関数 */

void HandleTCPClient(int clntSocket)
{
  char echoBuffer[RCVBUFSIZE];  /* エコー文字列のバッファ */
  int recvMsgSize;  /* 受信メッセージのサイズ */

  /* クライアントからの受信メッセージ */
  if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
    DieWithError("recv() failed");

  /* 受信した文字列を送信し、転送が終了していなければ次を受信する */
  while (recvMsgSize > 0) /* ゼロは転送の終了を意味する */
  {
    /* メッセージをクライアントにエコーバックする */
    if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
      DieWithError("send() failed");

    /* 受信するデータが残っていないか確認する */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
      DieWithError("recv() failed");
  }

  close(clntSocket);  /* クライアントソケットをクローズする */
}
