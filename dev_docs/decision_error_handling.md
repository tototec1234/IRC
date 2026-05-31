# 設計決定: エラーハンドリングと所有権

> **ステータス**: 決定（2026-05-29）  
> **セッション**: #0006  
> **関連**: [design.md](./design.md), [interface.md](./interface.md), [chapter2_general_rules.md](../docs/chapter2_general_rules.md), [decision_no_custom_templates.md](./decision_no_custom_templates.md)

---

## 1. 決定内容（概要）

| 区分 | 方針 |
|------|------|
| **C++ 例外** | 起動失敗のみ。メインループから未 catch 例外を出さない |
| **I/O エラー** | `bool` 戻り値 + 接続単位 disconnect |
| **IRC プロトコルエラー** | `ReplyBuilder` → `CommandResult`（numeric）。例外にしない |
| **所有権** | `ServerState` が `Client` / `Channel` を所有。`auto_ptr` は使わない |
| **層境界** | 例外は層を越えない。A↔B は `CommandResult` のみ |

---

## 2. エラーの種類と渡し方

| 種類 | 例 | 手段 | 層 |
|------|-----|------|-----|
| プロトコルエラー | 未登録で PRIVMSG、nick 重複 | numeric → `CommandResult` | B |
| I/O 正常待ち | EAGAIN / EWOULDBLOCK | 何もしない | A |
| I/O 致命（接続単位） | recv=0、send 致命 errno | `bool false` → disconnect | A |
| 起動失敗 | bind / listen 失敗 | `throw` → `main` で catch | A / main |
| プログラミングミス | NULL 参照 | 防御的チェック（クラッシュ禁止） | 全層 |

**用語（注釈用）**

- **例外の伝播（Exception propagation）**: `throw` した例外が呼び出し元へ上がること
- **例外境界（Exception boundary）**: 例外を catch して別表現（ログ、return 1 等）に変換する線
- **エラーハンドリング方針（Error handling strategy）**: 例外 / 戻り値 / Result の使い分けルール

---

## 3. 例外境界

```text
main          ← 境界①: 起動失敗を catch
  └─ Server   ← 境界②: ループ内は throw しない
       ├─ Connection  → bool
       └─ dispatch     → CommandResult
```

### 3.1 main / 起動（A）

```cpp
try {
    Server server(port, password);
    server.run();
} catch (const std::exception& e) {
    std::cerr << "Fatal: " << e.what() << std::endl;
    return 1;
}
```

### 3.2 メインループ（A）

- クライアント fd の問題 → `disconnectClient(fd)`。throw しない
- `poll()` 致命失敗 → ログしてループ終了（または main まで throw し、境界①で catch）

### 3.3 B 層

- パース失敗・権限不足 → numeric を `CommandResult` に詰める
- **`throw` しない**

---

## 4. 所有権ルール

### 4.1 基本

```text
ServerState  ──所有（new/delete）──> Client, Channel
Channel      ──非所有参照──────────> Client*
B 層         ──非所有参照──────────> Client*, Channel*
Connection   ──所有（close）──────> fd
```

### 4.2 具体ルール

| 対象 | 所有者 | 解放 |
|------|--------|------|
| `Client` | `ServerState` | `removeClient(fd)` のみ |
| `Channel` | `ServerState` | `removeChannelIfEmpty()` のみ |
| `Client*` in Channel | 非所有 | `delete` 禁止 |
| fd | `Connection` | デストラクタで `close()` |

### 4.3 `auto_ptr` を使わない

C++98 では `auto_ptr` があるが、本プロジェクトでは **採用しない**。

| 理由 | 説明 |
|------|------|
| コピー = 所有権移譲 | `std::map` 操作で意図せず null 化しやすい |
| 非所有参照モデル | B/C2 は `Client*` で参照。1 所有者 + 多数参照と相性が悪い |
| 設計と一致 | 現設計は `ServerState::removeClient()` 単一経路で十分 |

`new` 失敗時は `main` の catch、または接続単位の巻き戻し + disconnect。`auto_ptr` は不要。

### 4.4 RAII[^010]

- **Connection**: fd をデストラクタで close
- **ServerState**: `removeClient()` 内で Channel 掃除 → `delete Client` の順を守る

---

## 5. Client 削除 API（B 層との関係）

### 5.1 B が呼ぶのは `removeClient(fd)` のみ

| 呼び出し元 | API | 典型シーン |
|-----------|-----|-----------|
| **B** | `ServerState::removeClient(fd)` | QUIT コマンド、`CommandResult.shouldDisconnect` 後 |
| **A / Server** | `ServerState::removeClient(fd)` | I/O 切断、disconnect 処理 |

B は **`removeClientFromAllChannels` を直接呼ばない**。

### 5.2 `removeClientFromAllChannels` は private

`ServerState::removeClient(fd)` の内部実装詳細。公開 API には載せない。

```cpp
void ServerState::removeClient(int fd) {
    Client* client = getClientByFd(fd);
    if (!client) return;
    removeClientFromAllChannels(*client);  // private
    // nick 辞書更新、delete client、fd 辞書 erase
}
```

根拠: [0004 設計整合性チャット](79d56ff9-fd4a-4f05-821a-7458b04dcdac) で合意。詳細は [decision_invite_and_removal.md](./decision_invite_and_removal.md)。

---

## 6. 関連ドキュメント

| ドキュメント | 内容 |
|-------------|------|
| [decision_no_custom_templates.md](./decision_no_custom_templates.md) | STL + 具象クラス方針 |
| [decision_invite_and_removal.md](./decision_invite_and_removal.md) | invite 命名、removeClient 内部化 |
| [A_fd_responsibility_design.md](./A_fd_responsibility_design.md) | C++98 コーディング注意 |

---

## 脚注

[^010]: **RAII** — コンストラクタで資源取得、デストラクタで解放。例外 unwind 時もデストラクタが走りやすい。

---

## 変更履歴

| 日付 | 内容 |
|------|------|
| 2026-05-29 | 初版（セッション #0006） |
