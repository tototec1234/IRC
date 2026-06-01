# B層実装リーダー

> 対象: B層（Protocol / Command）実装者 — 他層 API 参照用も含む  
> 作成日: 2026-06-01  
> 作成者: torinoue  
> 更新日: 2026-06-01  
> 更新者: torinoue  
> **役割**: 実装時の**読み物**（SSOT ではない）。Processing Flow、依存方針、呼び出し元、Open Questions 等  
> **公開 API の正**: [`diagrams/class_overview_diagram.md`](./diagrams/class_overview_diagram.md)  
> **契約憲章の正**: [`interface.md`](./interface.md)

## 1. Purpose

本ドキュメントは SSOT ではない。B 層実装者を主読者とし、他層 API を呼び出す際の参照用も兼ねる読み物である。

- 担当者間の依存関係と Processing Flow
- Network / IO層と Protocol / Command 層の接続方法
- 各 API の呼び出し元・利用コンテキスト
- 未決事項（Open Questions）

公開 API のメソッド名・シグネチャは [`class_overview_diagram.md`](./diagrams/class_overview_diagram.md) を正とする。  
層間契約の理由・禁止事項・設計決定は [`interface.md`](./interface.md)（契約憲章）を正とする。

全体設計・責務分割は `dev_docs/design.md` に記載する。
実装状況・変更予定・未実装項目は `dev_docs/roadmap.md` に記載する。

---

## 2. Dependency Policy

### 2.1 基本方針

本プロジェクトでは、担当者間の依存を最小化する。

特に重要な方針は以下。

- A: Network / IO層は、IRCコマンドの意味を知らない。
- B: Protocol / Command層は、Network / IO層の内部実装を知らない。
- Bは `Server`, `Connection`, `Poller`, `ConnectionManager` に依存しない。
- Bは送信処理を直接行わない。
- Bはコマンド処理結果を `CommandResult` として返す。
- A / Server は `CommandResult` を受け取り、対象fdのsend bufferへ積む。
- C1はClientとServerStateを管理する。
- C2はChannelとChannelModesを管理する。
- ChannelはClientを所有しない。`Client*` を参照するだけ。
- Clientの生成・削除はC1が担当する。

---

## 3. Processing Flow

基本的な処理フローは以下。

```text
A / Server
  poll() でfdイベントを受け取る
    ↓
A / Connection
  recv bufferから complete line を取り出す
    ↓
B / Parser
  line を Message に変換する
    ↓
B / CommandDispatcher
  Message と ServerState を使ってコマンド処理する
    ↓
B / CommandDispatcher
  CommandResult を返す
    ↓
A / Server
  CommandResult 内の OutgoingMessage を対象fdのsend bufferへ積む
    ↓
A / Connection
  POLLOUTでsend bufferを送信する
```

---

## 4. Common Interface Types

### 4.1 `OutgoingMessage`

> 公開 API・構造: [class_overview_diagram.md — **`CommandResult`** / **`OutgoingMessage`**](./diagrams/class_overview_diagram.md)[^fn-api-ref]

送信対象fdと送信文字列を表す。

```cpp
struct OutgoingMessage {
    int         fd;
    std::string message;

    OutgoingMessage(int targetFd, const std::string& text);
};
```

#### 役割

* Bが「誰に何を送るか」を表現する。
* 実際の `send()` は行わない。
* A / Server がこの情報を見てsend bufferへ積む。

---

### 4.2 `CommandResult`

> 公開 API・構造: [class_overview_diagram.md — **`CommandResult`**](./diagrams/class_overview_diagram.md)[^fn-api-ref]

1つのIRCコマンド処理結果を表す。

```cpp
struct CommandResult {
    std::vector<OutgoingMessage> replies;
    bool shouldDisconnect;

    CommandResult();

    void addReply(int fd, const std::string& message);
};
```

#### 役割

* コマンド処理後に送るべきメッセージを保持する。
* 必要に応じて切断要求を表す。
* BからAへ返される。
* BがAの `Connection` や `Server` を直接触らないための境界になる。

---

## 5. A: Network / IO Interface

担当: A

Aはfd、socket、poll、recv buffer、send bufferを管理する。

AはIRCコマンドの意味を知らない。

### 5.1 `Server`

