# errno 参照禁止とノンブロッキング — 書籍と 42 ルールの読み替え

> **作成者**: torinoue
> **ステータス**: 知識整理（2026-06-19）
> **対象**: `src/a/Connection.cpp`（recv/send）/ `src/a/Server.cpp`（run/_acceptClient）
> **関連**: [a_implementation_plan.md](a_implementation_plan.md), [a_layer_io_flow.md](a_layer_io_flow.md), [../coding_standards/error_handling.md](../coding_standards/error_handling.md)

---

## 1. 結論（先に要点）

- 42 は **`read`/`recv`/`write`/`send` の後に `errno` を参照することを禁止**している（破ると評価 0 点対象）。
- 参考図書「TCP/IPソケットプログラミング C言語編」(5.3.1) は逆に **`EWOULDBLOCK`/`EAGAIN` を `errno` で判定する**標準手法を教える。
- したがって **書籍をそのまま写すと 42 で減点**。書籍の `errno` 判定を **`poll()` のレディネス判定に読み替える**必要がある。
- 現状の A 層コードは recv/send で `errno` を見ておらず、**すでに準拠**。EAGAIN 分岐を「足さない」ことが正解。

---

## 2. なぜ 42 は errno 参照を禁止するのか

### (a) 設計強制 — poll を迂回させないため

42 の核心要件は「**全ての I/O を `poll()` のレディネス判定経由で行う**」こと。
`errno == EAGAIN` を見て `recv` をループで回せると、`poll` を通さず「読めるまで read し続ける」busy-loop が書けてしまう。これは CPU を浪費し評価 0 点対象。`errno` 参照を一律禁止することで抜け道を塞いでいる。

→ 本実装の設計（`poll` が POLLIN を返したときに **1 回だけ** recv、残りは次の `poll` に任せる）なら、**そもそも EAGAIN を見る必要が無い**。これが「最速かつ準拠」の理由。

### (b) POSIX 的な落とし穴 — errno は -1 のときしか意味を持たない

`errno(3)` の仕様:

- `errno` は**システムコールが -1（エラー）を返したときのみ**有効。
- **成功した呼び出しは `errno` を 0 に戻さない**（前のエラーの残骸が残る）。

→ `recv` が `>0`（成功）を返した後に `errno` を見ると、無関係な過去のエラー値を読む。戻り値だけで判断する（`>0` / `0` / `<0`）規律につながる。

---

## 3. 書籍 vs 42 の読み替え表

| 観点 | 一般的なソケット本 / POSIX | 42 ft_irc | 本実装の方針 |
|------|---------------------------|-----------|-------------|
| 読めるか判定 | `errno == EWOULDBLOCK` | `poll()` の POLLIN | `poll()` |
| recv/send 後の `errno` | 見てよい（標準） | **禁止** | 見ない |
| read のループ | EAGAIN まで drain も可 | poll 1 イベント = 1 read | 1 イベント 1 recv |
| send 残データ | EAGAIN で再試行 | poll の POLLOUT で再試行 | POLLOUT トグル |

---

## 4. 現状コードの準拠状況

### 準拠している箇所（このまま維持）

- `Connection::readFromSocket()` — `n == 0` / `n < 0` を戻り値だけで判定。`errno` 不使用。
- `Connection::writeToSocket()` — `sent <= 0` を戻り値だけで判定。`errno` 不使用。

### 注意箇所

- `Server::_acceptClient()` の `errno == EAGAIN || errno == EWOULDBLOCK`
  - `accept` 後の `errno` 参照であり、禁止対象の「read/recv/write/send 後」には**該当しない**（0 点対象ではない）。
  - ただし **1 イベント 1 accept** の設計では `poll` が POLLIN を返した直後の `accept` は成功するため、この分岐は**実質デッドコード**。規約への安全側として削除も可。

### 足してはいけないパターン（アンチパターン）

```cpp
// ✗ 42 では禁止: recv 後の errno 参照
ssize_t n = recv(fd, buf, sizeof(buf), 0);
if (n < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) continue;  // ← これがダメ
    return false;
}
```

`a_implementation_plan.md` §4.2 の参照コードや `Connection.cpp` のコメントにある「Phase7 で EAGAIN を分ける」は、この読み替えに照らすと**実装不要**。

---

## 5. ノンブロッキング化で実際に必要な作業

`errno` 処理ではなく、以下だけ:

1. `fcntl(fd, F_SETFL, O_NONBLOCK)` を listen fd（`Server` ctor）と cs（`_acceptClient`）に適用。
   - macOS / 42 制約: **`fcntl(fd, F_SETFL, O_NONBLOCK)` の形のみ許可**（`F_GETFL` で取得して OR する書き方は不可）。
2. `signal(SIGPIPE, SIG_IGN)` — send 中に相手切断で SIGPIPE → プロセス即死を防止。

---

## 6. 参考 URL

| 資料 | 内容 |
|------|------|
| [オーム社 書籍ページ](https://www.ohmsha.co.jp/book/9784274065194/) | 「TCP/IPソケットプログラミング C言語編」一次資料。日本語版サンプル CSockets.tar.gz |
| [man recv(2)](https://man7.org/linux/man-pages/man2/recv.2.html) | `EAGAIN/EWOULDBLOCK` の意味（ノンブロッキングで「今は無い」） |
| [man errno(3)](https://man7.org/linux/man-pages/man3/errno.3.html) | 「errno is never set to zero by any system call」「examine only when the return indicates an error」（§2(b) の根拠） |
| [man fcntl(2)](https://man7.org/linux/man-pages/man2/fcntl.2.html) | `F_SETFL` / `O_NONBLOCK` |
| [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) | ノンブロッキング + `poll` の実例 |
| 42 ft_irc subject（手元 PDF） | "checking the value of errno ... is strictly forbidden" の原文を一次確認 |

---

## 変更履歴

| 日付 | 変更者 | 内容 |
|------|--------|------|
| 2026-06-19 | torinoue | 初版（errno 禁止 × 書籍読み替えの整理） |
