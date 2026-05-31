# クラス構造比較図

> 作成日: 2026-05-25
> 更新日: 2026-05-31
> 用途: 外部ft_irc実装との設計比較
> 対象: IRC_torinoue, barimehdi77, itsYakub, Ala-Na, ft_IRC-InternetRelayChat-

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

## 比較サマリ

| 要素 | IRC_torinoue | barimehdi77 | itsYakub | Ala-Na | ft_IRC-InternetRelayChat- |
|------|:-----------:|:-----------:|:--------:|:------:|:---------------------------:|
| **層分離** | ✅ 4層 | ❌ | ⚠️ 部分的 | ⚠️ 部分的 | ⚠️ 部分的 |
| **Connection** | ✅ | ❌ | ❌ | ❌ | ❌（Client内） |
| **Parser** | ✅ | Request | 内包 | Command | CommandHandler内 |
| **Dispatcher** | ✅ | Server内 | CommandHandler | Command | CommandHandler |
| **ReplyBuilder** | ✅ | Server内 | ServerReplies | Numerics | buildMessage/sendMsg |
| **ServerState** | ✅ | ❌ | ❌ | ❌ | ❌（IrcServer） |
| **InviteList** | ✅ | ❌ | ✅ | ✅ | ✅（nick名） |
| **BannedList** | ❌ | ✅ | ✅ | ✅ | ❌ |
| **送信バッファ** | ✅ Connection | ❌ | ❌ | ❌ | ✅ Client内 |
| **POLLOUT** | ✅ | ❌ | ❌ | ❌ | ✅ |

### 結論

**設計の分かれ目は「B層の分割粒度」と「IOバッファの所在」。**

| パターン | 該当 |
|---------|------|
| 4層分離 + CommandResult | IRC_torinoue（設計） |
| CommandHandler分離 + Server逆参照 | itsYakub, ft_IRC-InternetRelayChat- |
| Serverモノリシック | barimehdi77 |
| namespace + UserStatus | Ala-Na |

**ft_IRC-InternetRelayChat-から参考にできる点（Mandatory Part）:**
- `Client` 内 recv/send バッファ + `enablePollout` — IO堅牢性はIRC_torinoueのConnection設計と同趣旨（A層実装の参考）
- `makePrefix()` — `nick!user@host` 生成（ReplyBuilder へ）
- `removeClientFromAllChannels` — IRC_torinoueでは `removeClient` に内包する方針（`decision_invite_and_removal.md`）

**ft_IRC-InternetRelayChat-のコマンド別ファイル分割（`srcs/commands/`）について:**
- Mandatory Part 固定（約10コマンド・Bonus なし）なら **IRC_torinoue への必須採用は不要**
- `CommandDispatcher` + private method + `ReplyBuilder` 分離で十分。Dispatcher 肥大リスクは低い
- MODE のみ複雑度が高いため、必要なら `ModeHandler.cpp` への**部分分割**は任意

**IRC_torinoue設計の独自点:**
- Connection / ServerState / CommandResult による責務分離
- Server逆参照なし（CommandDispatcher → ServerState 経由）
- Inviteを Client* で管理（nick変更時の整合性を設計側で扱いやすい）
- B層: Parser / CommandDispatcher / ReplyBuilder の3分割（コマンド1ファイル分割は採用しない方針）

---

## 1. IRC_torinoue（設計図）

**特徴:** 4層分離、Connectionクラスあり、ServerState集中管理

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

## 5. ft_IRC-InternetRelayChat-

**特徴:** CommandHandlerにParser/Dispatcher/ReplyBuilderを集約、Client内バッファ、送信バッファ+POLLOUT対応

> Mandatory Part中心。Bot・CAP/WHO/WHOIS等の拡張コマンドは図から除外（実装あり）。


**IRC_torinoue設計との主な差分:**

| 観点 | IRC_torinoue | ft_IRC-InternetRelayChat- |
|------|-------------|---------------------------|
| IO分離 | Connectionクラス | Client内 `_buffer` / `_sendBuffer` |
| 状態集約 | ServerState | IrcServerが直接管理 |
| B層分割 | Parser / Dispatcher / ReplyBuilder | CommandHandlerに集約 |
| 返却型 | CommandResult | なし（sendMsgで即バッファ積み） |
| Client/Channel | ポインタ + ServerState | map値（Client/Channel）+ 内部ポインタ参照 |
| Invite管理 | Client* の set | nick名の set |
| コマンド配置 | Dispatcher + private method | `srcs/commands/` 1ファイル1コマンド |

```mermaid
classDiagram
    direction TB
    
    class IrcServer {
        -int m_listenFd
        -vector~pollfd~ m_pollFds
        -map~int, Client~ m_clients
        -map~string, Channel~ m_channels
        -CommandHandler m_commandHandler
        +run()
        +queueMessageForClient(fd, msg)
        +enablePollout(fd)
        +removeClientFromAllChannels(fd)
        +getCreateChannel(name) Channel&
    }
    
    class CommandHandler {
        -IrcServer& m_server
        -map~string, CommandFunc~ m_cmdMap
        +parseCommand(line, client)
        -parse(rawLine) Message
        -sendMsg(fd, msg)
        -buildMessage(...) string
        -handleJoin(msg, client)
        -handleKick(msg, client)
        -handleMode(msg, client)
    }
    
    class Message {
        +string prefix
        +string command
        +vector~string~ params
        +string trailing
        +bool isValid
    }
    
    class Client {
        -int m_fd
        -string m_buffer
        -string m_sendBuffer
        -string m_nickName
        -bool m_isRegistered
        -set~string~ m_joinedChannels
        +extractNextCommand(cmd) bool
        +appendSendBuffer(msg)
        +hasPendingSend() bool
        +makePrefix() string
    }
    
    class Channel {
        -string m_name
        -ChannelModes m_channelModes
        -IrcServer* m_server
        -map~int, Client*~ m_memberViews
        -set~string~ m_invitedNicks
        -set~int~ m_operators
        +addMember(client)
        +broadcast(msg)
        +addInvitedNick(nick)
    }
    
    class ChannelModes {
        +bool inviteOnly
        +bool topicProtected
        +string passKey
        +int userLimit
    }

    IrcServer "1" *-- "1" CommandHandler : owns
    IrcServer "1" o-- "*" Client : map値で保持
    IrcServer "1" o-- "*" Channel : map値で保持
    CommandHandler ..> Message : creates
    CommandHandler ..> Client : operates
    CommandHandler ..> Channel : operates
    CommandHandler ..> IrcServer : m_server参照
    Channel "1" *-- "1" ChannelModes : owns
    Channel ..> IrcServer : m_server参照
    Client ..> Channel : m_joinedChannels

    style IrcServer fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style CommandHandler fill:#50B878,stroke:#3A8A5A,color:#fff
    style Message fill:#50B878,stroke:#3A8A5A,color:#fff
    style Client fill:#F5A623,stroke:#C4841C,color:#fff
    style Channel fill:#795548,stroke:#5D4037,color:#fff
    style ChannelModes fill:#795548,stroke:#5D4037,color:#fff
```
