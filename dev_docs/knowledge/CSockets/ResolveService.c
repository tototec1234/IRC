#include <stdio.h> /* printf()とfprintf()に必要 */
#include <netinet/in.h> /* htons()に必要 */
#include <netdb.h> /* getservbyname()に必要 */
#include <stdlib.h> /* atoi()に必要 */

unsigned short ResolveService(char service[], char protocol[])
{
  struct servent *serv; /* サービス情報を格納する構造体 */
  unsigned short port; /* 戻り値となるポート */

  if ((port = atoi(service)) == 0) /* ポートは数値か？ */
  {
    /* 数値ではない。名前と解釈して検索してみる */
    if ((serv = getservbyname(service, protocol)) == NULL)
    {
      fprintf(stderr, "getservbyname() failed");
      exit(1);
    }
    else
      port = serv->s_port; /* 名前からポート（ネットワークバイト順）が見つかる */
  }
  else
    port = htons(port); /* portをネットワークバイト順に変換する */

  return port;
}
