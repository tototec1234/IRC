# セッションログ exam06 #0003

> 日付: 2026-10-13
> 開始時刻: 18:18 JST
> 終了時刻: （未記入）
> 実稼働時間: （未記入）
> セッション種別: exam06（mini_serv）試験後の振り返り — man 参照先の調査と、Mac 練習環境の整備
> **参加者: torinoue**（Navigator: AI）
> 場所: 校舎の Linux（c1r4s2）
> 試験そのものの結果: （未記入。このログの対象外）

---

## セッション概要

**論点:**

- 試験中に `FD_ZERO` などのマクロを思い出せなかった。man を参照したが、`main` のあるプログラムや関数の実装例に辿り着けなかった。
- コード自体は問題なく書けた（[セッションログ #0002](0002_session_log_exam06.md) の到達点どおり）。欠けていたのは「引ける man を知っていること」である。

**結論（先取り）:**

- 本命は `man 2 select_tut`。exam06 とほぼ同じ構造の、`main()` 付きの完全なプログラムが EXAMPLES に載っている。
- `select(2)` の SEE ALSO 末尾の 1 文（`For a tutorial with discussion and examples, see select_tut(2).`）だけが入口で、`select(2)` の EXAMPLES（stdin を 5 秒監視する短い例）で引き返すと到達できない。試験中の失敗はこの構造によるもの。
- 成果物は [man 参照先](../mini_serv_man_reference.md)。帰宅後の Mac 練習用に、Linux 版 man とヘッダを `linux_ref/` へ持ち帰ってコミットした。

---

## 決定事項

| 項目 | 決定 |
|------|------|
| 調査の成果物 | `mini_serv_man_reference.md` として `exam06_miniserv` 直下に置く（既存の `mini_serv_study_plan.md` などと同じ階層・命名） |
| Linux man の入手方法 | kernel.org の `man-pages-5.10.tar.gz` を本線、校舎 `/usr/share/man` からのコピーを保険。2 つを突き合わせ、差分が出たら**校舎のコピーを採用する**（試験で引くのは校舎のマシンだから） |
| 保存形式 | roff ソースのまま。`MANPATH` を通して `man 2 select_tut` が本番と同じ打鍵で引ける形にする |
| 範囲 | 最小セット 11 ページ + `FD_*` の 4 リンク（約 212KB） |
| ヘッダ | 持ち帰る。glibc のヘッダは kernel.org には無いので校舎からのコピー一択 |
| 置き場所 | `exam06_miniserv/linux_ref/` に置いてコミットする |
| 圧縮 | 非圧縮で置く。macOS の man が `.gz` を扱えるかが環境依存のため、確実な側に寄せる |

roff ソースのまま置く判断は、ライセンス上も都合が良い。`select_tut.2` 冒頭の VERBATIM ライセンス表示（`Permission is granted to make and distribute verbatim copies of this manual provided the copyright notice and this permission notice are preserved on all copies.`）が、整形済みテキストに変換すると落ちる。

---

## Spike 記録

### Spike 1: 各 man の EXAMPLES 節に実際にコードがあるか

```sh
for p in "2 select" "2 select_tut" "2 socket" "2 bind" "2 listen" "2 accept" "2 send" "2 recv" "7 ip"; do
	out=$(MANWIDTH=80 man ${=p} 2>/dev/null | col -b)
	printf '%-16s : lines=%-5s main=%-3s EXAMPLE_sec=%s\n' "$p" \
		"$(printf '%s' "$out" | wc -l | tr -d ' ')" \
		"$(printf '%s' "$out" | grep -cE 'main\(')" \
		"$(printf '%s' "$out" | grep -cE '^EXAMPLE')"
done
```

結果（抜粋）:

```
2 select         : lines=415   main=1   EXAMPLE_sec=1
2 select_tut     : lines=482   main=2   EXAMPLE_sec=1
2 socket         : lines=229   main=0   EXAMPLE_sec=1
2 bind           : lines=178   main=1   EXAMPLE_sec=1
2 listen         : lines=98    main=0   EXAMPLE_sec=1
2 accept         : lines=192   main=0   EXAMPLE_sec=1
2 send           : lines=262   main=0   EXAMPLE_sec=1
2 recv           : lines=330   main=0   EXAMPLE_sec=1
```

解説: **`EXAMPLE_sec=1` でも `main=0` の man は、節があるだけで中身が参照の 1 文しかない。** `socket(2)` は「An example of the use of socket() is shown in getaddrinfo(3).」、`listen(2)` と `accept(2)` は「See bind(2).」、`send(2)` と `recv(2)` は「shown in getaddrinfo(3)」のみ。試験中に `man 2 socket` → `man 2 accept` → `man 2 send` と順に開いても、コードは 1 行も出てこない。この 5 つは開く価値が低いと判断できる。

### Spike 2: zsh は変数展開で単語分割しない

Spike 1 を最初に書いたとき、全 19 項目が `NOT FOUND` になった。

```sh
# 失敗版（zsh）
for p in "2 select" ...; do man $p >/dev/null 2>&1 && ... ; done   # 全部 NOT FOUND
```

`man` に `2 select` という 1 つの引数が渡っていた。bash は `$p` を空白で単語分割するが、**zsh は既定では分割しない**。明示的に分割するには `${=p}` を使う。

```sh
man ${=p}        # → man 2 select として渡る
```

解説: [セッションログ #0001](0001_session_log_exam06.md) に zsh と bash の差の Spike があるが、これはその続き。試験中にシェルのワンライナーで確認を回すとき、Mac の zsh と校舎の環境でここが効く。

### Spike 3: `man FD_ZERO` が通る仕組み

```sh
ls -l /usr/share/man/man3/FD_*.gz
```

```
FD_CLR.3.gz   -> ../man2/select.2.gz
FD_ISSET.3.gz -> ../man2/select.2.gz
FD_SET.3.gz   -> ../man2/select.2.gz
FD_ZERO.3.gz  -> ../man2/select.2.gz
```

解説: マクロ名そのものが `select.2` へのシンボリックリンクになっている。**`man FD_ZERO` と打てばセクション番号を思い出す必要がない。**試験中に「select は 2 だったか 3 だったか」で迷う手間が消える。この 4 リンクは `linux_ref/man/man3/` にも再現した。

### Spike 4: 実機のヘッダに何が書いてあるか

```sh
grep -n "define[[:space:]]*FD_\(ZERO\|SET\|CLR\|ISSET\|SETSIZE\)" /usr/include/x86_64-linux-gnu/sys/select.h
grep -n "struct sockaddr_in$" -A 12 /usr/include/netinet/in.h
grep -n "define[[:space:]]*INADDR_\(ANY\|LOOPBACK\)" /usr/include/netinet/in.h
```

結果:

| 探すもの | 場所 |
|------|------|
| `FD_SETSIZE`、`FD_SET`、`FD_CLR`、`FD_ISSET`、`FD_ZERO` | `sys/select.h` 73〜88 行 |
| `struct sockaddr_in` | `netinet/in.h` 245 行 |
| `INADDR_ANY` = `((in_addr_t) 0x00000000)` | `netinet/in.h` 194 行 |
| `INADDR_LOOPBACK` = `((in_addr_t) 0x7f000001)` | `netinet/in.h` 207 行 |

解説: **man が無い環境でもヘッダは必ずある**（無ければコンパイルできない）。man より確実な代替手段。ヘッダの位置が分からないときは次で出せる（動作確認済み）。

```sh
echo '#include <sys/select.h>' | cc -E -H -xc - -o /dev/null 2>&1 | grep select.h
```

---

## PoC 記録

### PoC 1: 非圧縮 + シンボリックリンクの man ツリーを `man -M` で引けるか

**目的:** 持ち帰ったファイルを、`man 2 select_tut` と同じ打鍵で引ける形に置けるかを確かめる。macOS の man が `.gz` を扱えるか不明なので、非圧縮で成立するかが焦点。

**手順:**

```sh
cd /tmp && mkdir -p reftest/man/man2 reftest/man/man3 reftest/man/man7
for p in select select_tut bind accept socket listen send recv close write; do
	zcat /usr/share/man/man2/$p.2.gz > reftest/man/man2/$p.2
done
zcat /usr/share/man/man7/ip.7.gz > reftest/man/man7/ip.7
for m in FD_ZERO FD_SET FD_ISSET FD_CLR; do ln -sf ../man2/select.2 reftest/man/man3/$m.3; done
man -M /tmp/reftest/man 2 select_tut | head -3
man -M /tmp/reftest/man FD_ZERO      | head -3
du -sh reftest
```

**結果:**

```
SELECT_TUT(2)              Linux Programmer's Manual             SELECT_TUT(2)
SELECT(2)                  Linux Programmer's Manual                 SELECT(2)
212K	reftest
```

**判定: 採用。** 非圧縮でもシンボリックリンク経由でも引ける。212KB はリポジトリの負担にならない。この構成を `linux_ref/` に採用した。

### PoC 2: `env.sh` を zsh で `source` したとき自身の位置を解決できるか

**目的:** Mac 側のリポジトリパスを決め打ちせずに `MANPATH` を通したい。

**手順:**

```sh
printf 'EXAM06_REF="${EXAM06_REF:-${0:A:h}}"\nprint "解決結果: $EXAM06_REF"\n' > /tmp/zshtest/env.sh
cd / && zsh -c 'source /tmp/zshtest/env.sh'
```

**結果:**

```
解決結果: /tmp/zshtest
system manpath: /usr/local/man:/usr/local/share/man:/usr/share/man
```

**判定: 採用。** zsh は `source` したスクリプトの `$0` にそのファイルのパスを入れるので、`${0:A:h}` で自身のディレクトリが取れる。別ディレクトリから `source` しても解決した。

### PoC 3: kernel.org に 5.10 の tarball があるか

**目的:** 校舎の `manpages-dev` 5.10 と同じ upstream を、家の Mac から取得できるか。

**手順:**

```sh
curl -sIL https://mirrors.edge.kernel.org/pub/linux/docs/man-pages/Archive/man-pages-5.10.tar.gz -o /dev/null -w '%{http_code}\n'
curl -s https://mirrors.edge.kernel.org/pub/linux/docs/man-pages/Archive/ | grep -oE 'man-pages-5\.1[0-9]\.tar\.gz' | sort -u
```

**結果:** HTTP 200。`man-pages-5.10.tar.gz` から `5.13` までが Archive にある。

**判定: 採用。** 家でも同一バージョンを取得できる。ただし **tarball には `man3/FD_ZERO.3` などのリンクが含まれない**（Debian のリンクは `make install` が生成している）ので、tarball から構成する場合も `ln -s` は自分で実行する必要がある。

---

## 到達点

1. [man 参照先](../mini_serv_man_reference.md) を作成した。1〜6 節が試験中に引くための内容、7 節が Mac 練習環境の構築手順。
2. `linux_ref/` に Linux 版 man 11 ページ、`FD_*` の 4 リンク、ヘッダ 3 本を置き、コミット 9e02dd9 に入れた。シンボリックリンクは mode 120000 で正しく格納されている（`git ls-files -s` で確認）。
3. 校舎の man のバージョンを `.TH` 行で記録した。`PROVENANCE.txt` に sha256 とともに残してある。

```
.TH SELECT     2 2020-11-01 "Linux" "Linux Programmer's Manual"
.TH SELECT_TUT 2 2020-04-11 "Linux" "Linux Programmer's Manual"
.TH IP         7 2020-11-01 "Linux" "Linux Programmer's Manual"
```

### 今回のコードへ反映できる点

| 箇所 | 内容 |
|------|------|
| `mini_serv.c` の待ち受けアドレス | `htonl(2130706433)`（`bc` で計算した 10 進リテラル）を `htonl(INADDR_LOOPBACK)` にできる。`ip(7)` に「in_addr should be assigned one of the INADDR_* values (e.g., INADDR_LOOPBACK) using htonl(3)」と、`htonl` を通す指示まで書かれている |
| 配布 `main.c` の `accept` | 第 3 引数に `int` 変数のアドレスを渡している。`accept(2)` の SYNOPSIS は `socklen_t *addrlen` なので、`-Wall -Wextra -Werror` では非互換ポインタでエラーになる。`mini_serv.c` は `accept(fd, NULL, NULL)` なので影響しないが、試験で配布 `main.c` から行をコピーすると踏む |

`accept(2)` の DESCRIPTION に「When addr is NULL, nothing is filled in; in this case, addrlen is not used, and should also be NULL.」とあり、`NULL, NULL` は仕様上正しい。

### [課題文 和訳](../mini_serv_ja.md) の劣化を 1 件修正

エディタの Markdown 整形で、`` `client %d: ` `` の末尾スペースがバッククォートの外へ出て `` `client %d:` `` + スペース 2 つになっていた。3 か所（67・100・104 行目）。

「コロンの後にスペース 1 つもバイト列の一部」という、テスターが一致を見る条件が読み取れなくなるため、元の表記に戻した上で「コロンの後にスペース 1 つ」という注記を本文に追加した。整形で再発しても意味が落ちない形にした。併せて固定事項 5 の太字範囲のずれ（`` `select` **がゲートである。** `` → `` **`select` がゲートである。** ``）も戻した。

---

## 次のチャットへの引き継ぎ

次のチャットは、このファイルのこの節から始める。会話の履歴は引き継がれない。

**帰宅後に Mac でやること:**

1. `linux_ref/env.sh` を作る（**校舎では作成漏れ**。内容は [man 参照先](../mini_serv_man_reference.md) の 7.5 節）。
2. `source linux_ref/env.sh` のあと、7.6 節の確認を実行する。基準値は `man 2 select_tut | col -b | grep -c 'FD_SET'` が 7、EXAMPLES 節が 285 行、`man 7 ip | col -b | grep -c 'sin_'` が 8。
3. macOS の man は groff ではなく mandoc で整形するため、`.EX` / `.EE` で囲まれたコード例の見た目が校舎と一致しない可能性がある。上の基準値で欠落していないかを判定する。大きく下回る場合は roff を直接読む（`less linux_ref/man/man2/select_tut.2`）。
4. kernel.org の 5.10 tarball と突き合わせる（7.4 節）。差分が出たら校舎のコピーを採用し、差分の内容を `PROVENANCE.txt` に書き足す。

**この時点の未コミット（帰宅前のコミット対象）:**

| 対象 | 状態 |
|------|------|
| `mini_serv.c` | 空行 4 行の追加と、ファイル末尾への改行付与のみ。ロジック変更なし |
| `mini_serv_ja.md` | 表の整列（整形による）と、上記の劣化 1 件の修正 |
| `linux_ref/env.sh` | 未作成。作ってからコミットする |
| `a.out`, `mini_serv` | ビルド成果物。`.gitignore` に追加してコミット対象外にする |

---

## AI 議事録

- 18:18 開始。試験中にマクロを思い出せなかったという報告を受け、どの man が参考になるかの調査を依頼された。
- 18:19 に Spike 2（zsh の単語分割）で最初のループが全滅した。`man` 自体は動いていたので、シェルの挙動を疑って 2 分で切り分けた。
- 18:24 に [man 参照先](../mini_serv_man_reference.md) を作成（1〜6 節）。
- 18:29 にドライバーから「帰宅後は Mac なので、校舎のうちに man を持ち帰っておくべきか」という提起。macOS には `select_tut(2)` が無く、`struct sockaddr_in` に `sin_len` があるため、持ち帰りに意味があると判断した。方針 5 項目をドライバーが選択。
- 18:35 に 7 節を追記。PoC 1〜3 で構成を検証したうえで手順を書いた。
- 18:42 にドライバーが `linux_ref/` を作成し、18:45 にコミット 9e02dd9。`env.sh` の作成が漏れた。
- 18:46 に未コミットの洗い出しを依頼され、`mini_serv_ja.md` の劣化と `env.sh` の作成漏れを検出した。

**Spike / PoC の選定について:** 上記 Spike 4 件・PoC 3 件は AI が選定した。試験中に再現したい手順（Spike 1・3・4）と、Mac 環境の構築判断の根拠（PoC 1〜3）を基準にした。削る・足すべきものがあればドライバーの指示で改める。

---

## 関連資料

- [man 参照先](../mini_serv_man_reference.md)
- [学習プラン](../mini_serv_study_plan.md)
- [セッションログ #0002](0002_session_log_exam06.md)
- [課題文 和訳](../mini_serv_ja.md)
- [データフロー](../mini_serv_data_flow.md)

---

## 後日訂正

（なし）
