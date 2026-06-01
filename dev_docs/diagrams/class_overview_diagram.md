# クラス関係図

> **SSOT (Single Source of Truth)**: 本図が**公開 API とクラス関係**の正式な定義です。
> 他のドキュメントとの差異がある場合、本図を正とします。

> **スコープ**: クラス間の関係性と公開API（`+`）のみ記載。
> プライベートメソッド（`-`ChannelService）は省略。詳細は実装フェーズで決定。
> 契約憲章 → [`interface.md`](../interface.md) / 実装読み物 → [`b_implementation_reader.md`](../b_implementation_reader.md)（SSOT ではない）

> 作成日: 2026-05-23
> 用途: MTG資料（印刷用ペラ1枚）
> 設計原則: [class_diagram_design_principles.md](../learning/class_diagram_design_principles.md) このmdは学習メモ的なものです

---

## 【実装】ircserv クラス構造図

> 詳細設計: クラスは？関係は？

## クラス数と難易度


| 担当  | クラス数            | 主要クラス                                     | 難易度             |
| --- | --------------- | ----------------------------------------- | --------------- |
| A   | 2 (+2 optional) | Server, Connection                        | ★★★ poll/バッファ管理 |
| B   | 4               | Parser, Message, Dispatcher, ReplyBuilder | ★★☆ RFC理解が必要    |
| C1  | 2 (+1 optional) | Client, ServerState                       | ★★☆ 辞書整合性       |
| C2  | 2 (+1 optional) | Channel, ChannelModes                     | ★☆☆ 比較的シンプル     |


### +optional の根拠


| 担当  | Optional クラス                   | 分離条件（design.md Section 3 参照） |
| --- | ------------------------------ | ---------------------------- |
| A   | Poller, ConnectionManager (+2) | poll管理・fd辞書が肥大化した場合          |
| C1  | ClientRegistry (+1)            | ServerState が肥大化した場合         |
| C2  | ChannelService (+1)            | CommandDispatcher が肥大化した場合   |


**方針:** 初期実装では必須クラスのみ。肥大化したら optional を分離。

---

## 色凡例


| 色       | 意味                        |
| ------- | ------------------------- |
| 🔵 青    | A層: Network/IO            |
| 🟢 緑    | B層: Protocol/Command      |
| 🟠 オレンジ | C1層: Client/ServerState   |
| 🤎 茶    | C2層: Channel/ChannelModes |


```mermaid

classDiagram
    direction TB

   
    %% === 最上段: Connection, Server, Parser, CommandDispatcher ===
    class Connection {
        -int _fd
        -string _recvBuffer
        -string _sendBuffer
        +getFd() int
        +readFromSocket() bool
        +writeToSocket() bool
        +hasCompleteLine() bool
        +popLine() string
        +bufferSend(msg)
        +hasPendingOutput() bool
    }
    
    class Server {
        -int _listenFd
        -vector~pollfd~ _pollfds
        -map~int, Connection*~ _connections
        -ServerState _state
        +run()
        +sendTo(fd, msg)
        +applyCommandResult(result)
    }
    
    class Parser {
        +parse(line) Message$
    }
    
    class CommandDispatcher {
        +dispatch(fd, msg, state) CommandResult
    }
    
    %% === 中段: Message, ServerState, ReplyBuilder, CommandResult ===
    class Message {
        -string _command
        -vector~string~ _params
        +getCommand() string
        +getParams() vector
        +getParamCount() size_t
        +hasParam(index) bool
        +getSingleParam(index) string
    }
    
    class ServerState {
        -string _password
        -map~int, Client*~ _fdToClient
        -map~string, Client*~ _nickToClient
        -map~string, Channel*~ _channels
        +getClientByFd(fd) Client*
        +updateNick(client, nick)
        +removeClient(fd)
    }
    
    class ReplyBuilder {
        +welcome(client) string$
        +needMoreParams(client, cmd) string$
        +join(client, channel) string$
    }
    
    class CommandResult {
        +vector~OutgoingMessage~ replies
        +bool shouldDisconnect
        +addReply(fd, msg)
    }
    
    %% === 下段: Client, Channel, ChannelModes ===
    class Client {
        -int _fd
        -string _nick
        -string _username
        -string _realname
        -string _host
        -bool _passOk
        -bool _registered
        +getFd() int
        +getNick() string
        +getUsername() string
        +getRealname() string
        +getHost() string
        +getFullPrefix() string
        +isRegistered() bool
    }
    
    class Channel {
        -string _name
        -string _topic
        -set~Client*~ _members
        -set~Client*~ _operators
        -set~Client*~ _invited
        -ChannelModes _modes
        +addMember(client)
        +isOperator(client) bool
        +addInvite(client)
        +isInvited(client) bool
    }
    
    class ChannelModes {
        -bool _inviteOnly
        -bool _topicRestricted
        -string _key
        -int _limit
        +setInviteOnly(value)
        +hasKey() bool
    }

    %% === 関係線 ===
    Server "1" *-- "*" Connection : owns
    Server "1" *-- "1" ServerState : owns
    Server ..> Parser : uses
    Server ..> CommandDispatcher : uses
    Server ..> CommandResult : applies
    
    Parser ..> Message : creates
    CommandDispatcher ..> Message : receives
    CommandDispatcher ..> ServerState : operates
    CommandDispatcher ..> ReplyBuilder : uses
    CommandDispatcher ..> CommandResult : returns
    
    ReplyBuilder ..> Client : uses
    ReplyBuilder ..> Channel : uses
    
    ServerState "1" *-- "*" Client : owns
    ServerState "1" o-- "*" Channel : references
    
    Channel "1" *-- "1" ChannelModes : owns
    Client "*" --o "*" Channel : references

    style Server fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Connection fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Parser fill:#50B878,stroke:#3A8A5A,color:#fff
    style Message fill:#50B878,stroke:#3A8A5A,color:#fff
    style CommandDispatcher fill:#50B878,stroke:#3A8A5A,color:#fff
    style ReplyBuilder fill:#50B878,stroke:#3A8A5A,color:#fff
    style CommandResult fill:#50B878,stroke:#3A8A5A,color:#fff
    style Client fill:#F5A623,stroke:#C4841C,color:#fff
    style ServerState fill:#F5A623,stroke:#C4841C,color:#fff
    style Channel fill:#795548,stroke:#5D4037,color:#fff
    style ChannelModes fill:#795548,stroke:#5D4037,color:#fff
```



---

