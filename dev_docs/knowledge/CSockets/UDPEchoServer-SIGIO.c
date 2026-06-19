#include <stdio.h>     /* printf()��fprintf()��ɬ�� */
#include <sys/socket.h> /* socket()��bind()��connect()��ɬ�� */
#include <arpa/inet.h>  /* sockaddr_in��inet_ntoa()��ɬ�� */
#include <stdlib.h>    /* atoi()��ɬ�� */
#include <string.h>    /* memset()��ɬ�� */
#include <unistd.h>    /* close()��ɬ�� */
#include <fcntl.h>     /* fcntl()��ɬ�� */
#include <sys/file.h>  /* O_NONBLOCK��FASYNC��ɬ�� */
#include <signal.h>     /* signal()��SIGALRIA��ɬ�� */
#include <errno.h>     /* errno��ɬ�� */

#define ECHOMAX 255    /* ������ʸ����κ���Ĺ */

void DieWithError(char *errorMessage); /* ���顼�����ؿ� */
void UseIdleTime();                    /* �����ɥ���֤����Ѥ���ؿ� */
void SIGIOHandler(int signalType);     /* SIGIO�Υϥ�ɥ�ؿ� */

int sock;                        /* �����åȡʥ����ʥ�ϥ�ɥ��Ѥ˥������Х�˺����� */

int main(int argc, char *argv[])
{
    struct sockaddr_in echoServAddr;  /* �����ФΥ��ɥ쥹 */
    unsigned short echoServPort;      /* �����ФΥݡ����ֹ� */
    struct sigaction handler;         /* �����ʥ�����Υ������������ */

   /* �ѥ�᡼���ο�������������ǧ */
   if (argc != 2)
   {
       fprintf(stderr,"Usage: %s <SERVER PORT>\n", argv[0]);
       exit(1);
   }

   echoServPort = atoi(argv[1]); /* 1���ܤΰ����� ��������ݡ����ֹ� */

   /* �ǡ����������������˻Ȥ������åȤ���� */
   if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
      DieWithError("socket() failed");

   /* �����ФΥ��ɥ쥹��¤�Τ򥻥åȥ��å� */
   memset(&echoServAddr, 0, sizeof(echoServAddr));    /* ��¤�Τ˥��������� */
   echoServAddr.sin_family = AF_INET;                 /* ���󥿡��ͥåȥե��ߥ� */
   echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Ǥ�դμ������󥿥ե����� */
   echoServAddr.sin_port = htons(echoServPort); /* �ݡ��� */

   /* �������륢�ɥ쥹�إХ���� */
   if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
       DieWithError("bind() failed");

   /* SIGIO�Υ����ʥ�ϥ�ɥ������ */
   handler.sa_handler = SIGIOHandler;
   /* �������ʥ��ޥ�������ޥ�������� */
   if (sigfillset(&handler.sa_mask) < 0)
     DieWithError("sigfillset() failed");
   /* �ե饰�ʤ� */
   handler.sa_flags = 0;

   if (sigaction(SIGIO, &handler, 0) < 0)
       DieWithError("sigaction() failed for SIGIO");

   /* �����åȤ��ͭ����SIGIO��å���������������� */
   if (fcntl(sock, F_SETOWN, getpid()) < 0)
     DieWithError("Unable to set process owner to us");

   /* �Υ�֥��å���I/O��SIGIO�����򥻥åȥ��å� */
   if (fcntl(sock, F_SETFL, O_NONBLOCK | FASYNC) < 0)
       DieWithError("Unable to put client sock into nonblocking/async mode");

   /* ������Υ��Ƥۤ��λŻ��򤹤�ʥ������ϥХå����饦��ɤ�ȯ������� */

   for (;;)
       UseIdleTime();

   /* ������ʬ�ˤ���ã���ʤ� */
}

void UseIdleTime()
{
    printf(".\n");
    sleep(3);     /* 3�ô�ư��� */
}

void SIGIOHandler(int signalType)
{
    struct sockaddr_in echoClntAddr; /* �ǡ������������������ɥ쥹 */
    unsigned int clntLen;            /* ���ɥ쥹Ĺ */
    int recvMsgSize;                /* �ǡ��������Υ����� */
    char echoBuffer[ECHOMAX];       /* �ǡ��������ΥХåե� */

    do /* ���ϥǡ������ʤ��ʤ�ޤǼ¹� */
    {
       /* �����ϥѥ�᡼���Υ����������� */
       clntLen = sizeof(echoClntAddr);

       if ((recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0,
              (struct sockaddr *) &echoClntAddr, &clntLen)) < 0)
       {
           /* recvfrom()���֥��å����褦�Ȥ������ʵ��Ƥ����ͣ��Υ��顼�� */
           if (0 != EWOULDBLOCK)
               DieWithError("recvfrom() failed");
       }
       else
       {
           printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

           if (sendto(sock, echoBuffer, recvMsgSize, 0, (struct sockaddr *)
                 &echoClntAddr, sizeof(echoClntAddr)) != recvMsgSize)
               DieWithError("sendto() failed");
        }
     } while (recvMsgSize >= 0);
     /* ���٤Ƥμ�������λ */
 }
