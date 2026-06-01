# 層間インターフェース仕様

> **SSOT（契約憲章）**: 層間契約の**理由・ルール・設計決定**を定める。経緯と「なぜそうなったか」を読むための憲章的ドキュメント。  
> **公開 API の正**: [`diagrams/class_overview_diagram.md`](./diagrams/class_overview_diagram.md)  
> **実装読み物（B層主読者）**: [`b_implementation_reader.md`](./b_implementation_reader.md) — SSOT ではない  
> 各層の責務は [`design.md`](./design.md) を参照。

本ドキュメントは層間 API の「何を」ではなく「なぜ・どう守るか」を定める憲章である。  
公開 API のメソッド名・シグネチャは [`class_overview_diagram.md`](./diagrams/class_overview_diagram.md) を正とする。  
Processing Flow や各 API の呼び出し元・利用コンテキストは [`b_implementation_reader.md`](./b_implementation_reader.md) を参照する（本書は SSOT ではない）。

## スコープ

```
A層 ─complete line→ B層 ─CommandResult→ A層
                     │
          ┌──────────┴──────────┐
          ↓                     ↓
        C1層                  C2層
    (Client, ServerState)  (Channel, ChannelModes)
```

| 境界 | 方向 | 内容 |
|------|------|------|
| A→B | 入力 | complete line（`\r\n` 区切り文字列） |
| B→A | 出力 | `CommandResult`（送信先fd + 送信文字列） |
| B↔C1 | 双方向 | `Client`, `ServerState` 操作API |
| B↔C2 | 双方向 | `Channel`, `ChannelModes` 操作API |

---

## チーム構成

| 担当 | メンバー | 範囲 |
|------|---------|------|
| A | torinoue | Network / IO（Server, Connection） |
| B | torinoue | Protocol / Command（Parser, Dispatcher, ReplyBuilder） |
| C1 | taro | Client / ServerState |
| C2 | hanako | Channel / ChannelModes |

---

## 1. 境界オブジェクト（A↔B）

### 1.1 CommandResult

B層からA層への戻り値。送信先fdと送信文字列を保持。

```cpp
struct CommandResult {
    std::vector<OutgoingMessage> replies;
    bool shouldDisconnect;
    
    CommandResult();
    void addReply(int fd, const std::string& message);
};
```

**役割:**
- コマンド処理後に送るべきメッセージを保持
- 必要に応じて切断要求を表す
- BがAの `Connection` や `Server` を直接触らないための境界

### 1.2 OutgoingMessage

```cpp
struct OutgoingMessage {
    int         fd;
    std::string message;
    
    OutgoingMessage(int targetFd, const std::string& text);
};
```

**役割:**
- Bが「誰に何を送るか」を表現
- 実際の `send()` は行わない
- A / Server がこの情報を見てsend bufferへ積む

---

## 2. A層概要（Network / IO）

> 詳細は `onboarding_A.md` 参照

| クラス | 責務 |
|--------|------|
| `Server` | メインループ、イベント振り分け、`CommandResult` 適用 |
| `Connection` | fd、recv/send buffer、ノンブロッキングI/O |
| `Poller` | (optional) pollfd管理 |
| `ConnectionManager` | (optional) Connection辞書管理 |

**A層はIRCコマンドの意味を知らない。**

---

## 3. B層概要（Protocol / Command）

> 詳細は `onboarding_B.md` 参照

| クラス | 責務 |
|--------|------|
| `Parser` | 文字列 → `Message` 変換 |
| `Message` | IRCメッセージ構造体 |
| `CommandDispatcher` | コマンド振り分け、状態更新、結果生成 |
| `ReplyBuilder` | 返信文字列生成（Numeric Reply等） |

**B層はNetwork/IOクラスに依存しない。送信は `CommandResult` 経由。**

### 3.1 Message

```cpp
class Message {
private:
    std::string              _command;
    std::vector<std::string> _params;
public:
    const std::string& getCommand() const;
    const std::vector<std::string>& getParams() const;
    size_t getParamCount() const;
    const std::string& getSingleParam(size_t index) const;
    bool hasParam(size_t index) const;
};
```

---

