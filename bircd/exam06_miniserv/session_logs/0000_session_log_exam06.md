# セッションログ exam06 #0000

> 日付: 2026-10-12
> 開始時刻: 15:47 JST
> 終了時刻: 16:28 JST
> 実稼働時間: 1h
> セッション種別: exam06（mini_serv）準備 — 課題把握・参照実装の調査
> **参加者: torinoue**（Navigator: AI）

---

## セッション概要

**論点:**
- 42 exam06 の予想問題 `mini_serv` を、bircd の学習を活かして、試験中に段階的に書ける形で練習する。
- 実装の前に、課題文の意味と、完成品の挙動を把握する。

**結論（先取り）:**
- 提出は C の `mini_serv.c` 1 ファイル。`exam_gideline.txt` の「C++98」は誤りだった。
- 実装は 7 段階で積む。bircd から持っていくのは `fd_set`、fd を添字にしたクライアント表、`select` のループだけ。
- 完成品の挙動を `nc` で観察してから段階 1 に入る。観察用の完成品は GitHub の既存の回答例から選ぶ。ソースは学習者が読まず、バイナリとして使う。
- 1 本目の候補 Glagan 版は、完成品として使えない（PoC 記録）。2 本目の akadjoker 版は前半の確認は課題文どおりだが、読まないクライアントの確認で止まり、原因は未確認。
- 読まないクライアント、リークと fd の確認、仕上げは校舎の Linux で行う。

---

## 完了タスク

| # | 内容 | 成果物 |
|---|------|--------|
| 1 | 進め方の決定（言語 C、置き場、段階、bircd から捨てるもの） | このログの「決定事項」 |
| 2 | 課題文の和訳 | [mini_serv_ja.md](../mini_serv_ja.md) |
| 3 | データフロー図 | [mini_serv_data_flow.md](../mini_serv_data_flow.md) |
| 4 | `mini_db.hpp`（rank06 の別課題の回答例）の分析 | [mini_db_analysis.md](../miniserv_samples/mini_db_analysis.md) |
| 5 | 参照実装 2 本のブラックボックス確認（macOS） | このログの PoC 記録、AI 議事録 |

---

## 決定事項

| 項目 | 決定 |
|------|------|
| 言語 | C。提出は `mini_serv.c` |
| 実装の置き場 | `bircd/exam06_miniserv/mini_serv.c`。ドライバーが vim で書く |
| AI の役割 | Navigator。穴埋めスケルトンとレビュー。完成コードは書かない |
| エージェント | このチャット（ローカルの Agent）で継続 |
| ゴール | やりながら把握する |
| bircd から持っていくもの | `fd_set`、fd を添字にしたクライアント表、`select` のループ |
| bircd から持っていかないもの | 関数ポインタ、`#define`、固定長配列、libft、`poll`、`\r\n` 切り出し |
| 観察用の完成品 | GitHub の既存の回答例をリポジトリの外（`/tmp`）で取得・ビルドする。ソースは読まない |
| 実施形態 | 1 人。ターミナル 3〜4 枚 |
| 観察する挙動 | 接続順の ID と arrived / left、複数行、改行なしの送信、読まないクライアント、引数なしと使用中ポート |
| `nc` | 手元の macOS の `nc`。Linux との違いは注記する |
| ハンズオン資料の置き場 | `bircd/exam06_miniserv/mini_serv_nc_handson.md`（Linux の結果が出てから書く） |