> 公開 API（メソッド名・シグネチャ）: [class_overview_diagram.md — **`Server`**](./diagrams/class_overview_diagram.md)[^fn-api-ref]

#### 呼び出し元・利用コンテキスト

| メソッド | 利用者 | 内容 |
| -------- | ------ | ---- |
| `run()` | `main` | サーバのメインループを開始する |
| `sendTo()` | `Server`内部 | 指定fdのsend bufferへ送信文字列を積む |
| `applyCommandResult()` | `Server`内部 | `CommandResult` を処理し、返信をsend bufferへ積む |
| `disconnectClient()` | `Server`内部 | 対象fdの接続を安全に切断する |

#### 補足

`CommandDispatcher` は `Server` を直接呼ばない。
`Server` が `CommandResult` を受け取り、`sendTo()` を呼ぶ。

---

### 5.2 `Connection`

> 公開 API（メソッド名・シグネチャ）: [class_overview_diagram.md — **`Connection`**](./diagrams/class_overview_diagram.md)[^fn-api-ref]

#### 呼び出し元・利用コンテキスト

| メソッド | 利用者 | 内容 |
| -------- | ------ | ---- |
| `getFd()` | `Server` | Connectionが持つfdを返す |
| `readFromSocket()` | `Server` | `recv()` してrecv bufferへ追加する。切断・致命エラーならfalse |
| `writeToSocket()` | `Server` | send bufferから送れる分だけ `send()` する。切断・致命エラーならfalse |
| `hasCompleteLine()` | `Server` | `\r\n` 単位の完全な1行があるか確認する |
| `popLine()` | `Server` | recv bufferから1行取り出す |
| `bufferSend()` | `Server` | send bufferへ送信文字列を追加する |
| `hasPendingOutput()` | `Server` | send bufferに未送信データがあるか確認する |

#### 補足

`Connection` はIRCコマンドの意味を知らない。

やらないこと:

* `PASS` 判定
* `NICK` 判定
* `JOIN` 処理
* `PRIVMSG` 配送先判定
* `MODE` 解釈

---

### 5.3 `Poller` optional

`Poller` は必要に応じて `Server` から分離する。

