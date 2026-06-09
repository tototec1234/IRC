# bircd Lesson ↔ ircserv A 層 Phase 対応表

> **用語**
> - **Lesson**（本表の左列）= [bircd 学習カリキュラム](bircd_learning_curriculum.md) の段階
> - **Phase**（本表の右列）= [ircserv A 層実装計画](../a_header_tmp/a_implementation_plan.md) の機能軸フェーズ

混同防止のため、ドキュメント上はこの2語を使い分ける。

---

## 対応一覧

| bircd Lesson | ircserv A 層 Phase | 学べる範囲 | 備考 |
|--------------|-------------------|-----------|------|
| **Lesson 1** bircd 完全理解 | **Phase 0〜1** 起動骨格 / Listen ソケット | ◎ ほぼ全部 | `main` / `get_opt` / `srv_create` |
| **Lesson 2** select → poll | **Phase 2** poll ループ + Accept | ◎ ほぼ全部 | [lesson2_guide.md](lesson2_guide.md)。本リポは poll 版済み |
| **Lesson 3** バッファリング | **Phase 3** Connection + 受信バッファ / **Phase 5** 送信経路 | △ 概念・演習 | bircd 本体に `\r\n` 切り出しなし。`client_write.c` は空 |
| **Lesson 4** ノンブロッキング I/O | **Phase 7** ノンブロッキング化 | △ カリキュラム演習 | bircd 本体はブロッキングのまま |
| **Lesson 5** C++98 化 | **Phase 0〜8** A 層全体の写経指針 | △ 設計対応表 | ft_irc 本体（`src/a/`）への橋渡し |
| （該当 Lesson なし） | **Phase 4** B 層連携 | ✗ | Parser / Dispatcher は bircd に無い |
| **Lesson 3** の一部 + **Lesson 1** の切断パターン | **Phase 6** 切断・エラー処理 | △ 最小限 | `recv==0` のみ本体にあり。`POLLERR/HUP`・`SIGPIPE`・`QUIT` は別途 |
| — | **Phase 8** 仕上げ | △ 参考程度 | `inet_ntoa`（`srv_accept.c`）等 |

---

## PING/PONG 結合テスト合格に必要な Phase

ircserv の **Phase 5 完了**がゴール（[a_implementation_plan.md](../a_header_tmp/a_implementation_plan.md) §3）。

bircd Lesson だけでは足りない部分:

| 不足する論点 | どこで学ぶか |
|-------------|-------------|
| Parser → Dispatcher → CommandResult | ircserv **Phase 4**（B 層完成済み） |
| `applyCommandResult` + POLLOUT 送信 | ircserv **Phase 5** + Lesson 3 の送信バッファ概念 |
| 提出必須の fcntl | ircserv **Phase 7** + Lesson 4 |

---

## 読む順序（推奨）

```
Lesson 1 → Lesson 2 → ircserv Phase 2 仕上げ
    → Lesson 3（受信）→ ircserv Phase 3
    → ircserv Phase 4（B 層連携）
    → Lesson 3（送信）→ ircserv Phase 5 🎯 PING/PONG
    → ircserv Phase 6 → Lesson 4 → ircserv Phase 7 → Phase 8
```

---

## 関連ドキュメント

- [bircd_learning_curriculum.md](bircd_learning_curriculum.md)
- [lesson2_guide.md](lesson2_guide.md)
- [README.md](README.md) — Git タグ（`lesson-1` / `lesson-2`）
- [a_implementation_plan.md](../a_header_tmp/a_implementation_plan.md)
