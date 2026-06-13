# 層間インターフェース仕様

> **SSOT（契約憲章）**: 層間契約の**理由・ルール・設計決定**を定める。経緯と「なぜそうなったか」を読むための憲章的ドキュメント。  
> **公開 API のSSOT**: [`diagrams/class_overview_diagram.md`](./diagrams/class_overview_diagram.md)  
> **実装読み物（B層主読者）**: [`b_implementation_reader.md`](./b_implementation_reader.md) — SSOT ではない  
> 各層の責務は [`design.md`](./design.md) を参照。

本ドキュメントは層間 API の「何を」ではなく「なぜ・どう守るか」を定める憲章である。  
公開 API のメソッド名・シグネチャは [`class_overview_diagram.md`](./diagrams/class_overview_diagram.md) を正とする。  
Processing Flow や各 API の呼び出し元・利用コンテキストは [`b_implementation_reader.md`](./b_implementation_reader.md) を参照する（本書は SSOT ではない）。

## スコープ

```
A層 ─complete line→ B層 ─CommandResult→ A層
                     │
                     ↓
                   C層
   (ServerState facade / Client, Channel, ChannelModes)
```

| 境界 | 方向 | 内容 |
|------|------|------|
| A→B | 入力 | complete line（`\r\n` 区切り文字列） |
| B→A | 出力 | `CommandResult`（送信先fd + 送信文字列） |
| B↔C | 双方向 | `ServerState`, `Client`, `Channel`, `ChannelModes` 操作API |
| A→C | 接続 lifecycle | accept / disconnect 時に `ServerState` facade を呼び、Client 作成・削除を行う |

---

## チーム構成

| 担当 | メンバー | 範囲 |
|------|---------|------|
| A | torinoue | Network / IO（Server, Connection） |
| B | torinoue | Protocol / Command（Parser, Dispatcher, ReplyBuilder） |
| C | tyamaoka | ServerState / ClientRegistry / Client / Channel / ChannelModes |

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

## 4. C層インターフェース

C層は IRC 上の状態を管理する。

現在の実装では、数字付きの C1 / C2 という分類よりも、以下の責務で見る。

| 区分 | クラス | 責務 |
|------|--------|------|
| Facade / ownership | `ServerState` | B層向け窓口、Client / Channel 所有、辞書、Client-Channel 関係同期、cleanup |
| Registry internal | `ClientRegistry` | fd / nick から Client を引く内部実装。B層は直接触らない |
| Entity | `Client` | IRC user 状態、登録状態、所属 channel cache |
| Entity | `Channel` | channel 内部状態、member/operator/invite/topic/modes |
| Value-like state | `ChannelModes` | `+i`, `+t`, `+k`, `+l` の状態 |

B層は `ServerState` を C層の主な窓口として使う。  
また、A層は接続 lifecycle のために `ServerState` facade を呼ぶ。Client 登録時の host 情報は A層だけが知るため、accept 時に A層が fd と接続元 host を渡して `ServerState::addClient(fd, host)` を呼ぶ。
ただし、B層は reply / protocol 判断のために `Client`, `Channel`, `ChannelModes` の公開 getter / 局所状態 API を参照・操作してよい。

Client と Channel の関係を作る/壊す操作は `ServerState` 経由で行う。

### 4.1 ServerState

| 関数 | 戻り値 | 呼び出し元 | 説明 |
|------|--------|-----------|------|
| `const std::string& getPassword() const` | `const std::string&` | B | サーバパスワード |
| `void addClient(int fd, const std::string& host)` | `void` | A / Server | accept 時に fd と接続元 host を渡して Client 作成 |
| `Channel* addClientToChannel(Client*, const std::string&)` | `Channel*` | B | Client と Channel の参加関係を同期し、必要なら Channel 作成 |
| `void removeClientFromChannel(Client*, const std::string&)` | `void` | B | Client と Channel の参加関係を解除し、空 Channel を削除 |
| `void inviteClientToChannel(Client*, Channel*)` | `void` | B | invite list に Client を追加する C層窓口 |
| `void removeInviteFromChannel(Client*, Channel*)` | `void` | B | invite list から Client を削除する C層窓口 |
| `void removeClientFromAllInvites(Client*)` | `void` | ServerState / cleanup | 全 Channel の invite list から Client を削除 |
| `void removeClient(int fd)` | `void` | A / Server, B | Client削除（Channel参照、invite、辞書を cleanup） |
| `Client* getClientByFd(int fd)` | `Client*` | B | fdからClient取得 |
| `Client* getClientByNick(const std::string&)` | `Client*` | B | nickからClient取得 |
| `bool nickExists(const std::string&) const` | `bool` | B | nick重複確認 |
| `bool updateNick(Client&, const std::string&)` | `bool` | B | nick変更（辞書とClient cacheを同期。重複時false） |
| `Channel* getChannel(const std::string&)` | `Channel*` | B | Channel取得（なければNULL） |
| `Channel* getOrCreateChannel(const std::string&)` | `Channel*` | B / ServerState | Channel取得or作成 |
| `void removeChannelIfEmpty(const std::string&)` | `void` | B / ServerState | 空Channel削除 |

