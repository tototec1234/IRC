# mini_serv 試験中の man 参照先

> 作成日: 2026-10-13 18:24 JST
> 追記: 2026-10-13 18:35 JST（7 節「Mac での練習環境（Linux 版 man の持ち帰り）」を追加）
> 目的: 試験中に `FD_ZERO` などのマクロや `struct sockaddr_in` のメンバ名を思い出せなかったときに、最短で引ける man を決めておく
> 調査環境: Ubuntu（`uname -r` = 6.8.0-138-generic）、`manpages-dev` 5.10-1ubuntu1
> 前提: [学習プラン](mini_serv_study_plan.md)、[課題文 和訳](mini_serv_ja.md)
> 注意: 本文の記載は上の調査環境で実際に `man` を読んで確認した内容。校舎の Linux では「5. 試験前の確認」を先に実行する

---

## 1. 結論

本命は **`man 2 select_tut`**。exam06 とほぼ同じ構造の、`main()` 付きの完全なプログラム（TCP フォワーダ、約 200 行）が EXAMPLES に載っている。

`man 2 select` の SEE ALSO 末尾に、次の 1 文で案内されている。

```
For a tutorial with discussion and examples, see select_tut(2).
```

`select(2)` だけを読んで引き返すと、この例に到達できない。試験中に `select(2)` の EXAMPLES（stdin を 5 秒監視するだけの短い例）で満足してしまうのが、今回の失敗の構造。

---

## 2. 用途別の対応表

| 思い出せないもの | 引く man | 場所 |
|------|------|------|
| `FD_ZERO` / `FD_SET` / `FD_CLR` / `FD_ISSET` の引数と戻り値 | `man 2 select` | SYNOPSIS に 4 つの宣言 |
| 上記マクロの意味、「集合は毎周作り直す」の根拠 | `man 2 select` | DESCRIPTION の `File descriptor sets` 節 |
| select ループ全体の骨格、読む集合と書く集合の併用、部分送信の積み残し | `man 2 select_tut` | EXAMPLES |
| `nfds` の意味、select の作法と落とし穴 | `man 2 select_tut` | DESCRIPTION の `Practical` 節と `Select law` 節 |
| `struct sockaddr_in` / `struct in_addr` のメンバ名 | `man 7 ip` | `Address format` 節 |
| `INADDR_ANY` / `INADDR_LOOPBACK` の値と、`htonl` / `htons` の使い分け | `man 7 ip` | `Address format` 節の直後 |
| `bind` → `listen` → `accept` をエラー処理付きで書く形（`main()` 入り） | `man 2 bind` | EXAMPLES |
| 最小の select 動作確認コード | `man 2 select` | EXAMPLES |

### コード例が載っていない man（試験中に開く価値が低い）

| man | EXAMPLES の中身 |
|------|------|
| `man 2 socket` | 「An example of the use of socket() is shown in getaddrinfo(3).」のみ |
| `man 2 bind` の冒頭 | 「An example of the use of bind() with Internet domain sockets can be found in getaddrinfo(3).」（ただし後続に AF_UNIX の完全な例がある。2.1 参照） |
| `man 2 listen` | 「See bind(2).」のみ |
| `man 2 accept` | 「See bind(2).」のみ |
| `man 2 send` | 「An example of the use of sendto() is shown in getaddrinfo(3).」のみ |
| `man 2 recv` | 「An example of the use of recvfrom() is shown in getaddrinfo(3).」のみ |
| `man 2 close` / `man 2 write` / `man 3 malloc` / `man 3 memset` / `man 3 strstr` / `man 3 atoi` / `man 7 socket` / `man 7 tcp` | EXAMPLES 節そのものが無い |

`man 3 getaddrinfo` にはクライアントとサーバ両方の `main()` があるが、`getaddrinfo` 自体が exam06 の許可関数外なので、socket 準備の形を見る以外の用途は無い。

### 2.1 `man 2 bind` の EXAMPLES について

AF_UNIX（`struct sockaddr_un`）の例であり、`sockaddr_in` の書き方は得られない。得られるのは次の 3 点。

