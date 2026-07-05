# bind 失敗（Address already in use）— SO_REUSEADDR 対応

> **種別**: バグ修正 + 開発体験改善（Decision Record / Issue・PR 下書き）  
> **レビュー源**: samatsum 氏プレビュー（2026-07-05、同席なし）  
> **対象**: `src/a/Server.cpp`（コンストラクタ）/ `src/main.cpp`  
> **関連**: [errno_and_nonblocking_42_policy.md](../../a_devdoc/errno_and_nonblocking_42_policy.md)  
> **ステータス**: 実装済み（Linux 検証済み）。Issue → PR → チーム合意待ち  
> **ブランチ案**: `fix/A-port-unavailable-after-exit`（`main` から）

---

## 1. 背景（指摘原文の要約）

samatsum 氏より（2026-07-05）:

> `SO_REUSEADDR` を付けていないことで、しばらくどのターミナルからも前回使用したポートを使用できなくなるバグがある。時間経過で問題ないっぽいから重要度は低い。OS の挙動だから再現が難しい。

再現ログ（samatsum 氏環境）:

```text
$ ./ircserv 4242 pass
terminate called after throwing an instance of 'std::runtime_error'
  what():  bind() 失敗
Aborted (core dumped)

（しばらく待つ）

$ ./ircserv 4242 pass
Server listening on port 4242
```

対策案（samatsum 氏）:

```cpp
if (setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, &optionOn, sizeof(optionOn)) < 0) {
    close(m_listenFd);
    continue;
}
```

---

## 2. 現象

| 状況 | 挙動 |
|------|------|
| サーバ終了直後に同じポートで再起動 | `bind()` が `EADDRINUSE` で失敗することがある |
| 数十秒〜数分待つ | 同じポートで bind 成功 |
| 修正前 | `std::runtime_error` が未捕捉 → `Aborted (core dumped)` |
| 修正後 | `Fatal: bind() 失敗: Address already in use` と表示して終了（exit 1） |

**本番評価への影響**: 低。開発・デバッグ中の再起動体験が主な問題。

---

## 3. 原因

TCP の listen ソケットを `close()` したあと、カーネルはその `(アドレス, ポート)` を **TIME_WAIT** 等の状態でしばらく保持することがある。  
`SO_REUSEADDR` なしで `bind()` すると、直前に使っていたポートが「まだ使用中」と見なされ `EADDRINUSE` になる。