### 4.2 Client

| 関数 | 戻り値 | 呼び出し元 | 説明 |
|------|--------|-----------|------|
| `int getFd() const` | `int` | B, Channel/ReplyBuilder | fdを取得 |
| `const std::string& getNick() const` | `const std::string&` | B, Channel/ReplyBuilder | nickを取得 |
| `const std::string& getUsername() const` | `const std::string&` | B | usernameを取得 |
| `const std::string& getRealname() const` | `const std::string&` | B | realnameを取得 |
| `const std::string& getHost() const` | `const std::string&` | B | hostを取得 |
| `std::vector<Channel*> getChannels() const` | `std::vector<Channel*>` | B, ServerState | 所属Channel一覧 |
| `std::string getFullPrefix() const` | `std::string` | B | `nick!user@host` を返す |
| `void setUsername(const std::string&)` | `void` | B | username設定 |
| `void setRealname(const std::string&)` | `void` | B | realname設定 |
| `void setHost(const std::string&)` | `void` | C / internal | host再設定用。接続時の初期化は原則 `ServerState::addClient(fd, host)` 経由 |
| `void _unsafe_setNick(const std::string&)` | `void` | ServerState / ClientRegistry | nick cache 更新用。B層は直接呼ばない |
| `void _unsafe_joinChannel(Channel*)` | `void` | ServerState | 所属Channel cache追加。B層は直接呼ばない |
| `void _unsafe_leaveChannel(Channel*)` | `void` | ServerState | 所属Channel cache削除。B層は直接呼ばない |
| `void setPassOk(bool)` | `void` | B | PASS成功状態設定 |
| `bool isPassOk() const` | `bool` | B | PASS済みか |
| `bool isRegistered() const` | `bool` | B | 登録完了か |
| `bool canRegister() const` | `bool` | B | 登録可能か（PASS/NICK/USER揃った） |
| `void markRegistered()` | `void` | B | 登録完了にする |

**⚠️ 注意**: `_unsafe_setNick()` は外部から直接呼ばない。`ServerState::updateNick()` を使う。

**getFullPrefix()** は RFC 1459 Section 2.3 に基づき、サーバー→クライアントのメッセージに必要。詳細は `rfc1459_prefix_analysis.md` 参照。

### 4.3 Channel

| 関数 | 戻り値 | 呼び出し元 | 説明 |
|------|--------|-----------|------|
| `const std::string& getName() const` | `const std::string&` | B, ServerState | channel名 |
| `const std::string& getTopic() const` | `const std::string&` | B | topic取得 |
| `void setTopic(const std::string&)` | `void` | B | topic設定 |
| `bool hasMember(Client*) const` | `bool` | B | 参加済みか |
| `void _unsafe_addMember(Client*)` | `void` | ServerState | member追加。B層は直接呼ばない |
| `void _unsafe_removeMember(Client*)` | `void` | ServerState | member削除。B層は直接呼ばない |
| `void _unsafe_removeClientState(Client*)` | `void` | ServerState | Channel内部のmember/operator/invite cleanup。B層は直接呼ばない |
| `std::vector<Client*> getMembers() const` | `std::vector<Client*>` | B | member一覧 |
| `size_t memberCount() const` | `size_t` | B | member数 |
| `bool isOperator(Client*) const` | `bool` | B | operator権限確認 |
| `void setOperator(Client*, bool)` | `void` | B | operator権限の付与・剥奪 |
| `void addInvite(Client*)` | `void` | ServerState | invite list 追加。B層はServerState経由 |
| `bool isInvited(Client*) const` | `bool` | B | 招待済みか |
| `void removeInvite(Client*)` | `void` | ServerState | invite list 削除。B層はServerState経由 |
| `ChannelModes& getModes()` | `ChannelModes&` | B | mode変更用 |
| `const ChannelModes& getModes() const` | `const ChannelModes&` | B | mode参照用 |
| `bool isEmpty() const` | `bool` | B, ServerState | member 0人か |

### 4.4 ChannelModes

