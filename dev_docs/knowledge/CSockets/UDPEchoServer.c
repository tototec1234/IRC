#include <stdio.h> /* printf()、fprintf()に必要 */
#include <sys/socket.h> /* socket()、bind()に必要 */
#include <arpa/inet.h> /* sockaddr_in、inet_ntoa()に必要 */
#include <stdlib.h> /* atoi()に必要 */
#include <string.h> /* memset()に必要 */
#include <unistd.h> /* close()に必要 */

#define ECHOMAX 255 /* エコー文字列の最大長 */

void DieWithError(char *errorMessage); /* 外部エラー処理関数 */

int main(int argc, char *argv[])
{
  int sock; /* ソケット */
  struct sockaddr_in echoServAddr; /* ローカルアドレス */
  struct sockaddr_in echoClntAddr; /* クライアントアドレス */
  unsigned int cliAddrLen; /* 着信メッセージの長さ */
  char echoBuffer[ECHOMAX]; /* エコー文字列用バッファ */
  unsigned short echoServPort; /* サーバのポート番号 */
  int recvMsgSize; /* 受信メッセージのサイズ */

  if (argc != 2) /* パラメータの数が正しいか確認 */
  {
    fprintf(stderr,"Usage: %s <UDP SERVER PORT>\n", argv[0]);
    exit(1);
  }

  echoServPort = atoi(argv[1]); /* 1つ目の引数：ローカルポート番号 */

  /* データグラムの送受信に使うソケットを作成 */
  if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    DieWithError("socket() failed");

  /* ローカルのアドレス構造体を作成 */
  memset(&echoServAddr, 0, sizeof(echoServAddr)); /* 構造体にゼロを埋める */
  echoServAddr.sin_family = AF_INET; /* インターネットアドレスファミリ */
  echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* ワイルドカードを使用 */
  echoServAddr.sin_port = htons(echoServPort); /* ローカルポート */

  /* ローカルアドレスへバインド */
  if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
    DieWithError("bind() failed");

  for (;;) /* プログラムが終了するまで繰り返し実行 */
  {
    /* 入出力パラメータのサイズをセット */
    cliAddrLen = sizeof(echoClntAddr);

    /* クライアントからメッセージを受信するまでブロックする  */
    if ((recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0,

      (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
      DieWithError("recvfrom() failed");

    printf("Handling client %s\n", inet_ntoa(echoClntAddr. sin_addr));

    /* 受信したデータグラムをクライアントに返信する */
    if (sendto(sock, echoBuffer, recvMsgSize, 0,
      (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != recvMsgSize)
      DieWithError("sendto() sent a different number of bytes than expected");
  }
  /* この部分には到達しない*/
}
