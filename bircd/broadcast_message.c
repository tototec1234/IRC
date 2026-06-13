#include "bircd.h"
#include "libft/libft.h"

/*
 スタブ: Lesson 3 検証用。
 切り出した1メッセージをサーバの stdout に表示するだけ。
 本実装（他クライアントへ配送）は Lesson 4 で buf_write + POLLOUT に差し替え。
*/
void broadcast_message(t_env *e, int sender_fd, char *msg)
{
	ft_putstr_fd("[fd=", 1);
	ft_putnbr_fd(sender_fd, 1);
	ft_putstr_fd("] msg=[", 1);
	ft_putstr_fd(msg, 1);
	ft_putstr_fd("]\n", 1);

	int		i;
	int		msg_len;
	char	full_msg[BUF_SIZE + 3]; // msg + "\r\n" + '\0'

	/* メッセージに \r\n を付ける */
	ft_strlcpy(full_msg, msg, BUF_SIZE);
	ft_strlcat(full_msg, "\r\n", BUF_SIZE + 3);
	msg_len = ft_strlen(full_msg);

	i = 0;
	while (i < e->maxfd)
	{
		if (e->fds[i].type == FD_CLIENT && i != sender_fd)
		{
			/*
			 バッファ溢れチェック
			溢れたら黙って捨てるのでIRC 的には本来エラー
			*/

			/*
			【設計メモ】固定長 buf_write のクライアント間不整合

			buf_write / buf_write_len はクライアントごとに独立。
			中身の量は各クライアントの送信進捗に依存し、進捗は相手の受信速度で決まる。

			速いクライアント:
				poll が POLLOUT を返す → client_write が send → 箱が空く
				→ 次の broadcast を全部受け取れる
			遅い/詰まったクライアント:
				send しきれず箱に残る → buf_write_len 高止まり
				→ 上の溢れチェックに引っかかり新メッセージが捨てられる

			結果: 同じ broadcast でも、速いクライアントは全受信、
					遅いクライアントは欠落。配送の不整合が現実に起きる。

			【方針】
			- 溢れの閾値を上げるだけの対症療法は採らない（根本解決にならない）。
			- ft_irc 提出版では送信キューを std::string 化して固定長制約を外す。
			ただし青天井は逆にメモリ枯渇リスク（受信せず垂れ流す悪意/故障
			クライアント）。
			- そこで SendQ リミットを設け、一定量を超えたクライアントは
			"Send queue exceeded" で切断する。
			「全員に均等配送」より「サーバを守る」を優先するのが
			実在 ircd（charybdis 等）の挙動。

			【レビューではテストが難しい】
			buf_write は BUF_SIZE=100 の極小アプリバッファだが、
			その手前に OS の TCP 送信バッファ（数十KB〜）＋ 受信側 TCP 受信バッファ（数十KB〜） があるらしい
			client_write の send はまず OS バッファに吸収される（文字通りバッファ）。
			OS バッファが満杯になって初めて send が詰まり、buf_write に残り、
			次の broadcast で 100 超 → "exceeded"。
			つまりkill STOP <pid 詰まらせる対象> しても、数百KB 流さないとアプリ層は溢れない。
			少量メッセージだと OS が飲み込んで何も起きないみたい

			送信側クライアント（詰まらせるためにSTOPさせた受信側とは別のnc）
			python3 -c "import sys
			for i in range(100000): sys.stdout.write('PING :flood%d\r\n' % i)" | nc localhost 6667
 			*/

			/* charybdis ircd/send.c の SendQ 超過検出をデバッグ出力で模倣。
			本家は超過で dead_link(切断)。ここではログのみ、切断は後段で実装。
			https://github.com/charybdis-ircd/charybdis/blob/master/ircd/send.c#L71
			 */
			if (e->fds[i].buf_write_len + msg_len > BUF_SIZE)   // ① 超過判定
			{
				/* ② ログ: "Max SendQ limit exceeded for <fd>: <現量+追加> > <上限>" */
				ft_putstr_fd("Max SendQ limit exceeded for fd ", 1);
				ft_putnbr_fd(i, 1);
				ft_putstr_fd(": ", 1);
				ft_putnbr_fd(e->fds[i].buf_write_len + msg_len , 1);
				ft_putstr_fd(" > ", 1);
				ft_putnbr_fd(BUF_SIZE, 1);
				ft_putstr_fd("\n", 1);
				/* ③ 切断は今回しない。*/
			}
			else	// ④ 上限内なら追記（既存ロジック）
			{
				ft_strlcat(e->fds[i].buf_write, full_msg, BUF_SIZE + 1);
				e->fds[i].buf_write_len += msg_len;
		// ここで「他クライアントの buf_write に追加したので POLLOUT で送信してもらう」実装になる（Lesson 4）。
		// 現在の poll 実装では、client_write が buf_write_len > 0 を検出した際に POLLOUT をセットし、送信する。
			}
		}
		i++;
	}
}
