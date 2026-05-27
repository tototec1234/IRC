# クラス構造比較図

> 作成日: 2026-05-25
> 更新日: 2026-05-25
> 用途: 外部ft_irc実装との設計比較
> 対象: IRC_torinoue, barimehdi77, itsYakub, Ala-Na

---

## 色凡例（全図共通）

| 色 | 意味 |
|----|------|
| 🔵 青 | Network/IO層 |
| 🟢 緑 | Protocol/Command層 |
| 🟠 オレンジ | Client/State層 |
| 🤎 茶 | Channel層 |
| ⚪ グレー | その他/ユーティリティ |

---

## 1. IRC_torinoue（設計図）

**特徴:** 4層分離、Connectionクラスあり、ServerState集中管理

```mermaid
classDiagram
    direction TB
    
    class Server {
        -int _listenFd
        -vector~pollfd~ _pollfds
        -map~int,Connection*~ _connections
        -ServerState _state
    }
    
    class Connection {
        -int _fd
        -string _recvBuffer
        -string _sendBuffer
        +...()
    }
    
    class Parser {
        +parse(line) Message
    }
    
    class Message {
        -string _command
        -vector~string~ _params
        +getCommand() string
        +getParams() vector~string~
        +getParamCount() size_t
        +hasParam(index) bool
        +getSingleParam(index) string
    }
    
    class CommandDispatcher {
        +dispatch(fd,msg,state)
    }
    
    class ReplyBuilder {
        +welcome()
        +error()
    }
    
    class CommandResult {
        +replies
        +shouldDisconnect
    }
    
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
    
    class ServerState {
        -map~int,Client*~ _fdToClient
        -map~string,Client*~ _nickToClient
        -map~string,Channel*~ _channels
    }
    
    class Channel {
        -string _name
        -set~Client*~ _members
        -set~Client*~ _operators
        -set~Client*~ _invited
    }
    
    class ChannelModes {
        -bool _inviteOnly
        -bool _topicRestricted
        -string _key
        -int _limit
    }

    Server *-- Connection
    Server *-- ServerState
    Server ..> Parser
    Server ..> CommandDispatcher
    CommandDispatcher ..> Message
    CommandDispatcher ..> ReplyBuilder
    CommandDispatcher ..> CommandResult
    ServerState *-- Client
    ServerState o-- Channel
    Channel *-- ChannelModes

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

## 2. barimehdi77/ft_irc

**特徴:** Serverモノリシック、全コマンドをServer内で処理

```mermaid
classDiagram
    direction TB
    
    class Server {
        -int _socketfd
        -pollfd* _pfds
        -map~int,Client*~ _clients
        -map~string,Channel*~ _allChannels
        +_parsing()
        +_joinChannel()
        +_kick()
        +_privmsg()
    }
    
    class Request {
        -string _command
        -vector~string~ _args
    }
    
    class Client {
        -int _clientfd
        -bool _Auth
        -bool _Registered
        -string _NickName
        -string _UserName
        -string _FullName
        -Modes _modes
        -map~string,Channel*~ _joinedChannels
    }
    
    class Channel {
        -string _name
        -string _key
        -string _topic
        -map~int,Client*~ _members
        -map~int,Client*~ _operators
        -vector~string~ _banned
    }
    
    class File {
        -string _filename
        -string _content
    }

    Server *-- Client
    Server *-- Channel
    Server ..> Request
    Server ..> File
    Client o-- Channel

    style Server fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Request fill:#50B878,stroke:#3A8A5A,color:#fff
    style Client fill:#F5A623,stroke:#C4841C,color:#fff
    style Channel fill:#795548,stroke:#5D4037,color:#fff
    style File fill:#9E9E9E,stroke:#757575,color:#fff