- `(struct sockaddr *)` へのキャストの書き方
- `accept` の第 3 引数が `socklen_t` 変数のアドレスであること
- `socket` → `bind` → `listen` → `accept` の各戻り値を 1 か所の出口（`handle_error` マクロ）へ送る形。`mini_serv.c` の `fatal()` と同じ考え方

### 2.2 `man 2 select_tut` の EXAMPLES に含まれるもの

- `socket` → `setsockopt(SOL_SOCKET, SO_REUSEADDR)` → `bind`（`struct sockaddr_in` と `htons` 付き）→ `listen` を関数に切り出した形
- `main()` の `for (;;)` 内で `FD_ZERO` を 3 つの集合に対して呼び、`FD_SET` で登録し、`select(nfds + 1, &readfds, &writefds, &exceptfds, NULL)` を呼び、`FD_ISSET` で分岐する流れ
- **送信待ちデータがあるときだけ writefds に入れる**書き方。`mini_serv.c` の `g_wbuf[fd] != NULL` の判定と同じ
- `write` が要求バイト数を書き切らなかったときの `*_avail` / `*_written` による積み残し管理。`mini_serv.c` の `flush_client()` と同じ目的
- `select` が `-1` で `errno == EINTR` のときは `continue` する分岐
- `accept` の第 3 引数に `socklen_t addrlen` のアドレスを渡す形

---

## 3. 試験中の引き方

```
man FD_ZERO          # select(2) へ飛ぶ。FD_SET / FD_ISSET / FD_CLR でも同じ
man 2 select_tut     # コード例の本体。less 内で /EXAMPLES と打って飛ぶ
man 7 ip             # less 内で /Address format と打って飛ぶ
```

`man FD_ZERO` が効く理由は、`/usr/share/man/man3/FD_ZERO.3.gz` が `../man2/select.2.gz` へのシンボリックリンクになっているため。`FD_SET` / `FD_ISSET` / `FD_CLR` も同じ。マクロ名をそのまま渡せるので、セクション番号を思い出す必要が無い。

目次を先に見たいときは次のとおり。

```
man 2 select_tut | col -b | grep -nE '^[A-Z][A-Z ]+$|^   [A-Z]'
```

該当名が思い出せないときは `man -k` で探す。

```
man -k select | grep '(2)'
```

---

## 4. man が引けないときの保険

ヘッダは、コンパイルできる環境なら必ず存在する。man より確実。

```
grep -n 'FD_SET\|FD_ZERO\|FD_ISSET\|FD_CLR' /usr/include/x86_64-linux-gnu/sys/select.h
grep -n -A12 'struct sockaddr_in$' /usr/include/netinet/in.h
grep -n 'INADDR_ANY\|INADDR_LOOPBACK' /usr/include/netinet/in.h
```

調査環境での結果。

| 探すもの | 場所 | 内容 |
|------|------|------|
| `FD_SETSIZE`、`FD_SET`、`FD_CLR`、`FD_ISSET`、`FD_ZERO` | `/usr/include/x86_64-linux-gnu/sys/select.h` 73〜88 行 | `__FD_SET` などへの `#define` |
| `struct sockaddr_in` | `/usr/include/netinet/in.h` 245 行 | `__SOCKADDR_COMMON (sin_)` / `sin_port` / `sin_addr` / `sin_zero` |
| `INADDR_ANY` | `/usr/include/netinet/in.h` 194 行 | `((in_addr_t) 0x00000000)` |
| `INADDR_LOOPBACK` | `/usr/include/netinet/in.h` 207 行 | `((in_addr_t) 0x7f000001)` |

パスの `x86_64-linux-gnu` は環境依存。見つからないときは次で探す。

```
echo '#include <sys/select.h>' | cc -E -H -xc - -o /dev/null 2>&1 | grep select.h
```

---

## 5. 試験前の確認（校舎の Linux で 1 回）

セクション 2 と 3 の man は Debian 系では `manpages-dev` パッケージ由来で、入っていないマシンでは `man 2 select` ごと引けない。

