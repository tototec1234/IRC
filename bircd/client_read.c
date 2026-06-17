/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client_read.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: toruinoue <toruinoue@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/09 12:14:14 by torinoue          #+#    #+#             */
/*   Updated: 2026/06/14 03:39:51 by toruinoue        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/*
これは　Lesson 3.3 受信バッファリング設計　において
バッファリングを考えるのが面倒なので
torinoueの get_next_line.cを再利用して　client_read.c　リファクタリングしたソース
課題書添付の　client_read.c　ベースのものは　client_read_subject.c　として残してある
*/

#include "bircd.h"
#include "libft/libft.h"
#include <sys/socket.h>
#include <unistd.h>

static int	get_crlf_pos(char *str, int len);
static int	read_and_store(t_env *e, int cs);
static void	extract_and_consume(t_env *e, int cs, int crlf_end);

/*
** 公開API。GNL の get_next_line 相当の司令塔。
** GNL との違い: poll 駆動なので recv は1イベント1回。
** 続きのデータは次の POLLIN で届く（ブロック回避）。
*/
void	client_read(t_env *e, int cs)
{
	int	r;
	int	crlf_end;

	r = read_and_store(e, cs);
	if (r <= 0)
	{
		close(cs);
		clean_fd(&e->fds[cs]);
		ft_putstr_fd("client ", 1);
		ft_putnbr_fd(cs, 1);
		ft_putstr_fd(" gone away\n", 1);
		return ;
	}
	crlf_end = get_crlf_pos(e->fds[cs].buf_read, e->fds[cs].buf_read_len);
	while (crlf_end)
	{
		extract_and_consume(e, cs, crlf_end);
		crlf_end = get_crlf_pos(e->fds[cs].buf_read,
				e->fds[cs].buf_read_len);
	}
}

/*
** GNL の get_newline_pos 相当。
** "\r\n" の次の添字（= 1メッセージの消費長）を返す。無ければ 0。
** len 走査なのはストリームに '\0' が混ざっても壊れないため。
** RFC2812 2.3: https://datatracker.ietf.org/doc/html/rfc2812#section-2.3
*/
static int	get_crlf_pos(char *str, int len)
{
	int	i;

	i = 0;
	while (i + 1 < len)
	{
		if (str[i] == '\r' && str[i + 1] == '\n')
			return (i + 2);
		i++;
	}
	return (0);
}

/*
** GNL の read_and_store 相当。recv 結果を buf_read に累積。
** GNL は read をループするが、ここでは1回だけ（非ブロッキング設計）。
*/
static int	read_and_store(t_env *e, int cs)
{
	char	tmp[BUF_SIZE];
	int		r;

	/*
	buf_read_len == BUF_SIZE のとき recv(cs, tmp, 0, 0) → 戻り値 0 → 「正常切断」と同じ経路で close。
	結果は クラアントが切断される　で同じだた、、「行が長すぎた = サーバー側で切断」と「相手が切った」の区別が消えている
	r==0 と r==-1 の区別がまだ無い。gone away 一本。88〜89行のコメントに自分で書いてる通り、「行が長すぎ(-1の長すぎパス)」「相手が切った(0)」「recvエラー(-1)」が同じログ。
	
	ft_irc では区別してエラーリプライ（ERROR :Closing Link 等）を返す設計になる。
	
	poll が POLLIN を返しても recv が EAGAIN になる稀ケース（spurious wakeup）。bircd 流儀では r<=0 一括切断で許容。ft_irc では EAGAIN は切断せずスキップが正しい。覚えておけ。
	*/
	if (BUF_SIZE - e->fds[cs].buf_read_len == 0)
		return (-1);  /* 行が長すぎ。呼び出し側で切断される */

	r = recv(cs, tmp, BUF_SIZE - e->fds[cs].buf_read_len, 0);
	if (r <= 0)
		return (r);

	ft_memcpy(e->fds[cs].buf_read + e->fds[cs].buf_read_len, tmp, r);
	e->fds[cs].buf_read_len += r;
	e->fds[cs].buf_read[e->fds[cs].buf_read_len] = '\0';
	return (r);
}

/*
** GNL の extrct_line_and_renew_storage 相当。
** GNL は malloc + strdup で storage を作り直すが、
** 固定バッファなので ft_memmove で前詰めする。
*/
static void	extract_and_consume(t_env *e, int cs, int crlf_end)
{
	e->fds[cs].buf_read[crlf_end - 2] = '\0';
	broadcast_message(e, cs, e->fds[cs].buf_read);
	// 送信済み部分削除
	ft_memmove(e->fds[cs].buf_read,
		e->fds[cs].buf_read + crlf_end,
		e->fds[cs].buf_read_len - crlf_end + 1);
	e->fds[cs].buf_read_len -= crlf_end;
}

/*
バッファ動作確認方法

make && ./bircd 6667
# 別ターミナル2枚:
nc localhost 6667                                  # 受信側
printf 'PING :a\r\nPING :b\r\n' | nc localhost 6667   # 2メッセージ一括 → while の確認
(printf 'PI'; sleep 1; printf 'NG :x\r\n') | nc localhost 6667  # 分割 → 累積の確認
python3 -c "import sys; sys.stdout.buffer.write(b'A'*200)" | nc localhost 6667  # \r\n無し200B → 切断の確認

*/