## 4. C1層インターフェース（taro担当）

### 4.1 Client

| 関数 | 戻り値 | 呼び出し元 | 説明 |
|------|--------|-----------|------|
| `int getFd() const` | `int` | B, C2 | fdを取得 |
| `const std::string& getNick() const` | `const std::string&` | B, C2 | nickを取得 |
| `const std::string& getUsername() const` | `const std::string&` | B | usernameを取得 |
| `const std::string& getRealname() const` | `const std::string&` | B | realnameを取得 |
| `const std::string& getHost() const` | `const std::string&` | B | hostを取得 |
| `std::string getFullPrefix() const` | `std::string` | B | `nick!user@host` を返す |
| `void setUsername(const std::string&)` | `void` | B | username設定 |
| `void setRealname(const std::string&)` | `void` | B | realname設定 |
| `void setHost(const std::string&)` | `void` | A | host設定（接続時） |
| `void setPassOk(bool)` | `void` | B | PASS成功状態設定 |
| `bool isPassOk() const` | `bool` | B | PASS済みか |
| `bool isRegistered() const` | `bool` | B | 登録完了か |
| `bool canRegister() const` | `bool` | B | 登録可能か（PASS/NICK/USER揃った） |
| `void markRegistered()` | `void` | B | 登録完了にする |

**⚠️ 注意**: `setNick()` は外部から直接呼ばない。`ServerState::updateNick()` を使う。

**getFullPrefix()** は RFC 1459 Section 2.3 に基づき、サーバー→クライアントのメッセージに必要。詳細は `rfc1459_prefix_analysis.md` 参照。

### 4.2 ServerState

| 関数 | 戻り値 | 呼び出し元 | 説明 |
|------|--------|-----------|------|
| `const std::string& password() const` | `const std::string&` | B | サーバパスワード |
| `void addClient(int fd)` | `void` | Server | Client作成 |
| `void removeClient(int fd)` | `void` | Server, B | Client削除（Channel参照も除去） |
| `Client* getClientByFd(int fd)` | `Client*` | B | fdからClient取得 |
| `Client* getClientByNick(const std::string&)` | `Client*` | B | nickからClient取得 |
| `bool nickExists(const std::string&) const` | `bool` | B | nick重複確認 |
| `void updateNick(Client&, const std::string&)` | `void` | B | nick変更（辞書も更新） |
| `Channel* getChannel(const std::string&)` | `Channel*` | B | Channel取得（なければNULL） |
| `Channel* getOrCreateChannel(const std::string&)` | `Channel*` | B | Channel取得or作成 |
| `void removeChannelIfEmpty(const std::string&)` | `void` | B / Server[^1] | 空Channel削除  |

[^1]: disconnect 時のクリーンアップで Server が呼ぶケースがあるため。

---

## 5. C2層インターフェース（hanako担当）

### 5.1 Channel

| 関数 | 戻り値 | 呼び出し元 | 説明 |
|------|--------|-----------|------|
| `const std::string& name() const` | `const std::string&` | B, C1 | channel名 |
| `bool hasMember(Client*) const` | `bool` | B | 参加済みか |
| `void addMember(Client*)` | `void` | B | member追加 |
| `void removeMember(Client*)` | `void` | B | member削除 |
| `void removeClient(Client*)` | `void` | B, C1 | member/operator/invited一括削除 |
| `std::vector<Client*> members() const` | `std::vector<Client*>` | B | member一覧 |
| `size_t memberCount() const` | `size_t` | B | member数 |
| `bool isOperator(Client*) const` | `bool` | B | operator権限確認 |
| `void addOperator(Client*)` | `void` | B | operator付与 |
| `void removeOperator(Client*)` | `void` | B | operator剥奪 |
| `void addInvite(Client*)` | `void` | B | `_invited` へ追加（状態変更。`ReplyBuilder.invite` は通知文字列生成） |
| `bool isInvited(Client*) const` | `bool` | B | 招待済みか |
| `void removeInvite(Client*)` | `void` | B | 招待解除 |
| `void setTopic(const std::string&)` | `void` | B | topic設定 |
| `const std::string& topic() const` | `const std::string&` | B | topic取得 |
| `ChannelModes& modes()` | `ChannelModes&` | B | mode変更用 |
| `const ChannelModes& modes() const` | `const ChannelModes&` | B | mode参照用 |
| `bool isEmpty() const` | `bool` | B, C1 | member 0人か |

