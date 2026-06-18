# A 層 I/O フロー（recv → B/C → send 往復）

> 対象: `src/a/Server.{cpp,hpp}` / `src/a/Connection.{cpp,hpp}`
> 1 本の IRC メッセージが `nc`（クライアント）から入り、B 層で解釈され、C 層で状態を参照/更新し、A 層が送り返すまでの往復経路。
> 位置づけ: **レビュー用の補助資料**。粒度は揃っていなくてよく、レビュアーが「どの関数・どのクラス・どの層」を追えることを優先する。

## 図 A: フローチャート（関数・クラス・層を明示）

凡例（ノードの色）:
- 青 = `Server` のメソッド（poll ループ / 接続管理 / 振り分け）
- 緑 = `Connection` のメソッド（fd ごとの recv/send バッファ）
- 橙 = B 層（Parser / CommandDispatcher / ReplyBuilder）
- 紫 = C 層（ServerState / Client / Channel）

```mermaid
flowchart TD
    start([poll が revents を返す]):::server --> branch{"fd の種類 / revents で分岐<br/>【Server::run() の for ループ】"}:::server

    branch -->|"listen fd & POLLIN"| accept["Server::_acceptClient()<br/>accept → new Connection<br/>_connections 登録<br/>_state.addClient(fd, host)"]:::server
    accept --> loopback

    branch -->|"client fd & POLLIN"| read["Server::_handleRead(fd)"]:::server
    branch -->|"client fd & POLLOUT"| write["Server::_handleWrite(fd)"]:::server
    branch -->|"POLLERR / POLLHUP / POLLNVAL"| disc

    %% ===== 受信（_handleRead の中身）=====
    read --> recv["Connection::readFromSocket()<br/>recv → _recvBuffer に累積"]:::conn
    recv -->|"false（recv==0 / エラー）"| disc
    recv -->|true| hasline{"Connection::hasCompleteLine()<br/>(\\r\\n あり?)"}:::conn
    hasline -->|なし| loopback
    hasline -->|あり| pop["Connection::popLine() → line"]:::conn
    pop --> parse

    %% ===== 解釈(B) + 状態(C) =====
    parse["Parser::parse(line) → Message"]:::blayer --> dispatch["CommandDispatcher::dispatch(fd, msg, _state)"]:::blayer
    dispatch --> cstate["ServerState / Client / Channel<br/>参照・更新（getClientByFd, 登録, nick 等）"]:::clayer
    cstate --> result["CommandResult を返す<br/>{ replies[{fd, message}], shouldDisconnect }"]:::blayer

    %% ===== 送信準備（applyCommandResult の中身）=====
    result --> apply["Server::applyCommandResult(result)"]:::server
    apply --> foreach{"各 reply:<br/>_connections.find(target)"}:::server
    foreach -->|見つかる| buf["Connection::bufferSend(message)<br/>＋ Server::_enablePollout(target)"]:::conn
    foreach -->|見つからない| skip["スキップ（continue）"]:::server
    buf --> hasline
    skip --> hasline

    %% ===== 送信（_handleWrite の中身）=====
    write --> wsock["Connection::writeToSocket()<br/>send() で _sendBuffer 送出 → 送れた分を erase"]:::conn
    wsock -->|"false（send 失敗）"| disc
    wsock -->|true| drained{"Connection::hasPendingOutput()"}:::conn
    drained -->|空になった| dis["Server::_disablePollout(fd)"]:::server
    drained -->|残あり| loopback
    dis --> loopback

    %% ===== 切断 =====
    disc["Server::_disconnectClient(fd)<br/>close + _removeFd + delete Connection<br/>+ _state.removeClient(fd)"]:::server --> loopback
    loopback([次の poll へ]):::server --> start

    classDef server fill:#e3f2fd,stroke:#1976d2,color:#0d47a1;
    classDef conn fill:#e8f5e9,stroke:#388e3c,color:#1b5e20;
    classDef blayer fill:#fff3e0,stroke:#f57c00,color:#e65100;
    classDef clayer fill:#f3e5f5,stroke:#7b1fa2,color:#4a148c;
```

