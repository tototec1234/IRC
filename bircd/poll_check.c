#include <stdio.h>
#include <poll.h>

// 数値を8ビットの2進数（01列）の文字列にして表示する関数
void print_binary8(short value)
{
    // 上位ビットから順番に1が立っているかチェックしていく
    for (int i = 7; i >= 0; i--)
    {
        // (1 << i) で調べたいマス目だけに 1 を立てたマスクを作り、& で判定
        if (value & (1 << i))
            printf("1");
        else
            printf("0");

        // 4ビットごとに半角スペースを入れて見やすくする
        if (i == 4)
            printf(" ");
    }
}

int main(void)
{
    printf("POLLIN  : "); print_binary8(POLLIN);   printf("  (0x%04X)\n", POLLIN);
    printf("POLLPRI : "); print_binary8(POLLPRI);  printf("  (0x%04X)\n", POLLPRI);
    printf("POLLOUT : "); print_binary8(POLLOUT);  printf("  (0x%04X)\n", POLLOUT);
    printf("POLLERR : "); print_binary8(POLLERR);  printf("  (0x%04X)\n", POLLERR);
    printf("POLLHUP : "); print_binary8(POLLHUP);  printf("  (0x%04X)\n", POLLHUP);
    printf("POLLNVAL: "); print_binary8(POLLNVAL); printf("  (0x%04X)\n", POLLNVAL);
    
    return (0);
}