#include <stdio.h>      /* printf()��fprintf()��ɬ�� */
#include <sys/socket.h> /* socket()��bind()��ɬ�� */
#include <arpa/inet.h>  /* sockaddr_in��ɬ�� */
#include <stdlib.h>     /* atoi()��ɬ�� */
#include <string.h>     /* memset()��ɬ�� */
#include <unistd.h>     /* close()��ɬ�� */

void DieWithError(char *errorMessage); /* �������顼�����ؿ� */

int main(int argc, char *argv[])
 {
     int sock;                         /* �����å� */
     struct sockaddr_in broadcastAddr; /* �֥����ɥ��㥹�ȥ��ɥ쥹 */
     char *broadcastIP;                /* IP�֥����ɥ��㥹�ȥ��ɥ쥹 */
     unsigned short broadcastPort;     /* �����ФΥݡ��� */
     char *sendString;                 /* �֥����ɥ��㥹�Ȥ���ʸ���� */
     int broadcastPermission;          /* �֥����ɥ��㥹�Ȥ�ͭ���ˤ��륽���åȥ��ץ���� */
     unsigned int sendStringLen;       /* �֥����ɥ��㥹�Ȥ���ʸ�����Ĺ�� */

     if (argc < 4)                     /* �ѥ�᡼���ο�������������ǧ */
     {
         fprintf(stderr,"Usage: %s <IP Address> <Port> <Send String>\n", argv[0]);
         exit(1);
     }

     broadcastIP = argv[1];           /* 1���ܤΰ������֥����ɥ��㥹�Ȥ���IP���ɥ쥹 */
     broadcastPort = atoi(argv[2]);   /* 2���ܤΰ������֥����ɥ��㥹���ѥݡ��� */
     sendString = argv[3] ;           /* 3���ܤΰ������֥����ɥ��㥹�Ȥ���ʸ���� */

     /* �ǡ������������������륽���åȤ���� */
     if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
         DieWithError("socket() failed");

     /* �����åȤ�֥����ɥ��㥹�Ȳ�ǽ������ */
     broadcastPermission = 1;
     if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission,
           sizeof(broadcastPermission)) < 0)
         DieWithError("setsockopt() failed");

     /* ��������Υ��ɥ쥹��¤�Τ���� */
     memset(&broadcastAddr, 0, sizeof(broadcastAddr));    /* ��¤�Τ򥼥������� */
     broadcastAddr.sin_family = AF_INET;                  /* ���󥿡��ͥåȥ��ɥ쥹�ե��ߥ� */
     broadcastAddr.sin_addr.s_addr = inet_addr(broadcastIP);/* �֥����ɥ��㥹��IP���ɥ쥹 */
     broadcastAddr.sin_port = htons(broadcastPort);          /* �֥����ɥ��㥹�ȥݡ��� */

     sendStringLen = strlen(sendString);  /* sendString��Ĺ����Ĵ�٤� */
     for (;;) /* �ץ�����ब��λ����ޤǷ����֤��¹� */
     {
         /* sendString�Υǡ��������򥯥饤����Ȥ�3�ä��Ȥ˥֥����ɥ��㥹�Ȥ���*/
         if (sendto(sock, sendString, sendStringLen, 0, (struct sockaddr *)
               &broadcastAddr, sizeof(broadcastAddr)) != sendStringLen)
             DieWithError("sendto() sent a different number of bytes than expected");

         sleep(3);  /* �ͥåȥ�������äѤ��ˤʤ�Τ��ɤ� */
      }
      /* ������ʬ�ˤ���ã���ʤ� */
 }
