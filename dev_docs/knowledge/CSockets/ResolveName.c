#include <stdio.h> /* fprintf()に必要 */
#include <netdb.h> /* gethostbyname()に必要 */

unsigned long ResolveName(char name[])
{
  struct hostent *host; /* ホスト情報を格納する構造体 */

  if ((host = gethostbyname(name)) == NULL)
  {
    fprintf(stderr, "gethostbyname() failed");
    exit(1);
  }

  /* ネットワークバイト順で表したバイナリのIPアドレスを返す */
  return *((unsigned long *)host->h_addr_list[0]);
}
