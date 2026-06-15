# B 層スタブ実装方針

> **ステータス**: 決定（2026-05-29）  
> **セッション**: #0006  
> **関連**: [proposal_stubs_20260523.md](./proposal_stubs_20260523.md)（C1/C2 スタブ）, [interface.md](./interface.md), [b_implementation_reader.md](./b_implementation_reader.md), [b_layer_reply_result_flow.md](./b_layer_reply_result_flow.md)

---

## 1. 目的

A 層（Network / IO）を B 層（Protocol / Command）と結合テストするため、**最小限の B 層スタブ**を先に用意する。

C1/C2 未完成でも A↔B 境界（complete line → `CommandResult`）を検証できるようにする。

---

## 2. スコープ（MVP スタブ）

| クラス | スタブ化 | 内容 |
|--------|----------|------|
| **Parser** | ✅ 実装 | 1 行 → `Message`（最低限: command + params 分割） |
| **CommandDispatcher** | ✅ 最小 | 未知コマンドは echo、`PING` → `PONG` 程度 |
| **Message** | ✅ 実装 | [`class_overview_diagram.md`](./diagrams/class_overview_diagram.md) + [`interface.md`](./interface.md) §3.1 に整合 |
| **CommandResult** | ✅ 実装 | 契約は [`interface.md`](./interface.md) §1、構造参考は [`b_implementation_reader.md`](./b_implementation_reader.md) §4.2 |
| **ReplyBuilder** | ⏸ 後回し | Dispatcher が固定文字列で返す |
| **C1/C2** | ✅ 流用 | `proposal_stubs_20260523.md` のスタブを使用 |

**今回作らない:** ReplyBuilder 全 numeric、全コマンド handler、ChannelService。

---

## 3. 配置先

```
IRC_torinoue/
  src/                    ← 将来の本実装（Phase 4 以降）
  stubs/                  ← スタブ・結合用（本ドキュメントの対象）
    b/
      Parser.hpp / .cpp
      Message.hpp / .cpp
      CommandDispatcher.hpp / .cpp
      CommandResult.hpp / .cpp
    c1/                   ← proposal_stubs から移植
    c2/
```

**方針 md 先行。** スタブコード本体は Phase 4 着手時に `IRC_torinoue/stubs/` へ新規作成（公開 API: [`class_overview_diagram.md`](./diagrams/class_overview_diagram.md)、契約憲章: [`interface.md`](./interface.md)、実装読み物: [`b_implementation_reader.md`](./b_implementation_reader.md)）。

---

## 4. CommandDispatcher 最小仕様

```cpp
CommandResult CommandDispatcher::dispatch(int fd, const Message& msg, ServerState& state);
```

| command | 動作 |
|---------|------|
| `PING` | `PONG :<servername>` を `CommandResult` に追加 |
| その他 | echo: `":<prefix> ECHO :" + 原文` または固定 `OK`（A 層結合確認用） |
| `QUIT` | （将来）`shouldDisconnect = true`。Client 削除は A 層の disconnect 処理に委譲 |

**依存:** `ServerState` スタブ（C1）。Channel スタブ（C2）は echo 段階では不要。

---

## 5. テスト方法

### 5.1 A 層未完成時

- Parser 単体: 文字列 in → `Message` assert
- Dispatcher 単体: `Message` + `ServerStateStub` → `CommandResult` assert

### 5.2 A 層完成後

```text
nc → Server → Connection::popLine()
  → Parser::parse()
  → CommandDispatcher::dispatch()
  → Server::applyCommandResult()
  → Connection::bufferSend()
```

---

## 6. C1/C2 スタブとの関係

| ドキュメント | 対象 |
|-------------|------|
| `proposal_stubs_20260523.md` | Client, ServerState, Channel, ChannelModes |
| 本ドキュメント | Parser, Message, CommandDispatcher, CommandResult |

B 層スタブテストでは **C1 ServerStateStub を必須**、C2 は JOIN/INVITE 実装段階まで不要。

---

## 7. 実装順序

1. `Message` + `Parser`（B 単体）
2. `CommandResult`（契約: [`interface.md`](./interface.md) §1、構造参考: [`b_implementation_reader.md`](./b_implementation_reader.md) §4.2）
3. `CommandDispatcher` echo 版
4. C1 スタブ移植（proposal から）
5. A 層と結合

---

## 変更履歴

| 日付 | 内容 |
|------|------|
| 2026-05-29 | 外部リポジトリ言及削除、SSOT 参照に統一 |
