# データフロー比較図

> 作成日: 2026-05-25
> 更新日: 2026-05-31
> 用途: 外部ft_irc実装とのデータフロー比較
> 対象: IRC_torinoue, barimehdi77, itsYakub, Ala-Na, ft_IRC-InternetRelayChat-
---

## 比較サマリ

| 要素 | IRC_torinoue | barimehdi77 | itsYakub | Ala-Na | ft_IRC-InternetRelayChat- |
|------|:-----------:|:-----------:|:--------:|:------:|:---------------------------:|
| **受信バッファ位置** | Connection | Server (map) | Client | Server | Client |
| **送信バッファ** | ✅ Connection | ❌ なし | ❌ なし | ❌ なし | ✅ Client |
| **CommandResult** | ✅ | ❌ | ❌ | ❌ | ❌ |
| **POLLOUT対応** | ✅ | ❌ | ❌ | ❌ | ✅ |
| **即時send()** | ❌ | ✅ | ✅ | ✅ | ❌ |
| **Parser分離** | ✅ | Request | 内包 | Command | CommandHandler内 |

### 結論

**IO堅牢性（非ブロッキング送信）の実装パターンが2系統ある。**

| パターン | 該当 | 特徴 |
|---------|------|------|
| Connection + CommandResult | IRC_torinoue（設計） | 層間データ型が明示、返信を一括適用 |
| Client内バッファ + enablePollout | ft_IRC-InternetRelayChat- | 実装済み、CommandHandlerから逐次バッファ積み |
| 即時 send() | barimehdi77, itsYakub, Ala-Na | シンプルだがブロックリスク |

**ft_IRC-InternetRelayChat-の位置づけ:**
- barimehdi77 / itsYakub / Ala-Na と異なり POLLOUT + 送信バッファあり
- IRC_torinoue設計と目的は同じ（非ブロッキングIO）だが、バッファ所在と返信集約方式が異なる

---

## 色凡例（全図共通）

| 色 | 意味 |
|----|------|
| 🔵 青 | Network/IO層 |
| 🟢 緑 | Protocol/Command層 |
| 🟠 オレンジ | Client/State層 |
| 🤎 茶 | Channel層 |

---

## 1. IRC_torinoue（設計図）

**特徴:** 層間で明確なデータ型を定義、CommandResultで送信を遅延

```mermaid
sequenceDiagram
    participant Client as IRCクライアント
    participant A as A層<br/>Server/Connection
    participant B as B層<br/>Parser/Dispatcher
    participant C1 as C1層<br/>Client/ServerState
    participant C2 as C2層<br/>Channel

    rect rgb(227, 242, 253)
        Note over Client,A: Network層
        Client->>A: TCP接続 + IRCコマンド
        A->>A: recv() → recvバッファ蓄積
        A->>A: \r\n で1行切り出し
    end

    rect rgb(232, 245, 233)
        Note over A,B: Protocol層
        A->>B: complete line (string)
        B->>B: Parser: line → Message
        B->>B: CommandDispatcher: コマンド判定
    end

    rect rgb(255, 243, 224)
        Note over B,C2: 状態更新
        alt NICK/USER/PASS
            B->>C1: Client状態更新
            C1-->>B: 結果
        else JOIN/KICK/MODE
            B->>C2: Channel状態更新
            C2-->>B: 結果
        end
    end

    rect rgb(232, 245, 233)
        Note over B,A: 返信生成
        B->>B: ReplyBuilder: 返信文字列生成
        B-->>A: CommandResult (fd + msg)
    end

    rect rgb(227, 242, 253)
        Note over A,Client: 送信
        A->>A: sendバッファに積む
        A->>A: POLLOUT時にsend()
        A-->>Client: 返信
    end
```

**データ型:**
- A→B: `std::string` (complete line)
- B内部: `Message` (command + params)
- B→A: `CommandResult` (replies + shouldDisconnect)

---

## 2. barimehdi77/ft_irc

**特徴:** Serverが全処理を内包、即時send()

```mermaid
sequenceDiagram
    participant Client as IRCクライアント
    participant Server as Server<br/>(全機能内包)
    participant Request as Request<br/>(Parser)
    participant ClientObj as Client
    participant Channel as Channel

    rect rgb(227, 242, 253)
        Client->>Server: TCP接続
        Server->>Server: poll() POLLIN検出
        Server->>Server: recv()
    end

    rect rgb(232, 245, 233)
        Server->>Request: _splitRequest(message)
        Request-->>Server: Request構造体
        Server->>Server: _parsing(message, i)
    end

    rect rgb(255, 243, 224)
        alt NICK/USER
            Server->>ClientObj: setNickName() / setUserName()
        else JOIN/KICK
            Server->>Channel: addMember() / removeMember()
        end
    end

    rect rgb(227, 242, 253)
        Server->>Server: 即時 send()
        Server-->>Client: 返信
    end
```

**データ型:**
- 内部: `Request` (command + args)
- 送信: 即時 `send()` (バッファリングなし)

**問題点:** `send()` がブロックする可能性あり

