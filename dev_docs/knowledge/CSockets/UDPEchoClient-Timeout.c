#include <stdio.h>      /* printf()、fprintf()に必要 */
#include <sys/socket.h> /* socket()、connect()、sendto()、recvfrom()に必要 */
#include <arpa/inet.h>  /* sockaddr_in、inet_addr()に必要 */
#include <stdlib.h>     /* atoi()に必要 */
#include <string.h>     /* memset()に必要 */
#include <unistd.h>     /* close()に必要 */
#include <errno.h>      /* errno、EINTRに必要 */
#include <signal.h>     /* sigaction()に必要 */

#define ECHOMAX        255    /* エコー文字列の最大長 */
#define TIMEOUT_SECS   2      /* 再送信までの秒数 */
#define MAXTRIES       5      /* 最大試行回数 */

int tries=0;   /* 送信回数のカウンタ（シグナルハンドラからのアクセス用、グローバル） */

void DieWithfrror(char *errorMessage);   /* エラー処理関数 */
void CatchAlarm(int ignored);            /* SIGALRMのハンドラ */

int main(int argc, char *argv[])
{
    int sock;                       /* ソケットディスクリプタ */
    struct sockaddr_in echoServAddr; /* エコーサーバのアドレス */
    struct sockaddr_in fromAddr;     /* エコー送信元のアドレス */
    unsigned short echoServPort;    /* エコーサーバのポート番号 */
    unsigned int fromSize;        /* recvfrom()のアドレスの入出力サイズ */
    struct sigaction myAction;      /* シグナルハンドラの設定用 */
    char *servIP;                   /* サーバのIPアドレス */
    char *echoString;               /* エコーサーバへ送信する文字列 */
    char echoBuffer[ECHOMAX+1];     /* エコー文字列の受信バッファ */
    int echoStringLen;              /* エコー文字列の長さ */
    int respStringLen;              /* 受信データグラムの長さ */
   
    if ((argc < 3) || (argc > 4))   /* 引数の数が正しいか確認 */
    {
        fprintf(stderr,"Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
        exit(1);
    }
   
    servIP = argv[1];           /* 1つ目の引数： サーバのIPアドレス（ドット区切り10進表記） */
    echoString = argv[2];       /* 2つ目の引数： エコー文字列 */
   
    if ((echoStringLen = strlen(echoString)) > ECHOMAX)

    DieWithError("Echo word too long");
   
    if (argc == 4)
        echoServPort = atoi(argv[3]); /* 指定のポート番号があれば使用 */
    else
        echoServPort = 7; /* エコーサービスのwell-known番号 */
   
    /* ベストエフォート型UDPデータグラムソケットを作成 */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");
   
    /* アラームシグナル用シグナルハンドラのセット */
    myAction.sa_handler = CatchAlarm;
    if (sigfillset(&myAction.sa_mask) < 0) /* ハンドラ内ではすべてをブロック */
    DieWithError("sigfillset() failed");
    myAction.sa_flags = 0;
   
    if (sigaction(SIGALRM, &myAction, 0) < 0)
        DieWithError("sigaction() failed for SIGALRM");
   
    /* サーバのアドレス構造体を作成 */
    memset(&echoServAddr, 0, sizeof(echoServAddr));  /* 構造体をゼロで埋める */

    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* サーバのIPアドレス */
    echoServAddr.sin_port = htons(echoServPort);       /* サーバのポート番号 */
   
    /* 文字列をサーバに送信 */
    if (sendto(sock, echoString, echoStringLen, 0, (struct sockaddr *)
               &echoServAddr, sizeof(echoServAddr)) != echoStringLen)
        DieWithError("sendto() sent a different number of bytes than expected");
   
    /* 応答を受信 */
   
    fromSize = sizeof(fromAddr);
    alarm(TIMEOUT_SECS);        /* タイムアウト時間を設定 */
    while ((respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0,
           (struct sockaddr *) &fromAddr, &fromSize)) < 0)
        if (errno == EINTR)     /* アラームの終了 */
        {
            if (tries < MAXTRIES) /* triesはシグナルハンドラ内でインクリメント */
            {
                printf("timed out, %d more tries...\n", MAXTRIES-tries);
                if (sendto(sock, echoString, echoStringLen, 0, (struct sockaddr *)
                            &echoServAddr, sizeof(echoServAddr)) != echoStringLen)
                    DieWithError("sendto() failed");
                alarm(TIMEOUT_SECS);
             }
             else
                 DieWithError("No Response");
         }
         else
             DieWithError("recvfrom() failed");
   
    /* recvfrom()が何かを受信したら、タイムアウトをキャンセル */
    alarm(0);
   
    /* 受信データをNULL文字で終端させる */
    echoBuffer[respStringLen] = '\0';
    printf("Received: %s\n", echoBuffer);  /* 受信データを表示 */
    
     close(sock);
     exit(0);
}

void CatchAlarm(int ignored)    /* SIGALRMのハンドラ */
{
    tries += 1;
}