> 公開 API（メソッド名・シグネチャ）: optional クラス — [class_overview_diagram.md — +optional の根拠](./diagrams/class_overview_diagram.md#optional-の根拠)[^fn-api-ref]

#### 呼び出し元・利用コンテキスト

| メソッド | 利用者 | 内容 |
| -------- | ------ | ---- |
| `add()` | `Server` | fdをpoll監視対象に追加する |
| `remove()` | `Server` | fdをpoll監視対象から削除する |
| `enableWrite()` | `Server` | `POLLOUT` 監視を有効化する |
| `disableWrite()` | `Server` | `POLLOUT` 監視を無効化する |
| `wait()` | `Server` | `poll()` を呼ぶ |
| `fds()` | `Server` | pollfd一覧を参照する |

#### 重要ルール

実際に `poll()` を呼ぶ場所は1箇所に限定する。

`Poller` を分離する場合も、`Server` から `Poller::wait()` を呼ぶ構造にする。
各 `Connection` が個別に `poll()` / `select()` を呼ぶことは禁止する。

---

### 5.4 `ConnectionManager` optional

`ConnectionManager` は必要に応じて `Server` から分離する。

> 公開 API（メソッド名・シグネチャ）: optional クラス — [class_overview_diagram.md — +optional の根拠](./diagrams/class_overview_diagram.md#optional-の根拠)[^fn-api-ref]

#### 呼び出し元・利用コンテキスト

| メソッド | 利用者 | 内容 |
| -------- | ------ | ---- |
| `addConnection()` | `Server` | fdからConnectionを作成する |
| `removeConnection()` | `Server` | Connectionを削除する |
| `getConnection()` | `Server` | fdからConnectionを取得する |
| `hasConnection()` | `Server` | 指定fdのConnectionが存在するか確認する |
| `sendTo()` | `Server` | 指定fdのConnectionのsend bufferへ積む |
| `clear()` | `Server` | 全Connectionを解放する |

#### 補足

Bは `ConnectionManager` を直接呼ばない。
Bは `CommandResult` を返し、A / Server が送信処理を行う。

---

## 6. B: Protocol / Command Interface

担当: B

BはIRCメッセージの解析、コマンド振り分け、返信生成を担当する。

BはAのNetwork / IOクラスに依存しない。

---

### 6.1 `Message`

> 公開 API（メソッド名・シグネチャ）: [class_overview_diagram.md — **`Message`**](./diagrams/class_overview_diagram.md)[^fn-api-ref]

#### 呼び出し元・利用コンテキスト

| メソッド | 利用者 | 内容 |
| -------- | ------ | ---- |
| `getCommand()` | `CommandDispatcher` | command名を取得する |
| `getParams()` | `CommandDispatcher` | parameter一覧を取得する |
| `getParamCount()` | `CommandDispatcher` | parameter数を取得する |
| `getSingleParam()` | `CommandDispatcher` | 指定indexのparameterを取得する |
| `hasParam()` | `CommandDispatcher` | 指定indexのparameterが存在するか |

#### 想定構造

```cpp
class Message {
private:
    std::string              _command;
    std::vector<std::string> _params;
};
```

#### 補足

`trailing` は `_params` の最後の要素として扱う。

例:

```text
PRIVMSG #room :hello world
```

```text
command = "PRIVMSG"
params  = ["#room", "hello world"]
```

---

### 6.2 `Parser`

> 公開 API（メソッド名・シグネチャ）: [class_overview_diagram.md — **`Parser`**](./diagrams/class_overview_diagram.md)[^fn-api-ref]

#### 呼び出し元・利用コンテキスト

| メソッド | 利用者 | 内容 |
| -------- | ------ | ---- |
| `parse()` | `Server` | 1行文字列をMessageへ変換する |

#### 補足

`Parser` はrecv bufferを扱わない。
recv bufferからcomplete lineを切り出すのはA / Connectionの責務。

---

### 6.3 `CommandDispatcher`

> 公開 API（メソッド名・シグネチャ）: [class_overview_diagram.md — **`CommandDispatcher`**](./diagrams/class_overview_diagram.md)[^fn-api-ref]

#### 呼び出し元・利用コンテキスト

| メソッド | 利用者 | 内容 |
| -------- | ------ | ---- |
| `dispatch()` | `Server` | IRC commandを実行し、結果を返す |

#### 補足

`dispatch()` は送信処理を直接行わない。

やらないこと:

* `send()`
* `sendTo()`
* `Connection` 操作
* `Poller` 操作
* `Server` 操作

やること:

* `Message` のcommandを見る
* `ServerState` から `Client` / `Channel` を取得する
* 必要に応じて状態を更新する
* `ReplyBuilder` で返信文字列を作る
* `CommandResult` に送信対象と文字列を詰める

---

### 6.4 `ReplyBuilder`

> 公開 API（メソッド名・シグネチャ）: [class_overview_diagram.md — **`ReplyBuilder`**](./diagrams/class_overview_diagram.md)[^fn-api-ref]

#### 呼び出し元・利用コンテキスト

| メソッド | 利用者 | 内容 |
| -------- | ------ | ---- |
| `welcome()` | `CommandDispatcher` | `001` welcomeを生成する |
| `needMoreParams()` | `CommandDispatcher` | `461` を生成する |
| `alreadyRegistered()` | `CommandDispatcher` | `462` を生成する |
| `passwordMismatch()` | `CommandDispatcher` | `464` を生成する |
| `nickInUse()` | `CommandDispatcher` | `433` を生成する |
| `noSuchNick()` | `CommandDispatcher` | `401` を生成する |
| `noSuchChannel()` | `CommandDispatcher` | `403` を生成する |
| `userNotInChannel()` | `CommandDispatcher` | `441` を生成する |
| `notOnChannel()` | `CommandDispatcher` | `442` を生成する |
| `notRegistered()` | `CommandDispatcher` | `451` を生成する |
| `chanOpPrivsNeeded()` | `CommandDispatcher` | `482` を生成する |
| `channelIsFull()` | `CommandDispatcher` | `471` を生成する |
| `inviteOnlyChan()` | `CommandDispatcher` | `473` を生成する |
| `badChannelKey()` | `CommandDispatcher` | `475` を生成する |
| `join()` | `CommandDispatcher` | JOIN通知を生成する |
| `privmsg()` | `CommandDispatcher` | PRIVMSG配送文を生成する |
| `kick()` | `CommandDispatcher` | KICK通知を生成する |
| `invite()` | `CommandDispatcher` | INVITE通知を生成する |
| `topic()` | `CommandDispatcher` | TOPIC通知を生成する |
| `topicReply()` | `CommandDispatcher` | `332` topic表示を生成する |
| `noTopic()` | `CommandDispatcher` | `331` topic未設定を生成する |
| `mode()` | `CommandDispatcher` | MODE通知を生成する |
| `channelModeIs()` | `CommandDispatcher` | `324` mode照会を生成する |

---

## 7. C1: Client / ServerState Interface

担当: C1

C1はClientの状態、Clientの生死、サーバ全体の辞書を管理する。

---

### 7.1 `Client`

> 公開 API（メソッド名・シグネチャ）: [class_overview_diagram.md — **`Client`**](./diagrams/class_overview_diagram.md)[^fn-api-ref]

#### 呼び出し元・利用コンテキスト

| メソッド | 利用者 | 内容 |
| -------- | ------ | ---- |
| `getFd()` | B / C2 | Clientに対応するfdを取得する |
| `getNick()` | B / C2 | nickを取得する |
| `getUsername()` | B | usernameを取得する |
| `getRealname()` | B | realnameを取得する |
| `getHost()` | B | hostを取得する |
| `getFullPrefix()` | B | `nick!user@host` 形式を返す（RFC 1459 Section 2.3 準拠） |
| `setUsername()` | B | usernameを設定する |
| `setRealname()` | B | realnameを設定する |
| `setHost()` | A | hostを設定する（接続時） |
| `setPassOk()` | B | PASS成功状態を設定する |
| `isPassOk()` | B | PASS済みか確認する |
| `isRegistered()` | B | 登録完了済みか確認する |
| `canRegister()` | B | PASS / NICK / USER が揃ったか確認する |
| `markRegistered()` | B | 登録完了状態にする |

#### 注意

`Client::setNick()` は外部から直接呼ばない。

nick変更は必ず `ServerState::updateNick()` を通す。

#### getFullPrefix() について

サーバーがクライアントにメッセージを送る際、送信元を `nick!user@host` 形式で付与する必要がある（RFC 1459 Section 2.3）。詳細は `dev_docs/rfc1459_prefix_analysis.md` 参照。

---

### 7.2 `ServerState`

> 公開 API（メソッド名・シグネチャ）: [class_overview_diagram.md — **`ServerState`**](./diagrams/class_overview_diagram.md)[^fn-api-ref]

#### 呼び出し元・利用コンテキスト

| メソッド | 利用者 | 内容 |
| -------- | ------ | ---- |
| `password()` | B | サーバpasswordを取得する |
| `addClient()` | Server | 新規接続時にClientを作成する |
| `removeClient()` | Server / B | 切断時にClientを削除する（Channel掃除・delete含む） |
| `getClientByFd()` | B | fdからClientを取得する |
| `getClientByNick()` | B | nickからClientを取得する |
| `nickExists()` | B | nick重複を確認する |
| `updateNick()` | B | Clientのnickとnick辞書を同時に更新する |
| `getChannel()` | B | Channelを取得する。なければNULL |
| `getOrCreateChannel()` | B | Channelを取得する。なければ作成 |
| `removeChannelIfEmpty()` | B / Server | 空Channelを削除する |

#### 削除ルール

`removeClient(int fd)` はClientを削除する前に、**private** メソッド（例: `removeClientFromAllChannels(Client&)`）で以下を削除する必要がある。B 層は `removeClient(fd)` のみ呼び、`removeClientFromAllChannels` は直接呼ばない（`decision_invite_and_removal.md` 参照）。

削除対象:

* Channel member
* Channel operator
* invited list

その後、空になったChannelを削除し、`fd -> Client` と `nick -> Client` の辞書を更新する。

---

### 7.3 `ClientRegistry` optional

`ClientRegistry` は `ServerState` が肥大化した場合に分離する。

初期実装では必須ではない。

> 公開 API（メソッド名・シグネチャ）: optional クラス — [class_overview_diagram.md — +optional の根拠](./diagrams/class_overview_diagram.md#optional-の根拠)[^fn-api-ref]

#### 呼び出し元・利用コンテキスト

| メソッド | 利用者 | 内容 |
| -------- | ------ | ---- |
| `add()` | `ServerState` | Clientを追加する |
| `remove()` | `ServerState` | Clientを削除する |
| `findByFd()` | `ServerState` | fdからClientを取得する |
| `findByNick()` | `ServerState` | nickからClientを取得する |
| `nickExists()` | `ServerState` | nick重複を確認する |
| `updateNick()` | `ServerState` | nick辞書を更新する |

---

## 8. C2: Channel Interface

担当: C2

C2はChannel内部状態とChannelModesを管理する。

---

### 8.1 `Channel`

> 公開 API（メソッド名・シグネチャ）: [class_overview_diagram.md — **`Channel`**](./diagrams/class_overview_diagram.md)[^fn-api-ref]

#### 呼び出し元・利用コンテキスト

| メソッド | 利用者 | 内容 |
| -------- | ------ | ---- |
| `name()` | B / C1 | channel名を取得する |
| `hasMember()` | B | Clientが参加済みか確認する |
| `addMember()` | B | memberを追加する |
| `removeMember()` | B | memberを削除する |
| `removeClient()` | B / C1 | member / operator / invited list からまとめて削除する |
| `members()` | B | member一覧を取得する |
| `memberCount()` | B | member数を取得する |
| `isOperator()` | B | Clientがchannel operatorか確認する |
| `addOperator()` | B | operator権限を付与する |
| `removeOperator()` | B | operator権限を剥奪する |
| `addInvite()` | B | 招待リストに追加する |
| `isInvited()` | B | 招待済みか確認する |
| `removeInvite()` | B | 招待状態を解除する |
| `setTopic()` | B | topicを設定する |
| `topic()` | B | topicを取得する |
| `modes()` | B | mode変更用 |
| `modes() const` | B | mode参照用 |
| `isEmpty()` | B / C1 | memberが0人か確認する |

#### 補足

`Channel` は `Client` を所有しない。
`Channel` は `Client*` を保持するだけで、`delete` はしない。

#### JOIN時のoperator初期化

新規作成されたChannelに最初のClientが参加する場合、参加処理は `addMember()` に加えて `addOperator()` も行う。

この判定は `CommandDispatcher` または `ChannelService::join()` が行う。

---

### 8.2 `ChannelModes`

> 公開 API（メソッド名・シグネチャ）: [class_overview_diagram.md — **`ChannelModes`**](./diagrams/class_overview_diagram.md)[^fn-api-ref]

#### 呼び出し元・利用コンテキスト

| メソッド | 利用者 | 内容 |
| -------- | ------ | ---- |
| `inviteOnly()` | B | `+i` 状態を取得する |
| `topicRestricted()` | B | `+t` 状態を取得する |
| `hasKey()` | B | `+k` 状態を取得する |
| `key()` | B | channel keyを取得する |
| `limit()` | B | user limitを取得する。無制限なら `-1` |
| `setInviteOnly()` | B | `+i` / `-i` を設定する |
| `setTopicRestricted()` | B | `+t` / `-t` を設定する |
| `setKey()` | B | `+k` を設定する |
| `unsetKey()` | B | `-k` を設定する |
| `setLimit()` | B | `+l` を設定する |
| `unsetLimit()` | B | `-l` を設定する |

---

### 8.3 `ChannelService` optional

`ChannelService` は `CommandDispatcher` が肥大化した場合に分離する。

初期実装では必須ではない。

> 公開 API（メソッド名・シグネチャ）: optional クラス — [class_overview_diagram.md — +optional の根拠](./diagrams/class_overview_diagram.md#optional-の根拠)[^fn-api-ref]

#### 呼び出し元・利用コンテキスト

| メソッド | 利用者 | 内容 |
| -------- | ------ | ---- |
| `canJoin()` | B | JOIN可能か判定する |
| `join()` | B | ClientをChannelに参加させ、最初のmemberならoperatorにする |
| `canKick()` | B | KICK権限があるか判定する |
| `kick()` | B | ClientをChannelから外す |
| `canInvite()` | B | INVITE権限があるか判定する |
| `invite()` | B | Clientを招待する |
| `canChangeTopic()` | B | TOPIC変更権限があるか判定する |
| `setTopic()` | B | topicを変更する |
| `canChangeMode()` | B | MODE変更権限があるか判定する |
| `isFull()` | B | user limitに達しているか判定する |
| `currentModeLine()` | B | MODE照会用のmode文字列を生成する |


---

## 9. Important Rules

> 契約ルールの正: [`interface.md`](./interface.md)[^fn-rules-ssot]

### 9.1 B does not depend on A

`CommandDispatcher` は以下をincludeしない。

* `Server.hpp`
* `Connection.hpp`
* `Poller.hpp`
* `ConnectionManager.hpp`

Bは `CommandResult` を返すだけにする。

---

### 9.2 A applies CommandResult

A / Server は `CommandResult` を受け取って送信処理を行う。

```cpp
CommandResult result = dispatcher.dispatch(fd, msg, state);
applyCommandResult(result);
```

`applyCommandResult()` は、`result.replies` を見て各fdのsend bufferへ積む。

---

### 9.3 Nick update

nick変更は必ず `ServerState::updateNick()` を通す。

NG:

```cpp
client.setNick(newNick);
```

OK:

```cpp
state.updateNick(client, newNick);
```

---

### 9.4 Client ownership

Clientの生成・削除はC1が担当する。

* `ServerState::addClient(fd)` で作成する。
* `ServerState::removeClient(fd)` で削除する。
* `Channel` は `Client*` を参照するだけ。
* `Channel` は `Client*` を `delete` しない。

---

### 9.5 Channel operator

operator権限は `Client` ではなく `Channel` が管理する。

NG:

```cpp
client.setOperator(true);
```

OK:

```cpp
channel.addOperator(&client);
```

理由:

1人のClientは複数Channelに参加でき、Channelごとにoperator権限が異なるため。

---

### 9.6 POLLOUT

送信すべきデータがあるfdだけ `POLLOUT` を有効にする。

* send bufferが空でない場合、`POLLOUT` を有効にする。
* send bufferが空になった場合、`POLLOUT` を無効にする。
* 常時 `POLLOUT` を監視し続けない。

---

### 9.7 Client removal cleanup

`ServerState::removeClient(fd)` はClientを削除する前に、全Channelから該当Clientへの参照を除去する。

削除対象:

* member
* operator
* invited list

`Channel` は `Client*` を所有しないため、Client削除後にChannel内へ `Client*` を残してはいけない。

---

### 9.8 TOPIC / MODE read operations

`TOPIC #channel` と `MODE #channel` は状態変更ではなく照会として扱う。

* `TOPIC #channel` はtopicがあれば `topicReply()`、なければ `noTopic()` を返す。
* `MODE #channel` は `currentModeLine()` で現在のmode文字列を作り、`channelModeIs()` を返す。

---

## 10. Open Questions

以下は実装しながら確定する。

### 10.1 `Poller` を最初から分離するか

現時点では optional。

Phase 4 初期実装では `Server` 内で管理してもよい。

肥大化した場合に `Poller` へ分離する。

---

### 10.2 `ConnectionManager` を最初から分離するか

現時点では optional。

まずは `Server` が `fd -> Connection` を直接管理してもよい。

send buffer管理が膨らむ場合、`ConnectionManager` へ分離する。

---

### 10.3 `ClientRegistry` を分離するか

現時点では optional。

まずは `ServerState` が `fd -> Client` と `nick -> Client` を直接持つ。

肥大化した場合に `ClientRegistry` へ分離する。

---

### 10.4 `ChannelService` を分離するか

現時点では optional。

まずは `CommandDispatcher` が `Channel` / `ChannelModes` を直接操作する。

肥大化した場合に `ChannelService` へ分離する。

---

## 脚注

[^fn-api-ref]: メソッド名・シグネチャの正は [`class_overview_diagram.md`](./diagrams/class_overview_diagram.md)。下表のメソッド名は同図と一致させること。API 変更時は class_overview を先に更新し、本ガイド・[`interface.md`](./interface.md)・onboarding 等の参照（ファイル名・§番号・リンク）も合わせて更新すること。

[^fn-rules-ssot]: §9 の契約ルールの正は [`interface.md`](./interface.md)（契約憲章）。本ガイド §9 は B 層実装者向けの補足説明である。