| 関数 | 戻り値 | 呼び出し元 | 説明 |
|------|--------|-----------|------|
| `bool isInviteOnly() const` | `bool` | B | +i状態 |
| `bool isTopicRestricted() const` | `bool` | B | +t状態 |
| `bool hasKey() const` | `bool` | B | +k状態 |
| `std::string getKey() const` | `std::string` | B | パスワード |
| `int getLimit() const` | `int` | B | 人数制限（-1=無制限） |
| `void setInviteOnly(bool)` | `void` | B | +i/-i設定 |
| `void setTopicRestricted(bool)` | `void` | B | +t/-t設定 |
| `void setKey(const std::string&)` | `void` | B | +k設定 |
| `void unSetKey()` | `void` | B | -k設定 |
| `void setLimit(int)` | `void` | B | +l設定 |
| `void unSetLimit()` | `void` | B | -l設定 |

### 4.5 ClientRegistry

`ClientRegistry` は `ServerState` の内部実装である。  
B層は `ClientRegistry` を直接触らず、`ServerState` の facade API を使う。

---

## 5. 重要ルール

### 5.1 【設計】Client登録とhost初期化は ServerState 経由

```cpp
// ❌ NG
state.addClient(fd, "");
client.setHost(host);

// ✅ OK
state.addClient(fd, host);
```

Client 登録時の host 情報は A層だけが知る。A層は accept 時に接続元 host を取得し、`ServerState::addClient(fd, host)` で Client 作成と host 初期化を同時に行う。

### 5.2 【実装】nick変更は ServerState 経由

```cpp
// ❌ NG
client._unsafe_setNick("newNick");

// ✅ OK
state.updateNick(client, "newNick");
```

### 5.3 【設計】Client-Channel関係の変更は ServerState 経由

```cpp
// ❌ NG
channel._unsafe_addMember(&client);
client._unsafe_joinChannel(&channel);

// ✅ OK
state.addClientToChannel(&client, "#channel");
```

JOIN / PART / KICK / QUIT / disconnect によって Client と Channel の関係を作る・壊す処理は `ServerState` が同期する。

### 5.4 【設計】Client削除は ServerState 経由

```cpp
// ServerState::removeClient(fd) が自動処理:
// - 全Channelからmember/operator/invited削除
// - fd辞書・nick辞書の更新
// - 空Channelの削除
```

### 5.5 【設計】operator権限は Channel が管理

```cpp
// ❌ NG
client.setOperator(true);

// ✅ OK
channel.setOperator(&client, true);
```

### 5.6 【設計】Channel は Client を所有しない

- `Channel` は `Client*` を保持するだけ
- `delete` は `ServerState` の責務

### 5.7 【設計】B層は送信処理を行わない

- `CommandDispatcher` は `send()` を呼ばない
- 結果は `CommandResult` に詰めて返す
- A層が `CommandResult` を適用して送信

---

## 6. 未確定事項（Open Questions）

| 項目 | 現状 | 決定タイミング |
|------|------|---------------|
| Poller分離 | optional | 実装時判断 |
| ConnectionManager分離 | optional | 実装時判断 |
| ChannelService分離 | optional | 実装時判断 |

### 6.1 確定済み（設計決定）

| 項目 | 決定 | 参照 |
|------|------|------|
| 自作 template | 不使用 | `decision_no_custom_templates.md` |
| エラー・所有権 | 起動時例外 / ループは bool+Result / ServerState 所有 | `decision_error_handling.md` |
| `removeClientFromAllChannels` | ServerState **private**。B は `removeClient(fd)` のみ | `decision_invite_and_removal.md` |
| ClientRegistry | `ServerState` 内部実装として分離済み。B は直接触らない | `knowledge/facade_delegation_update_nick.md` |
| invite 管理 | `ServerState` 経由。通知と招待券は別概念 | `decision_invite_responsibility.md`, `knowledge/invite_ticket_policy.md` |
| invite 系命名 | `Channel::addInvite` + `ReplyBuilder::invite`（B）共存 | `decision_invite_and_removal.md` |

---

## 7. 関連ドキュメント

| ドキュメント | 内容 |
|-------------|------|
| `design.md` | 全体設計・責務分割 |
| `workflow.md` | SSOT、実装、テストの更新手順 |
| `b_implementation_reader.md` | B層主読者向け実装読み物（SSOT ではない。Processing Flow、呼び出し元等） |
| `diagrams/class_overview_diagram.md` | 公開 API・クラス関係（SSOT） |
| `onboarding_A.md` | A層オンボーディング |
| `onboarding_B.md` | B層オンボーディング |
| `onboarding_C1.md` / `onboarding_C2.md` | 旧C1/C2分類に基づくオンボーディング。現実装ではC層全体の参考資料 |
| `rfc1459_prefix_analysis.md` | getFullPrefix() の根拠 |

---

## 変更履歴

| 日付 | 内容 |
|------|------|
| 2026-05-23 | 初版作成 |
| 2026-05-26 | 層間API契約書として再構成、外部リポジトリ依存削除 |
| 2026-06-01 | MTG決定: 契約憲章（SSOT）として位置づけ明確化 |
