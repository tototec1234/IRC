#include <stdio.h> /* printf()、fprintf()に必要 */
#include <sys/socket.h> /* socket()、connect()、sendto()、recvfrom()に必要 */
#include <arpa/inet.h> /* sockaddr_in、inet_addr()に必要 */
#include <stdlib.h> /* atoi()に必要 */
#include <string.h> /* memset()に必要 */
#include <unistd.h> /* for close()に必要 */

#define ECHOMAX 255 /* エコー文字列の最大長 */

void DieWithError(char *errorMessage); /* 外部エラー処理関数 */

int main(int argc, char *argv[])
{
  int sock; /* ソケットディスクリプタ */
  struct sockaddr_in echoServAddr; /* エコーサーバのアドレス */
  struct sockaddr_in fromAddr; /* エコー送信元のアドレス */
  unsigned short echoServPort; /* エコーサーバのポート番号 */
  unsigned int fromSize; /* recvfrom()のアドレスの入出力サイズ */
  char *servIP; /* サーバのIPアドレス */
  char *echoString; /* エコーサーバへ送信する文字列 */
  char echoBuffer[ECHOMAX+1]; /* エコー文字列の受信用バッファ */
  int echoStringLen; /* エコー文字列の長さ */
  int respStringLen; /* 受信した応答の長さ */

  if ((argc < 3) || (argc > 4)) /* 引数の数が正しいか確認 */
  {
    fprintf(stderr,"Usage: %s <Server IP> <Echo Word> [<Echo Fort>]\n", argv[0]);
    exit(1);
  }

  servIP = argv[1]; /* 1つ目の引数：サーバのIPアドレス（dotted quad）*/
  echoString = argv[2]; /* 2つ目の引数：エコー文字列 */

  if ((echoStringLen = strlen(echoString)) > ECHOMAX) /* 入力した文字列の長さをチェック */
    DieWithError("Echo word too long");

  if (argc == 4)
    echoServPort = atoi(argv[3]); /* 指定のポート番号があれば使用 */
  else
    echoServPort = 7; /* 7はエコーサービスのwell-knownポート番号 */

  /* UDPデータグラムソケットの作成 */
  if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    DieWithError("socket() failed");

  /* サーバのアドレス構造体の作成 */
  memset(&echoServAddr, 0, sizeof(echoServAddr)); /* 構造体にゼロを埋める */
  echoServAddr.sin_family = AF_INET; /* インターネットアドレスファミリ */
  echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* サーバのIPアドレス */
  echoServAddr.sin_port = htons(echoServPort); /* サーバのポート番号 */

  /* 文字列をサーバに送信 */
  if (sendto(sock, echoString, echoStringLen, 0, (struct sockaddr *)
         &echoServAddr, sizeof(echoServAddr)) != echoStringLen)
    DieWithError("sendto() sent a different   number of bytes than expected");

  /* 応答を受信 */
  fromSize = sizeof(fromAddr);
  if ((respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0,
    (struct sockaddr *) &fromAddr, &fromSize)) != echoStringLen)
    DieWithError("recvfrom() failed");

  if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
  {
    fprintf(stderr,"Error: received a packet from unknown source.\n");
    exit(1);
  }

  /* 受信データをNULL文字で終端させる */
  echoBuffer[respStringLen] = '\0';
  printf("Received: %s\n", echoBuffer); /* 引数のエコー文字列を表示 */

  close(sock);
  exit(0);
}