参考: [socket(7) — SO_REUSEADDR](https://man7.org/linux/man-pages/man7/socket.7.html)

### SO_REUSEPORT について（今回は採用しない）

| オプション | 主な用途 | ft_irc での要否 |
|------------|----------|-----------------|
| **SO_REUSEADDR** | 直前に bind していたポートへの **再 bind** を許可（TIME_WAIT 回避）。サーバ再起動の定番 | **必要** |
| **SO_REUSEPORT** | **複数プロセス**が **同時に** 同じ `(IP, port)` に bind し、カーネルが接続を分散（ロードバランシング） | **不要** |

`SO_REUSEPORT` は「複数の `ircserv` を同じポートで並列起動する」用途向け。本プロジェクトは **単一プロセス** の listen ソケットなので、`SO_REUSEADDR` だけで足りる。  
将来マルチプロセス構成にしない限り追加しない。

---

## 4. 対応内容

PR には以下 **3 点すべて** を含める（A/B/C）。

### A. `SO_REUSEADDR` の設定（本修正）

`src/a/Server.cpp` — `socket()` 成功後、`bind()` 前:

```cpp
int opt = 1;
if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    close(_listenFd);
    throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
}
```

### B. `bind()` 失敗メッセージの具体化

```cpp
throw std::runtime_error(std::string("bind() 失敗: ") + strerror(errno));
```

**42 ルールとの関係**: [errno_and_nonblocking_42_policy.md](../../a_devdoc/errno_and_nonblocking_42_policy.md) が禁止しているのは **`read`/`recv`/`write`/`send` 後の `errno` 参照**。`bind()` 失敗時の `errno` は対象外（`accept` と同様、起動時の診断用途）。

### C. `main.cpp` の例外捕捉

```cpp
try {
    Server server(static_cast<int>(port), std::string(argv[2]));
    server.run();
} catch (const std::exception& e) {
    std::cerr << "Fatal: " << e.what() << std::endl;
    return 1;
}
```

core dump ではなく、利用者向けにエラー内容を stderr へ出して終了する。

---

## 5. 再現手順（修正前の症状）

### 5.1 samatsum 氏手順（そのまま転記可）

1. `./ircserv <port> <pass>` で起動
2. サーバを終了（Ctrl+C 等）
3. **すぐに** 同じポートで再実行
4. 修正前: `bind() 失敗` → abort。しばらく待つと成功

### 5.2 Linux 検証（torinoue、2026-07-05）

**A. TIME_WAIT 系（SO_REUSEADDR で改善）**

1. `./ircserv 6667 p` を起動 → Ctrl+C で終了
2. **すぐに** `./ircserv 6667 p` を再実行
3. **修正後期待**: `Server listening on port 6667`（即成功）

**B. 別プロセスが LISTEN 中（SO_REUSEADDR では解決しない）**

バックグラウンド起動例:

```bash
./ircserv 6667 p > LOG &
```

この状態で同ポート起動すると:

```text
Fatal: bind() 失敗: Address already in use
```

→ **前のプロセスを止める必要がある**（次 §6）。

---

## 6. 運用マニュアル — ポート占有時の対処

> 本節は将来 README / USER_DOC へ転載する下書き。

### 6.1 症状

```text
Fatal: bind() 失敗: Address already in use
```

### 6.2 原因の切り分け

| 原因 | 見分け方 | 対処 |
|------|----------|------|
| **別の `ircserv` がまだ動いている** | `lsof` で LISTEN が見える | §6.3 でプロセス停止 |
| **直前に終了したばかり（TIME_WAIT）** | `lsof` に LISTEN なし。修正前のみ再現しやすい | `SO_REUSEADDR` 適用済みなら通常は不要。未適用ビルドでは数十秒待つ |

### 6.3 手順（Linux）

**1. ポートを掴んでいるプロセスを確認**

```bash
lsof -nP -iTCP:<port> -sTCP:LISTEN
```

例（ポート 6667）:

```text
COMMAND     PID     USER   FD   TYPE DEVICE SIZE/OFF NODE NAME
ircserv 1808750 torinoue    3u  IPv4  ...      0t0  TCP *:6667 (LISTEN)
```

**2. 通常終了を試す**

```bash
kill <PID>
```

**3. 終了しない場合（バックグラウンドジョブ等）**

```bash
kill -9 <PID>
```

検証ログでは `kill 1808750` では残り、`kill -9 1808750` で `[1] + killed ./ircserv 6667 p > LOG` となり LISTEN が消えた。

**4. 解放を確認してから再起動**

```bash
lsof -nP -iTCP:<port> -sTCP:LISTEN   # 出力なしであること
./ircserv <port> <pass>
```

### 6.4 フォアグラウンド vs バックグラウンド

| 起動方法 | 終了方法 |
|----------|----------|
| フォアグラウンド（そのターミナルで `./ircserv ...`） | 同ターミナルで **Ctrl+C** |
| バックグラウンド（`... &` や `nohup`） | `jobs` / `lsof` で PID を特定 → `kill` / `kill -9` |

### 6.5 注意

- `SO_REUSEADDR` は **TIME_WAIT による再 bind 拒否** を緩和する。**既に LISTEN 中の別プロセス** とは共存できない
- 評価時は不要な `ircserv` を残さない（ポート競合の原因になる）

---

## 7. 検証チェックリスト（PR 用）

- [ ] `make`（または `make debug`）が通る
- [ ] 起動 → Ctrl+C → **即** 再起動で同ポート bind 成功
- [ ] 意図的に二重起動したとき `Fatal: bind() 失敗: Address already in use` が表示され core dump しない
- [ ] §6.3 の `lsof` / `kill -9` 手順でポート解放後に再起動できる

---

## 8. 影響範囲・マージ方針

- **層**: A 層のみ（public API 変更なし）
- **PR #53**（`A_LAYER_DEBUG`）とは独立 → **先にマージ可**
- **チーム合意**: Issue で可視化 → PR でレビュー。A 層担当 + 全員 peer review（[workflow.md](../../workflow.md) §4）
- **外部プレビュー**: samatsum 氏は read-only。GitHub 上の co-author / write 権限は付与しない（42 制約）

---

## 9. Issue 化用サマリ

**タイトル案**: `fix(A): SO_REUSEADDR 未設定により終了直後の同一ポート bind が失敗する`

**概要**:
サーバ終了直後に同じポートで `./ircserv` を再起動すると `bind()` が `EADDRINUSE` で失敗することがある。`SO_REUSEADDR` を listen ソケットに設定して改善する。併せて `bind()` 失敗時のメッセージ具体化と `main` の例外捕捉で core dump を避ける。

**再現**:
1. `./ircserv <port> <pass>` 起動
2. 終了（Ctrl+C）
3. すぐ同ポートで再実行 → 修正前は bind 失敗（時間経過で成功）

**対応**:
- `setsockopt(SO_REUSEADDR)`（`Server.cpp`）
- `bind()` 失敗に `strerror(errno)`（`Server.cpp`）
- `main.cpp` で `std::exception` を捕捉

**スコープ外**:
- `SO_REUSEPORT`（単一プロセスのため不要）
- SIGINT ハンドラ等の graceful shutdown（別 Issue）

**参考**: [fix_bind_port_reuse.md](fix_bind_port_reuse.md)（本ファイル）、samatsum 氏プレビュー 2026-07-05

---

## 10. PR 説明用サマリ

```markdown
## Summary
- listen ソケットに `SO_REUSEADDR` を設定し、終了直後の同一ポート再起動を可能にした
- `bind()` 失敗時に errno をメッセージへ含め、原因切り分けを容易にした
- `main` で例外を捕捉し core dump の代わりに `Fatal: ...` を stderr へ出力

## Context
samatsum 氏プレビュー（2026-07-05）指摘 #1。詳細は `dev_docs/review_responses/2026-07-05_samatsum_preview/fix_bind_port_reuse.md`

## Test plan
- [ ] 起動 → Ctrl+C → 即再起動（同ポート）成功
- [ ] 二重起動時 `Address already in use` 表示、abort しない
- [ ] `lsof` / `kill -9` 手順（MD §6）でポート解放後に再起動

## Docs
- `dev_docs/review_responses/` 新設（レビュー指摘の索引）
- README 転載用: MD §6 運用マニュアル
```

---

## 11. 一次資料

- [socket(7) — SO_REUSEADDR / SO_REUSEPORT](https://man7.org/linux/man-pages/man7/socket.7.html)
- [bind(2)](https://man7.org/linux/man-pages/man2/bind.2.html)
- 書籍: 「TCP/IPソケットプログラミング C言語編」着信ソケット作成・`SO_REUSEADDR` の説明（該当章）
