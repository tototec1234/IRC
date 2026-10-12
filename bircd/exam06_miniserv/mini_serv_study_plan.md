# mini_serv 学習プラン（学習 8 時間、試験 2026-10-13 15:00）

> 作成日: 2026-10-12 16:45 JST
> 時間割の改訂: 2026-10-12 16:57 JST（開始 17:00、校舎への移動、睡眠、試験開始 15:00 を反映）
> 前提: [セッションログ #0000](session_logs/0000_session_log_exam06.md)、[課題文 和訳](mini_serv_ja.md)、[データフロー](mini_serv_data_flow.md)
> 進め方: ドライバー（torinoue）が書き、AI は Navigator として穴埋めのスケルトンとレビューを出す。完成コードは書かない。

---

## 1. 前提

| 項目 | 前提 |
|------|------|
| 学習時間 | 8 時間（夕方 3 時間 15 分、朝 4 時間 45 分） |
| 試験 | 2026-10-13 15:00 開始、3 時間 |
| 固定の予定 | 18:00–18:45 校舎へ移動。21:00–08:00 睡眠と食事など。13:30–15:00 昼食と休憩 |
| 環境 | 夕方は 18:00 まで Mac、18:45 から校舎の Linux。朝は Mac で始め、校舎の Linux での確認が必要になった時点で 45 分かけて校舎へ移動する |
| コードの受け渡し | 校舎で書いたコードを、朝の Mac で続けられるようにする。21:00 までに `git push` などで手元へ移す（git の操作はドライバーが行う） |
| 配布 `main.c` | 試験では配られる前提にする。ただし、配られなくても socket の準備と行の切り出しを自力で書けるようにする（付録 A） |
| 完成品の観察 | Mac で akadjoker 版を 10 分だけ触る。読まないクライアントの挙動は、自分の実装で確かめる |
| 通し練習 | 最後に、時間を測って白紙から書き直す。自分の完成版も配布 `main.c` も見ない |
| チャット | 段階 1 からは新しいチャットで進める。指示文は付録 C |

---

## 2. 時間割

### 夕方（2026-10-12）

| 時刻 | 分 | 内容 | 場所 | 終わりの判定 |
|---|---|---|---|---|
| 17:00–17:10 | 10 | 完成品の観察（`nc`） | Mac | 3.0 の 5 項目を自分の目で見た |
| 17:10–17:20 | 10 | 配布 `main.c` を読む | Mac | 付録 A の「変える箇所」を自分の言葉で言える |
| 17:20–17:40 | 20 | 段階 1 | Mac | 引数なしで `Wrong number of arguments\n` と終了 1 |
| 17:40–18:00 | 20 | 段階 2 | Mac | `nc 127.0.0.1 ポート` が繋がる。使用中ポートで `Fatal error\n` と終了 1 |
| 18:00–18:45 | 45 | 校舎へ移動 | | 校舎の Linux でビルドできる状態 |
| 18:45–19:35 | 50 | 段階 3 | Linux | 2 本目の `nc` で、1 本目に `server: client 1 just arrived\n` |
| 19:35–20:05 | 30 | 段階 4 | Linux | 1 本切ると、残りに `server: client 0 just left\n`。次の ID が続き番号 |
| 20:05–20:45 | 40 | 段階 5 | Linux | `hello\nworld\n` が 2 行になって他の全員に届く。送信者には届かない |
| 20:45–21:00 | 15 | 引き継ぎ | Linux | 3.10 の 3 点が済んだ |
| 21:00–08:00 | | 睡眠と食事など | | |

### 朝（2026-10-13）

| 時刻 | 分 | 内容 | 場所 | 終わりの判定 |
|---|---|---|---|---|
| 08:00–08:35 | 35 | 段階 6 | Mac | `hel` と `lo\n` を分けて送ると、相手には 1 行で届く |
| 08:35–09:35 | 60 | 段階 7 の実装 | Mac | `nc` 3 本で、arrived、行、left がすべて送信待ち経由で届く |
| 09:35–10:20 | 45 | 校舎へ移動 | | 校舎の Linux でビルドできる状態 |
| 10:20–10:50 | 30 | 段階 7 の Linux での確認 | Linux | 3.7 の確認がすべて通る |
| 10:50–11:00 | 10 | 休憩 | | |
| 11:00–12:30 | 90 | 通し練習（白紙から） | Linux | 90 分以内に段階 7 まで動く |
| 12:30–13:30 | 60 | 振り返りと弱点の再練習 | Linux | 付録 B のチェックリストを見ずに言える |
| 13:30–15:00 | 90 | 昼食と休憩 | | |
| 15:00– | | 試験 | | |

学習時間は夕方 195 分、朝 285 分で、合計 480 分。移動 2 回の 90 分は含めていない。

段階 6 と段階 7 の実装は Mac で行い、Linux が必要な確認（読まないクライアントと valgrind）の直前に校舎へ移動する。

---

## 3. 各段階でやること

段階ごとに、新しいチャットで次の順に進める。

1. AI が、その段階の論点と穴埋めスケルトンを出す。
2. ドライバーが vim で `bircd/exam06_miniserv/mini_serv.c` に書く。
3. ドライバーが `nc` で確かめ、コードを貼る。
4. AI がレビューする。
5. 終わりの判定（2 の表）が通ったら次へ進む。

ビルドは毎回同じコマンドにする。

```bash
gcc -Wall -Wextra -Werror mini_serv.c -o mini_serv
```

### 3.0 完成品の観察（10 分、Mac）

akadjoker 版は `/tmp/mini_serv_ref2/array/mini_serv` にビルド済み（Mac）。ソースは開かない。

ターミナルを 4 枚使う。1 枚目でサーバーを起動し、2〜4 枚目で `nc 127.0.0.1 ポート` を開く。

見るもの:

1. 引数なしで起動したときの標準エラーと `echo $?`
2. `nc` を 2 本、3 本と繋いだときに、既に居る側に出る行
3. `nc` で `hello` と打って Enter。誰に届き、誰に届かないか
4. `nc` で `hel` と打ち、Enter を押さずに Ctrl+D、続けて `lo` と Enter。相手に何がいつ届くか（macOS の `nc` で Ctrl+D が入力の途中送信になるかも、ここで確かめる）
5. `nc` を 1 本 Ctrl+C で切ったときに、残りに出る行。次に繋いだ `nc` の ID

読まないクライアントは、ここでは見ない。セッションログ #0000 のとおり、akadjoker 版は Mac でこの確認の途中で止まった。

### 3.1 段階 1（20 分、Mac）

- 論点: 引数の数の検査、標準エラーへの出力、終了ステータス、`Fatal error` の出口を 1 か所にまとめること。
- 根拠: 課題文の 9〜11 行目。[man 2 write](https://man7.org/linux/man-pages/man2/write.2.html)、[man 3 exit](https://man7.org/linux/man-pages/man3/exit.3.html)。

### 3.2 段階 2（20 分、Mac）

- 論点: `socket` / `bind` / `listen` の失敗を、段階 1 の出口へ送ること。待ち受けアドレスを 127.0.0.1 にすること。ポートを `argv[1]` から取ること。
- 根拠: 配布 `main.c`（付録 A）。[man 2 socket](https://man7.org/linux/man-pages/man2/socket.2.html)、[man 2 bind](https://man7.org/linux/man-pages/man2/bind.2.html)、[man 2 listen](https://man7.org/linux/man-pages/man2/listen.2.html)、[man 7 ip](https://man7.org/linux/man-pages/man7/ip.7.html)。

### 3.3 段階 3（50 分、Linux）

- 論点: 監視する fd の一覧と、毎周複写する作業用の集合。`select` の第 1 引数。listen fd が readable なら `accept`。ID の発行。arrived の宛先。
- bircd の対応: `init_fd` / `do_select` / `check_fd` / `srv_accept`。
- 根拠: [man 2 select](https://man7.org/linux/man-pages/man2/select.2.html)、[man 2 accept](https://man7.org/linux/man-pages/man2/accept.2.html)。
- この段階では、arrived を直接 `send` してよい。段階 7 で送信待ち経由に変える。

### 3.4 段階 4（30 分、Linux）

- 論点: `recv` が 0 以下のときの処理。監視の一覧から外すこと、`close`、left の宛先、そのクライアントのメモリの解放。
- 根拠: [man 2 recv](https://man7.org/linux/man-pages/man2/recv.2.html)、[man 2 close](https://man7.org/linux/man-pages/man2/close.2.html)。

### 3.5 段階 5（40 分、Linux）

- 論点: 受け取ったバイトを `\n` で行に分けること。各行の前に `client %d: ` を付けること。送信者を宛先から外すこと。
- 配布 `main.c` の `extract_message` と `str_join` を使ってよい（付録 A）。使うときは、誰がどのポインタを `free` するかを先に決める。
- 根拠: [man 3 sprintf](https://man7.org/linux/man-pages/man3/sprintf.3.html)。

### 3.6 段階 6（35 分、Mac）

- 論点: `\n` の無い末尾を、次の `recv` まで残すこと。残った分を切断時に解放すること。
- 確かめ方: `nc` で `hel` を Ctrl+D で送り、続けて `lo` と Enter。macOS の `nc` で Ctrl+D が途中送信になるかは 3.0 で確かめておく。

### 3.7 段階 7（実装 60 分は Mac、確認 30 分は Linux）

- 論点: クライアントごとの送信待ち。書く側の集合に入れる fd。writable の周回での `send`。送れた分だけ送信待ちから消すこと。arrived、行、left のすべてを送信待ち経由にすること。
- 根拠: [man 2 send](https://man7.org/linux/man-pages/man2/send.2.html)、[man 2 select](https://man7.org/linux/man-pages/man2/select.2.html)。
- Mac では、`nc` 3 本での arrived、行、left までを確かめる。読まないクライアントは Mac でも試してよいが、結果は Linux の確認で判断する。

Linux での確認（10:20 から）:

1. 3 本の `nc` を繋ぎ、1 本を Ctrl+Z で止める。
2. 別の `nc` から大量の行を流す（`yes 'xxxxxxxx' | head -n 200000 | nc 127.0.0.1 ポート`）。
3. 止めていない 3 本目と、まだ会話できる。サーバーが止まった `nc` を切断しない。
4. 止めた `nc` を `fg` で戻すと、溜まっていた行が届く。
5. valgrind で、メモリと fd のリークが無いことを確かめる。

```bash
valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./mini_serv 8081
```

valgrind の確認では、`nc` を何本か繋いで切ってから、サーバーを Ctrl+C で止める。Ctrl+C で止めた時点で開いている fd とメモリは、課題文がどこまで求めているかがはっきりしない。報告が出たら、その内容を見て判断する。

### 3.8 通し練習（90 分、Linux）

- 新しいファイル（例: `rehearsal/mini_serv.c`）に、白紙から書く。自分の完成版、配布 `main.c`、このプランの付録は見ない。見てよいのは Linux の man だけ。
- 30 分ごとに、どの段階まで進んだかを記録する。
- 書き終えたら、3.7 の確認を同じ手順で行う。

### 3.9 振り返り（60 分、Linux）

- 通し練習で詰まった箇所を、付録 B のチェックリストに追記する。
- 詰まった段階だけを、もう一度白紙から書く。
- 最後の 10 分で、付録 B を見ずに言えるかを確かめる。

### 3.10 引き継ぎ（20:45–21:00、Linux）

1. その時点の `mini_serv.c` を、朝の Mac で開ける状態にする（例: `git add` / `git commit` / `git push`。実行はドライバー）。
2. どの段階のどこまで動くかを、セッションログに 3 行で残す（AI に依頼してよい）。
3. 朝の最初に貼る指示文（付録 C）の「今日やること」を、次の段階に書き換えておく。

---

## 4. 遅れたときの切り方

移動と睡眠の時刻は動かせない。遅れは次の段階の時間で吸収する。

| 時点 | 状態 | 切り方 |
|------|------|--------|
| 18:00 | 段階 2 が終わっていない | そのまま移動する。18:45 から Linux で段階 2 を終わらせ、段階 3 を 40 分に縮める |
| 20:45 | 段階 5 が終わっていない | 引き継ぎを 20:50 からの 10 分に縮める。段階 5 の残りは朝の最初に回し、段階 6 を 25 分に縮める |
| 09:35 | 段階 7 の実装が終わっていない | そのまま移動する。10:20 から Linux で続け、確認とあわせて 11:00 までに終わらせる |
| 11:00 | 段階 7 が動いていない | 11:30 までに打ち切る。通し練習は 11:30–12:30 の 60 分にして、振り返りを 12:30–13:30 にする |
| どこでも | 1 つの不具合に 20 分以上かかっている | AI に、症状とコードと `nc` の出力を貼って相談する |

通し練習は削らない。試験は白紙から 3 時間なので、白紙から書いた経験が 1 回あるかどうかが、試験中の時間の使い方を左右する。

---

## 付録 A: 配布 `main.c`

### 出所

[git.hexanyn.fr `42/exam/rank-06` の `subjects/mini_serv/main.c`](https://git.hexanyn.fr/42/exam/rank-06/-/blob/master/subjects/mini_serv/main.c)。42 の公式サイトではなく、非公式のミラーである。試験で配られる `main.c` と同じかは確かめられない。課題文は「このファイルは禁止関数を使い、最終プログラムに残してはならない記述を含む」としている。

### 本文（2026-10-12 取得、93 行）

```c
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}


int main() {
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli; 

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(8081); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n");
	if (listen(sockfd, 10) != 0) {
		printf("cannot listen\n"); 
		exit(0); 
	}
	len = sizeof(cli);
	connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
	if (connfd < 0) { 
        printf("server acccept failed...\n"); 
        exit(0); 
    } 
    else
        printf("server acccept the client...\n");
}
```

### 読み方（3.0 のあとの 10 分で確かめる）

使える部分:

| 部分 | 内容 |
|------|------|
| `bzero` と `sin_family` / `sin_addr` / `sin_port` | 段階 2 の土台。`htonl(2130706433)` は 127.0.0.1（`0x7F000001`） |
| `bind` / `listen` の失敗判定 | 段階 2 の土台 |
| `str_join(buf, add)` | `buf` の後ろに `add` を足した新しい文字列を返し、古い `buf` は `free` する。`buf` が NULL でもよい |
| `extract_message(&buf, &msg)` | `buf` に `\n` があれば、その行（`\n` を含む）を `msg` に渡して 1 を返す。`buf` は残りの新しい文字列に替わる。`\n` が無ければ 0。確保に失敗したら -1 |

`htonl` と `htons` は課題文の許可関数一覧に無い。配布物がこの 2 つを使っているので、試験でもこのまま使うものと判断する。

段階 2 までに変える箇所（変え方は段階のスケルトンで扱う）:

| 箇所 | 問題 |
|------|------|
| `#include` | `printf`、`sprintf`、`calloc`、`malloc`、`free`、`exit`、`atoi` の宣言が無い。`-Werror` で止まる |
| `printf(...)` と `exit(0)` | `printf` は許可関数に無い。課題文の文字列と終了ステータス 1 にならない |
| `htons(8081)` | ポートを引数から取っていない |
| `int len` を `accept` に渡す | `accept` の第 3 引数は `socklen_t *`。型が違うと警告になる |
| `accept` を 1 回だけ呼ぶ | `select` のループの中に移す |
| `main()` | 引数を受け取れない |

段階 5 と 6 で確かめること:

- `extract_message` が 1 を返したとき、`msg` を `free` するのは呼び出し側である。
- `\n` で終わる受信を全部取り出したあと、`buf` は空文字列の確保済み領域になる。NULL にはならない。
- `str_join` と `extract_message` は、確保に失敗すると NULL または -1 を返す。課題文では、メモリ確保の失敗は `Fatal error`。
- `str_join` の `add` は `\0` で終わる文字列でなければならない。`recv` の戻り値の位置に `\0` を置いてから渡す。

---

## 付録 B: チェックリスト（通し練習の前に見ずに言えるようにする）

文字列:

- [ ] `Wrong number of arguments\n`
- [ ] `Fatal error\n`
- [ ] `server: client %d just arrived\n`
- [ ] `client %d: `（コロンのあとに空白 1 つ）
- [ ] `server: client %d just left\n`

決まり:

- [ ] 標準エラーへの出力と終了ステータス 1
- [ ] 待ち受けは 127.0.0.1 だけ
- [ ] ID は fd ではなく、接続順に 0 から
- [ ] arrived は、今つながった本人には送らない
- [ ] 行は送信者以外へ送る
- [ ] `select` が readable と示した fd にだけ `accept` / `recv` を呼ぶ
- [ ] `select` が writable と示した fd にだけ `send` を呼ぶ
- [ ] 読まないクライアントを切断しない
- [ ] `#define` を書かない
- [ ] 切断したクライアントのメモリと fd を解放する
- [ ] メモリ確保の失敗は `Fatal error`

通し練習と振り返りで、自分が詰まった箇所をここに追記する。

---

## 付録 C: 新しいチャットに貼る指示文

```text
42 exam06 の mini_serv（C、mini_serv.c）の練習を進めています。
参加者: torinoue。AI は Navigator に徹し、完成コードは書かない（.cursor/rules/scaffolding-workflow.mdc）。

以下を読んで現在地を把握してから作業を始めてください:
- bircd/exam06_miniserv/mini_serv_study_plan.md（学習プランと時間割）
- bircd/exam06_miniserv/session_logs/ 内の最新セッションログ
- bircd/exam06_miniserv/mini_serv_ja.md（課題文の和訳）
- bircd/exam06_miniserv/mini_serv_data_flow.md（データフローと段階表）

注意:
- GitHub などの回答例のソースは読まない・引用しない。配布 main.c はプランの付録 A にある。
- 段階ごとに、穴埋めスケルトン → 私が vim で書く → 私が貼ったコードをレビュー、の順で進める。
- 時間割から遅れたら、プランの「4. 遅れたときの切り方」で判断する。

今日やること: [例: 3.1 段階 1（引数なしと Fatal error）]
```
