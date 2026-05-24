# クラス関係図

> 作成日: 2026-05-23
> 用途: MTG資料（印刷用ペラ1枚）

---

## 全体クラス図

```mermaid
classDiagram
    direction TB
    
    class Server {
        -int _listenFd
        -vector~pollfd~ _pollfds
        -map~int, Connection*~ _connections
        -ServerState _state
        +run()
        +queueSend(fd, msg)
        +applyCommandResult(result)
    }
    
    class Connection {
        -int _fd
        -string _recvBuffer
        -string _sendBuffer
        +readFromSocket() bool
        +writeToSocket() bool
        +hasCompleteLine() bool
        +popLine() string
    }
    
    class Parser {
        +parse(line) Message$
    }
    
    class Message {
        -string _command
        -vector~string~ _params
        +command() string
        +params() vector
    }
    
    class CommandDispatcher {
        +dispatch(fd, msg, state) CommandResult
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
    
    class Client {
        -int _fd
        -string _nick
        -string _username
        -bool _passOk
        -bool _registered
        +nick() string
        +isRegistered() bool
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
    
    class Channel {
        -string _name
        -string _topic
        -set~Client*~ _members
        -set~Client*~ _operators
        -ChannelModes _modes
        +addMember(client)
        +isOperator(client) bool
    }
    
    class ChannelModes {
        -bool _inviteOnly
        -bool _topicRestricted
        -string _key
        -int _limit
        +setInviteOnly(value)
        +hasKey() bool
    }

    Server "1" *-- "*" Connection : owns
    Server "1" *-- "1" ServerState : owns
    Server ..> Parser : uses
    Server ..> CommandDispatcher : uses
    
    CommandDispatcher ..> Message : receives
    CommandDispatcher ..> ServerState : operates
    CommandDispatcher ..> ReplyBuilder : uses
    CommandDispatcher ..> CommandResult : returns
    
    ServerState "1" *-- "*" Client : owns
    ServerState "1" o-- "*" Channel : references
    
    Channel "1" *-- "1" ChannelModes : owns
    Channel "*" o-- "*" Client : references
```

---

## 担当別クラス配置

```mermaid
flowchart TB
    subgraph A_LAYER["A層: Network/IO（torinoue）"]
        Server
        Connection
    end
    
    subgraph B_LAYER["B層: Protocol/Command（torinoue）"]
        Parser
        Message
        CommandDispatcher
        ReplyBuilder
        CommandResult
    end
    
    subgraph C1_LAYER["C1層: Client/ServerState（taro）"]
        Client
        ServerState
    end
    
    subgraph C2_LAYER["C2層: Channel（hanako）"]
        Channel
        ChannelModes
    end

    style A_LAYER fill:#E3F2FD,stroke:#64B5F6
    style B_LAYER fill:#E8F5E9,stroke:#81C784
    style C1_LAYER fill:#FFF3E0,stroke:#FFB74D
    style C2_LAYER fill:#FFF3E0,stroke:#FFB74D
    
    style Server fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Connection fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Parser fill:#50B878,stroke:#3A8A5A,color:#fff
    style Message fill:#50B878,stroke:#3A8A5A,color:#fff
    style CommandDispatcher fill:#50B878,stroke:#3A8A5A,color:#fff
    style ReplyBuilder fill:#50B878,stroke:#3A8A5A,color:#fff
    style CommandResult fill:#50B878,stroke:#3A8A5A,color:#fff
    style Client fill:#F5A623,stroke:#C4841C,color:#fff
    style ServerState fill:#F5A623,stroke:#C4841C,color:#fff
    style Channel fill:#F5A623,stroke:#C4841C,color:#fff
    style ChannelModes fill:#F5A623,stroke:#C4841C,color:#fff
```

---

## クラス数と難易度

| 担当 | クラス数 | 主要クラス | 難易度 |
|------|---------|-----------|--------|
| A | 2 (+2 optional) | Server, Connection | ★★★ poll/バッファ管理 |
| B | 4 | Parser, Message, Dispatcher, ReplyBuilder | ★★☆ RFC理解が必要 |
| C1 | 2 (+1 optional) | Client, ServerState | ★★☆ 辞書整合性 |
| C2 | 2 (+1 optional) | Channel, ChannelModes | ★☆☆ 比較的シンプル |

---

## 色凡例

| 色 | 意味 |
|----|------|
| 🔵 青 | A層: Network/IO |
| 🟢 緑 | B層: Protocol/Command |
| 🟠 オレンジ | C1/C2層: アプリケーション状態 |