段階（[データフロー §7](../mini_serv_data_flow.md#7-段階との対応)）:

1. 引数不足と、accept 前の `Fatal error`
2. `127.0.0.1` で `socket` / `bind` / `listen`
3. `select` + `accept` + ID + `just arrived`
4. `recv` が 0 以下で `just left` と fd を閉じる
5. `\n` ごとに `client %d: ` を付けて他クライアントへ配る
6. 1 回の `recv` で行が割れても、行末まで貯めてから配る
7. 送信しきれない分をクライアントごとに残し、遅い相手を切断しない

---

## PoC 記録

### PoC: Glagan 版を観察用の完成品にできるか

**目的:**
[Glagan/42-exam-rank-06](https://github.com/Glagan/42-exam-rank-06) の `mini_serv.c` が、課題文どおりに動くかを `nc` で確かめる。動けば、`nc` ハンズオンの完成品として使う。

候補に選んだ理由: 検索結果の要約で、送信待ちのキューを持つことが分かった。読まないクライアントを切らない挙動を観察するには、送信待ちが必要である。

**対象:**
- コミット `09b1ca5cf2183bdee9774d2f27b63c564dc46516`（2021-12-16）
- ライセンス GPL-3.0
- 取得先 `/tmp/mini_serv_ref`（リポジトリの外）

**手順:**

```bash
git clone --depth 1 https://github.com/Glagan/42-exam-rank-06.git /tmp/mini_serv_ref
cd /tmp/mini_serv_ref
gcc -Wall -Wextra -Werror mini_serv.c -o mini_serv
bash blackbox.sh 18081
```

落ちた原因を調べるため、AddressSanitizer 付きでもビルドして同じ確認を流した。

```bash
gcc -g -fsanitize=address mini_serv.c -o mini_serv_asan
BIN=/tmp/mini_serv_ref/mini_serv_asan bash blackbox.sh 18091
```

確認スクリプト `/tmp/mini_serv_ref/blackbox.sh`（全文。`/tmp` は再起動で消えるため、ここに残す）:

```bash
#!/bin/bash
# Black-box check of a mini_serv binary against the subject, using macOS nc.
BIN=${BIN:-/tmp/mini_serv_ref/mini_serv}
PORT=${1:-18081}
W=/tmp/mini_serv_ref/bb
rm -rf "$W"; mkdir -p "$W"; cd "$W" || exit 1

show() { printf '%s: ' "$1"; od -An -c "$1" | tr -s ' ' | head -c 400; echo; }

echo "== T1 no args"
"$BIN" >o1.txt 2>e1.txt; echo "exit=$?"; show e1.txt

echo "== T2 port in use"
nc -l 127.0.0.1 $((PORT+1)) >/dev/null 2>&1 & HOLD=$!
sleep 0.5
"$BIN" $((PORT+1)) 2>e2.txt; echo "exit=$?"; show e2.txt
kill $HOLD 2>/dev/null; wait $HOLD 2>/dev/null

echo "== start server on $PORT"
"$BIN" $PORT >srv_out.txt 2>srv_err.txt & SRV=$!
sleep 0.5

mkfifo in0 in1 in2 in3
nc 127.0.0.1 $PORT <in0 >out0 & N0=$!
exec 3>in0
sleep 0.5
echo "-- after c0 connect"; show out0

nc 127.0.0.1 $PORT <in1 >out1 & N1=$!
exec 4>in1
sleep 0.5
echo "-- after c1 connect"; show out0; show out1

echo "== T3 multiline from c0"
printf 'hello\nworld\n' >&3
sleep 0.5
show out0; show out1

echo "== T4 split from c0"
: >out1.mark; cp out1 out1.before
printf 'hel' >&3
sleep 1
cmp -s out1 out1.before && echo "after 'hel': out1 unchanged" || { echo "after 'hel': out1 CHANGED"; show out1; }
printf 'lo\n' >&3
sleep 0.5
show out1

echo "== T5 lazy client"
nc 127.0.0.1 $PORT <in2 >out2 & N2=$!
exec 5>in2
sleep 0.5
echo "-- c2 arrived:"; show out0; tail -c 40 out1 | od -An -c | tr -s ' '
kill -STOP $N1
echo "c1 (nc pid $N1) stopped"
LINES=200000
yes 'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx' | head -n $LINES >&3
sleep 3
echo "c2 flood lines received: $(grep -c '^client 0: x' out2) / $LINES"
printf 'c2 still talking\n' >&5
sleep 0.5
echo "c0 got from c2:"; tail -n 1 out0
kill -0 $SRV 2>/dev/null && echo "server alive" || echo "server DEAD"
echo "c1 flood lines so far (stopped): $(grep -c '^client 0: x' out1)"
kill -CONT $N1
sleep 3
echo "c1 flood lines after resume: $(grep -c '^client 0: x' out1) / $LINES"
echo "c1 last line:"; tail -n 1 out1

echo "== T6 left and next id"
kill $N0; wait $N0 2>/dev/null; exec 3>&-
sleep 0.5
echo "c1 tail:"; tail -n 1 out1
echo "c2 tail:"; tail -n 1 out2
nc 127.0.0.1 $PORT <in3 >out3 & N3=$!
exec 6>in3
sleep 0.5
echo "after c3 connect, c1 tail:"; tail -n 1 out1

echo "== T7 stdin EOF on macOS nc"
printf 'eof-test\n' | nc 127.0.0.1 $PORT >out4 & N4=$!
sleep 1
kill -0 $N4 2>/dev/null && echo "nc after stdin EOF: still running" || echo "nc after stdin EOF: exited"
echo "c1 last 2 lines:"; tail -n 2 out1
kill $N4 2>/dev/null; wait $N4 2>/dev/null
sleep 0.5
echo "c1 last line after killing it:"; tail -n 1 out1

echo "== cleanup"
kill $N1 $N2 $N3 2>/dev/null
exec 4>&- 5>&- 6>&-
kill $SRV 2>/dev/null; wait 2>/dev/null
echo "server stdout bytes: $(wc -c <srv_out.txt) stderr bytes: $(wc -c <srv_err.txt)"
```

スクリプトの作り:
- 各クライアントは `nc` で、入力を FIFO（`in0`〜`in3`）から受け、受信をファイル（`out0`〜`out3`）に書く。
- 読まないクライアントは、その `nc` を `kill -STOP` で止めて再現する。
- `show` は `od -c` で改行を `\n` として見せる。`tr -s ' '` で空白を詰めているので、表示上は単語の間の空白が消える。

**結果:**

| 確認 | 課題文の要求 | Glagan 版 |
|------|--------------|-----------|
| T1 引数なし | 標準エラーに `Wrong number of arguments\n`、終了 1 | 終了 1。標準エラーは `Wrong number of arguments !\n`（`!` の前に空白と `!` が余計） |
| T2 使用中ポート | 標準エラーに `Fatal error\n`、終了 1 | 一致 |
| 接続通知 | 既存クライアントへ `server: client 1 just arrived\n` | 一致。最初のクライアントには何も届かない |
| T3 複数行 | `client 0: hello\n` と `client 0: world\n`、送信者には届かない | 一致 |
| T4 改行なし | `hel` は配らない。`lo\n` で `client 0: hello\n` | 一致 |
| T5 読まないクライアント | 切断しない。他のクライアントとの会話は続く | 大量の行を流した途中でサーバーが `Abort trap: 6` で終了 |
| T6 left と次の ID | — | 未実行（T5 でサーバーが落ち、スクリプトも終了ステータス 141 で終わった） |
| T7 macOS `nc` の標準入力 EOF | — | 未実行 |

ソースの該当箇所（原因の確認にだけ参照）:

```text
$ rg -n '^#define' mini_serv.c
11:#define BUFFER_SIZE 65535

$ rg -n 'Wrong' mini_serv.c
147:		write(STDERR_FILENO, "Wrong number of arguments !\n", 28);
```

AddressSanitizer の出力（抜粋）:

```text
==88028==ERROR: AddressSanitizer: heap-use-after-free on address 0x000107500610
READ of size 2 at 0x000107500610 thread T0
    #0 ... in wrap_strlen
    #1 ... in str_join mini_serv.c:59
    #2 ... in main mini_serv.c:273

freed by thread T0 here:
    #0 ... in wrap_free
    #1 ... in str_join mini_serv.c:69
    #2 ... in main mini_serv.c:273

previously allocated by thread T0 here:
    #0 ... in wrap_calloc
    #1 ... in str_join mini_serv.c:62
    #2 ... in main mini_serv.c:267
```

**解説:**
- `Abort trap: 6` は macOS 固有の問題ではない。`str_join` が、解放済みのメモリを `strlen` で読んでいる（heap-use-after-free）。未定義動作なので、Linux でも落ちる、または壊れた出力になる可能性がある。
- `#define BUFFER_SIZE` は、課題文の `Your program must not contains #define preproc` に反する。
- 引数なしの出力は、課題文の文字列と一致しない。
- 同じ AddressSanitizer 付きビルドで、`sprintf` の非推奨警告が 3 件出た（macOS の SDK による警告）。`-Wall -Wextra -Werror` の通常ビルドは、出力なしで成功した。

**判定:** 不採用。観察用の完成品として使えない。

---

## AI 議事録

### 1. 課題の把握

- 課題文は C の `mini_serv.c`。許可関数は 22 個で、`fcntl`、`signal`、`printf`、`inet_addr`、`htons`、`htonl` は含まれない。`htons` と `htonl` の採点上の扱いは、今回の資料からは判断できない。
- 課題文の要点と、課題文に無いこと（引数 2 つ以上、ポートが数値でない、改行なしで切断したときの受信残り、処理順、`send` の負の戻り値）を [mini_serv_ja.md](../mini_serv_ja.md) にまとめた。
- 送信は、`select` が writable と示した fd に対してだけ行う。到着通知、会話、切断通知は、最終形では全てクライアントごとの送信待ちを通す（[mini_serv_data_flow.md](../mini_serv_data_flow.md)）。

### 2. terminal-42s/42_examshell の rank06

- ドライバーが [mini_db.hpp](https://github.com/terminal-42s/42_examshell/blob/main/.resources/rank06/mini_db/mini_db.hpp) を見つけた。これは mini_serv ではなく、rank06 の別課題 `mini_db`（C++ のキー・バリューストア）の回答例だった。
- 同じリポジトリの rank06 には `mini_db/`（`sub.txt`、`mini_db.cpp`、`mini_db.hpp`、`tester.sh`）と `mini_serv/`（`sub.txt`、`mini_serv.c`、`tester.sh`）がある。
- `mini_serv/mini_serv.c` は、`ready` を出してすぐ `close` するスタブで、配布される `main.c` でも回答例でもない。
- 両方の `tester.sh` は、失敗があっても最後に `PASSED` を出して `exit 0` する。このリポジトリの [tester_serv.sh](../tester_serv.sh) は `mini_serv/tester.sh` と同じ内容。
- 分析は [mini_db_analysis.md](../miniserv_samples/mini_db_analysis.md)。和訳とデータフローは手元の `mini_serv.txt` だけから作り、この rank06 のファイルは根拠にしていない。

### 3. 参照実装の確認

- Glagan 版は不採用（PoC 記録）。
- 2 本目は [akadjoker/mini_serv](https://github.com/akadjoker/mini_serv) の `array/mini_serv.c`（コミット `d575e98`、2023-12-31。README に「2023-09-22 にこのコードで合格」とある）。`/tmp/mini_serv_ref2` に取得した。
  - ソースに `#define` は無い（`rg -c '^#define'` の出力なし）。`-Wall -Wextra -Werror` でビルド成功。
  - AddressSanitizer 付きビルドで同じ確認スクリプトを流した。T1〜T4 と接続通知は課題文どおりだった。T1 の標準エラーは `Wrong number of arguments\n`。
  - T5 で、`c1` の `nc` を止めて 200000 行を流す手順のまま止まった。サーバーの標準エラーは空で、AddressSanitizer の報告は無い。サーバーが止まった `c1` への `send` で詰まった可能性が高いが、未確認。
  - 止まったスクリプトと、テスト用の `nc` とサーバーは、AI が `pkill` で止めた。
- 確認スクリプトの全文は PoC 記録に残した。

### 4. Mac か Linux か

- 出力の文字列と終了ステータス、行の中継は、この Mac で確認できる。
- 次の 4 点は校舎の Linux で行う。
  1. 読まないクライアント。どの時点で `send` が詰まるかは送信バッファの大きさで変わり、その大きさは macOS と Linux で違う。採点のテスターは Linux で動く。
  2. リークと fd のリーク。Linux なら `valgrind --leak-check=full --track-fds=yes`。Apple Silicon の macOS では valgrind が使えない。
  3. man。試験中に読めるのは Linux の man。
  4. `nc` の種類。手元は macOS の `/usr/bin/nc`（BSD 系。オプションに `-N` が無い）。
- 校舎の Linux が使えない日は、Mac の Docker で Linux コンテナを使う。

---

## 次回やること

1. 校舎の Linux で akadjoker 版を取得・ビルドし、今日の確認スクリプトを再実行する。
2. 読まないクライアントの確認: 1 本の `nc` を Ctrl+Z で止め、別の `nc` から大量の行を流し、3 本目が会話を続けられるかを見る。
3. 結果をもとに、akadjoker 版を観察用の完成品にするかを決める。
4. `bircd/exam06_miniserv/mini_serv_nc_handson.md` を作る。
5. 段階 1（引数なしと `Fatal error`）を、ドライバーが `mini_serv.c` に書く。

---

## 関連資料

- [mini_serv.txt](../mini_serv.txt)（課題文の原文）
- [exam_gideline.txt](../exam_gideline.txt)（予想される試験の決まり）
- [mini_serv_ja.md](../mini_serv_ja.md)
- [mini_serv_data_flow.md](../mini_serv_data_flow.md)
- [mini_db_analysis.md](../miniserv_samples/mini_db_analysis.md)
- [bircd 分析](../../bircd_analysis.md)
- [bircd データフロー図](../../bircd_data_flow_diagram.md)
- [irssi ハンズオン](../../../dev_docs/onboarding_docs/irssi_handson_common.md)（ft_irc のオンボーディングで使った資料。今回の `nc` ハンズオンの手本）
- [Glagan/42-exam-rank-06](https://github.com/Glagan/42-exam-rank-06)
- [akadjoker/mini_serv](https://github.com/akadjoker/mini_serv)
- [man 2 select](https://man7.org/linux/man-pages/man2/select.2.html)
- [man 2 send](https://man7.org/linux/man-pages/man2/send.2.html)
- [man 2 recv](https://man7.org/linux/man-pages/man2/recv.2.html)

---

## 後日訂正

（なし）
