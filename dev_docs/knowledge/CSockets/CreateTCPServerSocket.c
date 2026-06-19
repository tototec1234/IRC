#include <sys/socket.h> /* socket()、bind()、connect()に必要 */
#include <arpa/inet.h>  /* sockaddr_in and inet_ntoa()に必要 */
#include <string.h>     /* memset()に必要 */

#define MAXPENDING 5    /* 未処理の接続要求の最大値 */

void DieWithError(char *errorMessage); /* エラー処理関数 */

int CreateTCPServerSocket(unsigned short port)
{
    int sock;                        /* 作成するソケット */
    struct sockaddr_in echoServAddr; /* ローカルアドレス */

    /* 着信接続要求に対するソケットを作成 */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* ローカルのアドレス構造体を作成 */
    memset(&echoServAddr, 0, sizeof(echoServAddr));    /* 構造体をゼロで埋める */
    echoServAddr.sin_family = AF_INET;                /* インターネットアドレスファミリ */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* ワイルドカードを使用 */
    echoServAddr.sin_port = htons(port);              /* ローカルポート */

    /* ローカルアドレスへバインド */
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");

    /* 着信接続要求のリスン中というマークをソケットに付ける */
    if (listen(sock, MAXPENDING) < 0)
        DieWithError("listen() failed");

    return sock;
}