```
man 2 select     >/dev/null && echo "select OK"
man 2 select_tut >/dev/null && echo "select_tut OK"
man 7 ip         >/dev/null && echo "ip OK"
```

`select_tut OK` が出ない場合は、4 節のヘッダ grep が唯一の手段になる。その前提で、`FD_*` の 4 つと `sockaddr_in` の 3 メンバは暗記側に回す。

---

## 6. 今回のコードへ反映できる点

### 6.1 `htonl(2130706433)` を `htonl(INADDR_LOOPBACK)` にする

現在の `mini_serv.c` は、待ち受けアドレスを 10 進数のリテラルで書き、`bc` で値を出した経緯をコメントに残している。

```c
servaddr.sin_addr.s_addr = htonl(2130706433);// bcコマンドで 1+2^(31-7)*127で値を出す
```

`INADDR_LOOPBACK` は `netinet/in.h` で `0x7f000001` と定義済みで、`ip(7)` に「in_addr should be assigned one of the INADDR_* values (e.g., INADDR_LOOPBACK) using htonl(3)」と、`htonl` を通す指示まで書かれている。マクロ名で書けば、試験中に値を計算する手間が消える。

- 根拠: [man 7 ip](https://man7.org/linux/man-pages/man7/ip.7.html) の Address format 節

### 6.2 配布 `main.c` の `accept` 第 3 引数

配布 `main.c` は `len` を `int` で宣言し、そのアドレスを `accept` に渡している。`accept(2)` の SYNOPSIS は `socklen_t *addrlen` なので、`-Wall -Wextra -Werror` では非互換ポインタでエラーになる。

`mini_serv.c` は `accept(g_sockfd, NULL, NULL)` にしているため影響しないが、試験で配布 `main.c` から `accept` 行をコピーすると踏む。クライアントのアドレスが不要なら、第 2 引数と第 3 引数は `NULL` でよい（`accept(2)` の DESCRIPTION に「When addr is NULL, nothing is filled in; in this case, addrlen is not used, and should also be NULL.」と明記あり）。

- 根拠: [man 2 accept](https://man7.org/linux/man-pages/man2/accept.2.html)

---

## 7. Mac での練習環境（Linux 版 man の持ち帰り）

帰宅後は MacBook での学習になるが、macOS の man は BSD 由来で **`select_tut(2)` が存在しない**。`select(2)` も別文書で EXAMPLES が無い。さらに macOS の `struct sockaddr_in` は先頭に `sin_len` があるため、4 節のヘッダ grep の結果も校舎と一致しない。よって Linux 版を持ち帰る。

### 7.1 方針（2026-10-13 決定）

| 項目 | 決定 |
|------|------|
| 入手方法 | kernel.org の `man-pages-5.10.tar.gz` を本線、校舎 `/usr/share/man` からのコピーを保険。2 つを突き合わせて一致を確認する |
| 形式 | roff ソースのまま。`MANPATH` を通して `man 2 select_tut` が本番と同じ打鍵で引ける形にする |
| 範囲 | 最小セット 11 ページ + `FD_*` の 4 リンク（約 212KB） |
| ヘッダ | 持ち帰る（glibc のヘッダは kernel.org には無いので、校舎からのコピー一択） |
| 置き場所 | `exam06_miniserv/linux_ref/` に置いてコミットする |

roff ソースのまま置くことは、ライセンス上も都合が良い。`select_tut.2` の冒頭には次の表示が含まれており、roff なら削らずに保持される。

```
.\" This manpage is copyright (C) 2001 Paul Sheer.
.\" %%%LICENSE_START(VERBATIM)
.\" Permission is granted to make and distribute verbatim copies of this
.\" manual provided the copyright notice and this permission notice are
.\" preserved on all copies.
```

整形済みテキストに変換すると、この表示が落ちる。

### 7.2 ディレクトリ構成

```
bircd/exam06_miniserv/linux_ref/
├── PROVENANCE.txt          出所・バージョン・取得日
├── env.sh                  MANPATH を通す（source して使う）
├── man/
│   ├── man2/               select.2, select_tut.2, bind.2, accept.2,
│   │                       socket.2, listen.2, send.2, recv.2, close.2, write.2
│   ├── man3/               FD_ZERO.3, FD_SET.3, FD_ISSET.3, FD_CLR.3（../man2/select.2 への symlink）
│   └── man7/               ip.7
└── include/
    ├── sys/select.h
    ├── bits/select.h
    └── netinet/in.h
```

`man3/FD_*.3` を symlink にする理由は、3 節の `man FD_ZERO` を Mac でも成立させるため。校舎の `/usr/share/man/man3/FD_ZERO.3.gz` も `../man2/select.2.gz` への symlink になっている。

### 7.3 校舎の Linux で実行する手順

`/usr/share/man` のファイルは gzip 圧縮されているが、**非圧縮で置く**。macOS の man が `.gz` を扱えるか環境依存のため、確実な側に寄せる。非圧縮 11 ページで 212KB なのでリポジトリの負担にならない。

```sh
cd ~/IRC/bircd/exam06_miniserv
mkdir -p linux_ref/man/man2 linux_ref/man/man3 linux_ref/man/man7
mkdir -p linux_ref/include/sys linux_ref/include/bits linux_ref/include/netinet

for p in select select_tut bind accept socket listen send recv close write; do
	zcat /usr/share/man/man2/$p.2.gz > linux_ref/man/man2/$p.2
done
zcat /usr/share/man/man7/ip.7.gz > linux_ref/man/man7/ip.7

for m in FD_ZERO FD_SET FD_ISSET FD_CLR; do
	ln -sf ../man2/select.2 linux_ref/man/man3/$m.3
done

cp /usr/include/x86_64-linux-gnu/sys/select.h   linux_ref/include/sys/
cp /usr/include/x86_64-linux-gnu/bits/select.h  linux_ref/include/bits/
cp /usr/include/netinet/in.h                    linux_ref/include/netinet/
```

ヘッダのパスは環境依存。見つからないときは次で実際の位置を出す。

```sh
echo '#include <sys/select.h>' | cc -E -H -xc - -o /dev/null 2>&1 | grep select.h
```

出所を記録する。`.TH` 行に日付が入っているので、パッケージ情報が取れなくてもバージョンの手掛かりになる。

```sh
{
	echo "取得日: $(date -Iseconds)"
	echo "取得元: 校舎の Linux の /usr/share/man と /usr/include"
	echo "uname: $(uname -srm)"
	echo "manpages-dev: $(dpkg-query -W -f='${Version}' manpages-dev 2>/dev/null)"
	echo "--- .TH 行 ---"
	grep -m1 '^\.TH' linux_ref/man/man2/*.2 linux_ref/man/man7/ip.7
	echo "--- sha256 ---"
	sha256sum linux_ref/man/man2/*.2 linux_ref/man/man7/ip.7
} > linux_ref/PROVENANCE.txt
```

調査環境で確認した `.TH` 行は次のとおり。校舎で同じ日付が出れば同一バージョン。

```
.TH SELECT     2 2020-11-01 "Linux" "Linux Programmer's Manual"
.TH SELECT_TUT 2 2020-04-11 "Linux" "Linux Programmer's Manual"
.TH IP         7 2020-11-01 "Linux" "Linux Programmer's Manual"
```

### 7.4 家の Mac で実行する手順（kernel.org 本線）

校舎の `manpages-dev` は 5.10 なので、upstream も 5.10 を取る。URL の存在は確認済み（HTTP 200）。

```sh
cd /tmp
curl -LO https://mirrors.edge.kernel.org/pub/linux/docs/man-pages/Archive/man-pages-5.10.tar.gz
tar xzf man-pages-5.10.tar.gz
```

校舎から持ち帰ったものと一致するかを突き合わせる。

```sh
cd ~/IRC/bircd/exam06_miniserv
for p in select select_tut bind accept socket listen send recv close write; do
	diff -q /tmp/man-pages-5.10/man2/$p.2 linux_ref/man/man2/$p.2
done
diff -q /tmp/man-pages-5.10/man7/ip.7 linux_ref/man/man7/ip.7
```

Debian は upstream にパッチを当てることがあるので、差分が出ても異常ではない。差分が出た場合は **校舎のコピーを採用する**（試験で引くのは校舎のマシンだから）。差分の内容は `PROVENANCE.txt` に書き足す。

tarball には `man3/FD_ZERO.3` が含まれない（Debian の symlink は `make install` が作る）。7.3 の `ln -sf` は tarball から構成する場合も必要。

### 7.5 MANPATH の通し方

`linux_ref/env.sh` を次の内容で作る。zsh で `source` したときに自身の位置から解決する（`${0:A:h}` の挙動は検証済み）。

```sh
# usage: source linux_ref/env.sh
EXAM06_REF="${EXAM06_REF:-${0:A:h}}"
export MANPATH="$EXAM06_REF/man:$(MANPATH= /usr/bin/manpath)"
```

常時有効にはしない。練習用のターミナルタブで `source` する運用にする。先頭に置くので、有効な間は `man 2 select` も Linux 版が出る。macOS 版を見たいときは次のとおり。

```sh
man -M "$(MANPATH= /usr/bin/manpath)" 2 select
```

`source` を使わずその場で引くこともできる。

```sh
man -M ~/IRC/bircd/exam06_miniserv/linux_ref/man 2 select_tut
```

### 7.6 Mac での動作確認

```sh
source ~/IRC/bircd/exam06_miniserv/linux_ref/env.sh
man 2 select_tut | head -1      # SELECT_TUT(2) ... Linux Programmer's Manual が出る
man FD_ZERO      | head -1      # SELECT(2) ... が出る（symlink 経由）
man 7 ip         | head -1      # IP(7) ... が出る
```

コードが欠けずに整形されているかの目安。調査環境（Linux, `MANWIDTH=80`）での基準値を併記する。

```sh
MANWIDTH=80 man 2 select_tut | col -b | grep -c 'FD_SET'                      # 基準値 7
MANWIDTH=80 man 2 select_tut | col -b | sed -n '/^EXAMPLES/,/^SEE ALSO/p' | wc -l   # 基準値 285
MANWIDTH=80 man 7 ip         | col -b | grep -c 'sin_'                        # 基準値 8
```

`grep -c 'FD_SET'` が 7 を大きく下回る場合は、mandoc がコード例を落としている。その場合は roff を直接読む（`less linux_ref/man/man2/select_tut.2`）か、校舎で整形済みテキストも作って持ち帰る。

macOS の man は groff ではなく mandoc で整形するため、**見た目が校舎と完全に一致しない**。`.EX` / `.EE`（コード例の囲み）や `.TP` の字下げが変わる可能性がある。EXAMPLES のコードが読めていれば練習の目的は満たすが、1 回目は校舎の表示と見比べて、欠落していないかを確認する。

`man -k` / `apropos` は Mac 側にこのツリーのデータベースが無いので使えない。代わりに grep する。

```sh
grep -rln 'FD_ISSET' ~/IRC/bircd/exam06_miniserv/linux_ref/man
```

### 7.7 ヘッダ grep の練習

4 節の grep を、持ち帰ったヘッダに対して行う。

```sh
R=~/IRC/bircd/exam06_miniserv/linux_ref/include
grep -n 'FD_SET\|FD_ZERO\|FD_ISSET\|FD_CLR' $R/sys/select.h
grep -n -A12 'struct sockaddr_in$' $R/netinet/in.h
grep -n 'INADDR_ANY\|INADDR_LOOPBACK' $R/netinet/in.h
```

このヘッダは **読むためだけ**のもの。Mac でのコンパイルには使わない（`-I` で指定すると glibc と macOS の libc が混ざって壊れる）。

### 7.8 git（実行はドライバー）

```sh
cd ~/IRC
git add bircd/exam06_miniserv/linux_ref bircd/exam06_miniserv/mini_serv_man_reference.md
git commit -m "docs(exam06): add man reference and Linux man/header snapshot for Mac practice"
git push
```

symlink をコミットするので、`git config core.symlinks` が false になっていないことを確認する。念のため次で確認できる。

```sh
git config --get core.symlinks   # 何も出なければ既定（true 相当）で問題ない
```