---

## 3. itsYakub/42-ft_irc

**特徴:** CommandHandler分離、直接send()

```mermaid
sequenceDiagram
    participant Client as IRCクライアント
    participant Server as Server
    participant Handler as CommandHandler
    participant ClientObj as Client
    participant Channel as Channel

    rect rgb(227, 242, 253)
        Client->>Server: TCP接続
        Server->>Server: poll() POLLIN検出
        Server->>Server: _handleClientMessage()
        Server->>Server: recv() → Client._buffer蓄積
    end

    rect rgb(232, 245, 233)
        Server->>Handler: processInput(buffer, fd)
        Handler->>Handler: _splitCmd()
        Handler->>Handler: コマンド判定
    end

    rect rgb(255, 243, 224)
        alt NICK/USER
            Handler->>ClientObj: setNick() / setUsername()
        else JOIN/KICK/MODE
            Handler->>Channel: addClient() / delClient()
        end
    end

    rect rgb(232, 245, 233)
        Handler->>Server: sendResponse(reply, fd)
    end

    rect rgb(227, 242, 253)
        Server->>Server: 即時 send()
        Server-->>Client: 返信
    end
```

**データ型:**
- Client内: `_buffer` (受信バッファをClient内に保持)
- 送信: `Server::sendResponse()` 経由で即時 `send()`

---

## 4. Ala-Na/ft_irc

**特徴:** namespace使用、UserStatus enum、設定ファイル対応

```mermaid
sequenceDiagram
    participant Client as IRCクライアント
    participant Server as irc::Server
    participant Command as irc::Command
    participant User as irc::User
    participant Channel as irc::Channel

    rect rgb(227, 242, 253)
        Client->>Server: TCP接続
        Server->>Server: poll() POLLIN検出
        Server->>Server: receiveDatas()
        Server->>Server: datasExtraction()
    end

    rect rgb(232, 245, 233)
        Server->>Command: コマンド実行
        Note over Command: Numerics.cppで返信生成
    end

    rect rgb(255, 243, 224)
        alt PASS/NICK/USER
            Command->>User: setNickname() / setUsername()
            User->>User: UserStatus更新
        else JOIN/KICK/MODE
            Command->>Channel: addUser() / deleteChanUser()
        end
    end

    rect rgb(227, 242, 253)
        Command->>Server: 送信要求
        Server->>Server: 即時 send()
        Server-->>Client: 返信
    end
```

**データ型:**
- `UserStatus` enum: PASS → NICK → USER → REGISTERED
- 送信: 即時 `send()`

---

## 5. ft_IRC-InternetRelayChat-

**特徴:** Client内バッファ、CommandHandlerが解析〜返信生成まで担当、送信バッファ+POLLOUT

> Mandatory Part中心。Bot等の拡張は図から除外。

```mermaid
sequenceDiagram
    participant Client as IRCクライアント
    participant Server as IrcServer
    participant Handler as CommandHandler
    participant ClientObj as Client
    participant Channel as Channel

    rect rgb(227, 242, 253)
        Note over Client,Server: Network層
        Client->>Server: TCP接続 + IRCコマンド
        Server->>Server: recv() → Client.m_buffer蓄積
        Server->>ClientObj: extractNextCommand() で1行切り出し
    end

    rect rgb(232, 245, 233)
        Note over Server,Handler: Protocol層
        Server->>Handler: parseCommand(line, client)
        Handler->>Handler: parse(): line → Message
        Handler->>Handler: m_cmdMap でコマンド判定
    end

    rect rgb(255, 243, 224)
        Note over Handler,Channel: 状態更新
        alt PASS/NICK/USER
            Handler->>ClientObj: setNickName() / setRegistered()
            Handler->>Server: isNickInUse() 等
        else JOIN/KICK/MODE
            Handler->>Server: getCreateChannel() / findChannel()
            Handler->>Channel: addMember() / removeMember()
            Handler->>ClientObj: joinChannel() / leaveChannel()
        end
    end

    rect rgb(232, 245, 233)
        Note over Handler,Server: 返信生成
        Handler->>Handler: buildMessage() / sendError()
        Handler->>ClientObj: appendSendBuffer(msg)
        Handler->>Server: enablePollout(fd)
    end

    rect rgb(227, 242, 253)
        Note over Server,Client: 送信
        Server->>Server: poll() POLLOUT検出
        Server->>Server: send() → sendBufferから消費
        Server-->>Client: 返信
    end
```

**データ型:**
- recv: `Client.m_buffer` 内で `\r\n` 切り出し
- 内部: `CommandHandler::Message` (prefix + command + params + trailing)
- 送信: `Client.m_sendBuffer` + `enablePollout()`（CommandResult相当の集約型なし）

**IRC_torinoue設計との差分:**
- 受信バッファ位置: Connection vs Client
- 返信の集約: CommandResult vs コマンド処理中に逐次 `appendSendBuffer`
- QUIT時: `removeClientFromAllChannels` をHandlerから直接呼ぶ（IRC_torinoueは `removeClient` に集約）