### 図 A の読み方（レビュー観点）

- **`Server::run()` の for ループ**が起点。`revents` で `_acceptClient` / `_handleRead` / `_handleWrite` / `_disconnectClient` に振り分ける（青）。
- **受信側**：`_handleRead`（青）が `Connection`（緑）の `readFromSocket → hasCompleteLine → popLine` を回し、取り出した行を B 層（橙）へ渡す。
- **解釈・状態**：B 層（橙）の `dispatch` が C 層（紫）を参照/更新し、`CommandResult` を A 層へ返す。
- **送信準備**：`applyCommandResult`（青）が reply ごとに送信先 `Connection`（緑）の `bufferSend` で積み、`_enablePollout` で POLLOUT を立てる。**ここでは送らない**。
- **送信**：別周回で POLLOUT が立つと `_handleWrite`（青）→ `Connection::writeToSocket`（緑）で `send()` 実行。送り切ったら `_disablePollout`。
- **責務の境界**：青(Server)=poll/接続管理/振り分け、緑(Connection)=fd ごとの recv/send バッファ、橙(B)=解釈、紫(C)=状態。A 層が B/C に依存するのは `Parser::parse` / `dispatch` / `CommandResult` の契約面のみ。

### 図 A の簡略化メモ（図には描かず実装にある挙動）

図はデータフローを優先しているため、以下は図に描いていない。実装（`Server::run()`）では考慮済みなので、図と食い違って見えても矛盾ではない。

1. **revents の判定順**：図では `POLLERR/POLLHUP/POLLNVAL → _disconnectClient` を独立分岐のように描いているが、実コードは 1 周回内で **(3) POLLIN → (3.5) POLLOUT → (4) HUP/ERR の順**に処理する。`POLLIN | POLLHUP` が同時に立つ fd は **先に読み切ってから**切断する（受信バッファに未読が残るのを防ぐ）。
2. **走査中 erase 対策**：`_disconnectClient` は `_pollfds` から要素を `erase` して配列が縮むため、`run()` の添字ループでは切断時に `--i` して添字を戻し、次要素のスキップを防いでいる。

## 図 B: シーケンス図（PING/PONG の時系列）

```mermaid
sequenceDiagram
    participant NC as nc (client)
    participant A as A層 (Server/Connection)
    participant B as B層 (Parser/Dispatcher/ReplyBuilder)
    participant C as C層 (ServerState/Client)

    NC->>A: "PING :foo\r\n" (TCP)
    Note over A: poll → POLLIN → _handleRead
    A->>A: readFromSocket() recv → _recvBuffer
    A->>A: hasCompleteLine() → popLine() = "PING :foo"

    A->>B: Parser::parse("PING :foo")
    B-->>A: Message{cmd=PING, params=["foo"]}
    A->>B: dispatch(fd, msg, _state)
    B->>C: getClientByFd(fd)
    C-->>B: Client*
    B->>B: ReplyBuilder::pong("foo")
    B-->>A: CommandResult{replies:[{fd, ":irc.local PONG irc.local :foo\r\n"}]}

    A->>A: applyCommandResult → bufferSend + _enablePollout(fd)
    Note over A: 次の poll で POLLOUT が立つ
    A->>A: _handleWrite → writeToSocket() → send()
    A-->>NC: ":irc.local PONG irc.local :foo\r\n"
```

## 要点

- **`bufferSend` と `send()` は別タイミング**：`applyCommandResult` は `_sendBuffer` に積んで POLLOUT を立てるだけ。実際の `send()` は次の poll 周回の `_handleWrite → writeToSocket` で実行される。
- **POLLOUT の明示トグル**：データを積むとき `_enablePollout`、送り切ったら `_disablePollout`（空 POLLOUT のビジーループ防止）。
- **送信先 fd は source とは限らない**：`applyCommandResult` は `reply.fd` ごとに `_connections.find` して投入（将来のブロードキャスト対応）。
- **層の責務分離**：A は I/O（recv/send/poll/接続管理）のみ。解釈は B、状態は C。
