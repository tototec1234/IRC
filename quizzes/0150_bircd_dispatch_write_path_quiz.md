# bircd ディスパッチ機構 & 送信経路クイズ

> 対象: bircd Lesson 3.2（FD イベントのディスパッチ骨格）/ Lesson 3.4（呼ばれない client_write）
> 教材: [bircd_learning_curriculum.md](../bircd/bircd_learning_curriculum.md) Lesson 3、`bircd/` ソースコード
> 目的: Lesson 3.2 / 3.4 の読解結果が定着しているか確認する
> 作成日: 2026-06-12

---

## 使い方

1. bircd のソースを**見ずに**回答を試みる
2. 分からない問題は「?」と記入
3. 回答後、ドライバーの指示で正解・解説を追記する（[phase_plan.md](../dev_docs/project_management/phase_plan.md) のクイズ運用ルール参照）

---

## ディスパッチ機構

### Q1: `client_read` を直接呼んでいるソースファイルはどれか？

- a) `main_loop.c`
- b) `check_fd.c`
- c) `srv_accept.c`
- d) 直接呼ぶ箇所は存在しない

**自分の回答**:

### Q2: `fct_read` スロットに `client_read` を代入しているのはどのタイミングか？

- a) サーバー起動時（`srv_create`）
- b) クライアント接続受付時（`srv_accept`）
- c) poll がイベントを返した時（`check_fd`）
- d) 毎ループの監視対象構築時（`init_fd`）

**自分の回答**:

### Q3: bircd の FD イベントディスパッチと B 層 `CommandDispatcher` の違いとして**正しくない**ものはどれか？

- a) 前者は FD イベント、後者は IRC コマンドを振り分ける
- b) 前者の分岐キーは fd の種類、後者はコマンド名
- c) 前者は ircserv では B 層の責任範囲である
- d) 後者がディスパッチする対象はアプリケーション層（L7）のプロトコル要素である

**自分の回答**:

### Q4: poll の FD イベント（POLLIN 等）は層モデル上の何か？

- a) アプリケーション層プロトコルの一部
- b) トランスポート層プロトコルの一部
- c) どの層のプロトコルでもない。カーネルが TCP（L4）の状態変化を通知するソケット API の仕組み
- d) ネットワーク層のヘッダフィールド

**自分の回答**:

### Q5: クライアント fd の POLLIN で `recv` が 0 を返した。裏で起きた TCP の出来事は？

- a) RST 受信
- b) FIN 受信（相手が正常クローズ）
- c) ACK 受信
- d) 再送タイムアウト

**自分の回答**:

---

## 送信経路（呼ばれない client_write）

### Q6: 現状の bircd（poll 版）で `client_write` は実行時に呼ばれるか？

- a) 毎ループ呼ばれる
- b) クライアントが2人以上いるとき呼ばれる
- c) 一度も呼ばれない
- d) 切断時のみ呼ばれる

**自分の回答**:

### Q7: Q6 の根拠となる「証拠の連鎖」の最後の環（直接原因）はどれか？

- a) `fct_write` スロットへの登録が無い
- b) `check_fd` に POLLOUT のディスパッチコードが無い
- c) `buf_write` に書き込むコードがどこにも無いため、POLLOUT が監視対象にならない
- d) `client_write.c` が空関数だから

**自分の回答**:

### Q8: `client_read` の `recv` → 即 `send` がアンチパターンである理由として**正しくない**ものはどれか？

- a) 送信先のソケットバッファ満杯で `send` がブロックし、全体が止まりうる
- b) ノンブロッキング時の部分送信 / EAGAIN への対処が無い
- c) ft_irc の評価要件「全 read/write は poll 等価物を1回通す」に違反する
- d) `recv` と `send` を同一関数内で呼ぶこと自体が POSIX 違反である

**自分の回答**:

### Q9: `init_fd` が「`buf_write` が空なら POLLOUT を監視しない」理由は？

- a) POLLOUT は送信バッファに空きがあれば常に立つため、監視すると poll が即返り busy loop になる
- b) POLLOUT と POLLIN は同時に監視できないため
- c) `events` のビット数に上限があるため
- d) カーネルが空バッファの fd を拒否するため

**自分の回答**:

### Q10: ft_irc 実装で `client_write` 相当が満たすべき設計を1〜3行で書け（自由記述）

**自分の回答**:

---

## 一次資料

- `man 2 poll` / `man 2 send` / `man 2 recv`
- [RFC 793 (TCP)](https://datatracker.ietf.org/doc/html/rfc793)
- [RFC 1122 (Requirements for Internet Hosts — 層モデル)](https://datatracker.ietf.org/doc/html/rfc1122)
- [RFC 1459 (IRC)](https://datatracker.ietf.org/doc/html/rfc1459)
- ソース: `bircd/bircd.h` / `bircd/srv_create.c` / `bircd/srv_accept.c` / `bircd/check_fd.c` / `bircd/init_fd.c` / `bircd/client_read.c` / `bircd/client_write.c`