```

---

## 3. itsYakub/42-ft_irc

**特徴:** CommandHandler分離、Channel内にinvited管理

```mermaid
classDiagram
    direction TB
    
    class Server {
        -int _serverFd
        -vector~pollfd~ _pollFds
        -vector~Client*~ _clients
        -vector~Channel*~ _channels
        -CommandHandler m_commandHandler
    }
    
    class CommandHandler {
        -Server& m_Server
        +processInput()
        -_handleJoinCmd()
        -_handleModeCmd()
        -_handleKickCmd()
    }
    
    class ServerReplies {
        +RPL_WELCOME
        +ERR_NOSUCHNICK
    }
    
    class Client {
        -int _fd
        -bool _is_auth
        -string _nickname
        -string _username
        -string _buffer
    }
    
    class Channel {
        -string m_name
        -string m_topic
        -string m_key
        -size_t m_memberLimit
        -bool m_inviteOnly
        -bool m_topicOperatorPrivilege
        -vector~Client*~ m_clients
        -vector~Client*~ m_operators
        -vector~Client*~ m_invitedClients
    }
    
    class Bot {
        -vector~string~ _bannedWords
    }

    Server *-- CommandHandler
    Server *-- Client
    Server *-- Channel
    Server *-- Bot
    CommandHandler ..> ServerReplies
    Channel o-- Client

    style Server fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style CommandHandler fill:#50B878,stroke:#3A8A5A,color:#fff
    style ServerReplies fill:#50B878,stroke:#3A8A5A,color:#fff
    style Client fill:#F5A623,stroke:#C4841C,color:#fff
    style Channel fill:#795548,stroke:#5D4037,color:#fff
    style Bot fill:#9E9E9E,stroke:#757575,color:#fff
```

---

## 4. Ala-Na/ft_irc

**特徴:** namespace使用、UserStatus enum、詳細なユーザーモード

```mermaid
classDiagram
    direction TB
    
    class Server {
        -vector~pollfd~ pfds
        -vector~User*~ users
        -vector~User*~ operators
        -vector~Channel*~ channels
        -string password
    }
    
    class Command {
        +execute()
    }
    
    class Numerics {
        +RPL_WELCOME()
        +ERR_NOSUCHNICK()
    }
    
    class User {
        -int _fd
        -string _nickname
        -string _username
        -string _real_name
        -UserStatus _status
        -bool userModes_a
        -bool userModes_i
        -bool userModes_o
        -vector~Channel*~ _channels
    }
    
    class Channel {
        -string chan_name
        -string chan_password
        -string chan_topic
        -string chan_mode
        -unsigned long max_nb_users
        -vector~User*~ vec_chan_users
        -vector~User*~ vec_chan_operators
        -vector~User*~ vec_chan_banned_users
        -vector~User*~ vec_chan_invited_users
    }

    Server *-- User
    Server *-- Channel
    Server ..> Command
    Command ..> Numerics
    User o-- Channel

    style Server fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Command fill:#50B878,stroke:#3A8A5A,color:#fff
    style Numerics fill:#50B878,stroke:#3A8A5A,color:#fff
    style User fill:#F5A623,stroke:#C4841C,color:#fff
    style Channel fill:#795548,stroke:#5D4037,color:#fff
```

---

## 比較サマリ

| 要素 | IRC_torinoue | barimehdi77 | itsYakub | Ala-Na |
|------|:-----------:|:-----------:|:--------:|:------:|
| **層分離** | ✅ 4層 | ❌ | ⚠️ 部分的 | ⚠️ 部分的 |
| **Connection** | ✅ | ❌ | ❌ | ❌ |
| **Parser** | ✅ | Request | 内包 | Command |
| **Dispatcher** | ✅ | Server内 | CommandHandler | Command |
| **ReplyBuilder** | ✅ | Server内 | ServerReplies | Numerics |
| **ServerState** | ✅ | ❌ | ❌ | ❌ |
| **InviteList** | ✅ | ❌ | ✅ | ✅ |
| **BannedList** | ❌ | ✅ | ✅ | ✅ |

### 結論

**IRC_torinoueの設計は最も層分離が明確。** 他実装はServerが肥大化する傾向。

ただし他実装から学べる点:
- `UserStatus` enum（Ala-Na）: 状態遷移の明示化
- `CommandHandler` 分離（itsYakub）: IRC_torinoueと同じ発想
- `getFullId()` メソッド（itsYakub）: `nick!user@host` 生成