### 5.2 ChannelModes

| 関数 | 戻り値 | 呼び出し元 | 説明 |
|------|--------|-----------|------|
| `bool inviteOnly() const` | `bool` | B | +i状態 |
| `bool topicRestricted() const` | `bool` | B | +t状態 |
| `bool hasKey() const` | `bool` | B | +k状態 |
| `const std::string& key() const` | `const std::string&` | B | パスワード |
| `int limit() const` | `int` | B | 人数制限（-1=無制限） |
| `void setInviteOnly(bool)` | `void` | B | +i/-i設定 |
| `void setTopicRestricted(bool)` | `void` | B | +t/-t設定 |
| `void setKey(const std::string&)` | `void` | B | +k設定 |
| `void unsetKey()` | `void` | B | -k設定 |
| `void setLimit(int)` | `void` | B | +l設定 |
| `void unsetLimit()` | `void` | B | -l設定 |

---

## 6. 重要ルール

### 6.1 【実装】nick変更は ServerState 経由

```cpp
// ❌ NG
client.setNick("newNick");

// ✅ OK
state.updateNick(client, "newNick");
```

### 6.2 【設計】Client削除は ServerState 経由

```cpp
// ServerState::removeClient(fd) が自動処理:
// - 全Channelからmember/operator/invited削除
// - fd辞書・nick辞書の更新
// - 空Channelの削除
```

### 6.3 【設計】operator権限は Channel が管理

```cpp
// ❌ NG
client.setOperator(true);

// ✅ OK
channel.addOperator(&client);
```

### 6.4 【設計】Channel は Client を所有しない

- `Channel` は `Client*` を保持するだけ
- `delete` は `ServerState` の責務

### 6.5 【設計】B層は送信処理を行わない

- `CommandDispatcher` は `send()` を呼ばない
- 結果は `CommandResult` に詰めて返す
- A層が `CommandResult` を適用して送信

---

## 7. 未確定事項（Open Questions）

| 項目 | 現状 | 決定タイミング |
|------|------|---------------|
| Poller分離 | optional | 実装時判断 |
| ConnectionManager分離 | optional | 実装時判断 |
| ClientRegistry分離 | optional | 実装時判断 |
| ChannelService分離 | optional | 実装時判断 |

### 7.1 確定済み（設計決定）

| 項目 | 決定 | 参照 |
|------|------|------|
| 自作 template | 不使用 | `decision_no_custom_templates.md` |
| エラー・所有権 | 起動時例外 / ループは bool+Result / ServerState 所有 | `decision_error_handling.md` |
| `removeClientFromAllChannels` | ServerState **private**。B は `removeClient(fd)` のみ | `decision_invite_and_removal.md` |
| invite 系命名 | `addInvite`（C2）+ `ReplyBuilder.invite`（B）共存 | `decision_invite_and_removal.md` |

---

## 8. 関連ドキュメント

| ドキュメント | 内容 |
|-------------|------|
| `design.md` | 全体設計・責務分割 |
| `b_implementation_reader.md` | B層主読者向け実装読み物（SSOT ではない。Processing Flow、呼び出し元等） |
| `diagrams/class_overview_diagram.md` | 公開 API・クラス関係（SSOT） |
| `onboarding_A.md` | A層オンボーディング |
| `onboarding_B.md` | B層オンボーディング |
| `onboarding_C1.md` | taro向けオンボーディング |
| `onboarding_C2.md` | hanako向けオンボーディング |
| `rfc1459_prefix_analysis.md` | getFullPrefix() の根拠 |

---

## 変更履歴

| 日付 | 内容 |
|------|------|
| 2026-05-23 | 初版作成 |
| 2026-05-26 | 層間API契約書として再構成、外部リポジトリ依存削除 |
| 2026-06-01 | MTG決定: 契約憲章（SSOT）として位置づけ明確化 |
