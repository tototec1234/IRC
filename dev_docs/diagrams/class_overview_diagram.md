# クラス関係図

> **SSOT (Single Source of Truth)**: 本図は**クラス関係と主要 public API** の正式な定義です。
> 完全な public API 一覧は [`interface.md`](../interface.md) と header を参照します。

> **スコープ**: クラス間の関係性、層間境界に関わる API、不整合防止に重要な API のみ記載。
> private メンバ、private メソッド、単純 getter / setter は原則省略します。Optional クラスは詳細を実装フェーズで決定。
> **SSOT（契約憲章・意図）**: 層間契約の**理由・ルール・設計決定** → [`interface.md`](../interface.md)（本図と同じ契約の別視点。完全な API 一覧は `interface.md` と header を参照）  
> **実装読み物（B層主読者）**: [`b_implementation_reader.md`](../b_implementation_reader.md) — SSOT ではない

> 作成日: 2026-05-23
> 用途: MTG資料（印刷用ペラ1枚）
> 設計原則（オンボ必読）: [class_diagram_design_principles.md](../learning/class_diagram_design_principles.md) — 本図の読み方・SSOT の役割分担

---

## 【実装】ircserv クラス構造図

> 詳細設計: クラスは？関係は？

## クラス数と難易度


| 担当  | クラス数            | 主要クラス                                     | 難易度             |
| --- | --------------- | ----------------------------------------- | --------------- |
| A   | 2 (+2 optional) | Server, Connection                        | ★★★ poll/バッファ管理 |
| B   | 4               | Parser, Message, Dispatcher, ReplyBuilder | ★★☆ RFC理解が必要    |
| C   | 5               | ServerState, ClientRegistry, Client, Channel, ChannelModes | ★★☆ 状態整合性 |


### +optional の根拠


| 担当  | Optional クラス                   | 分離条件（design.md Section 3 参照） |
| --- | ------------------------------ | ---------------------------- |
| A   | Poller, ConnectionManager (+2) | poll管理・fd辞書が肥大化した場合          |
| B/C | ChannelService (+1)            | CommandDispatcher の channel 操作が肥大化した場合 |


**方針:** 初期実装では必須クラスのみ。肥大化したら optional を分離。

---

## 色凡例


| 色       | 意味                         |
| ------- | -------------------------- |
| 🔵 青    | A層: Network/IO             |
| 🟢 緑    | B層: Protocol/Command       |
| 🟠 濃いアンバー | C層: facade / ownership     |
| 🟡 薄いアンバー | C層: internal registry      |
| ⚫ グレー   | C層: entity                 |
| ⚪ 薄いグレー | C層: value-like state       |


```mermaid
classDiagram
    direction TB

    %% === A layer: Network / IO ===
    class Connection {
        <<A: Network / IO>>
        +read / write
        +line buffering
        +send buffering
    }

    class Server {
        <<A: Network / IO>>
        +run()
        +sendTo(fd, msg)
        +applyCommandResult(result)
    }

    %% === B layer: Protocol / Command ===
    class Parser {
        <<B: Protocol>>
        +parse(line) Message$
    }

    class Message {
        <<B: Protocol>>
        +command / params
    }

    class CommandDispatcher {
        <<B: Command>>
        +dispatch(fd, msg, state) CommandResult
        +validate IRC command
        +update C layer state
    }

    class ReplyBuilder {
        <<B: Reply>>
        +numeric replies
        +broadcast messages
    }

    class CommandResult {
        <<B/A boundary>>
        +replies
        +shouldDisconnect
    }

    %% === C layer: facade / registry / entities ===
    class ServerState {
        <<C: Facade / Owner>>
        +addClient(fd)
        +removeClient(fd)
        +updateNick(client, nick) bool
        +addClientToChannel(client, name) Channel*
        +removeClientFromChannel(client, name)
        +inviteClientToChannel(client, channel)
        +removeInviteFromChannel(client, channel)
    }

    class ClientRegistry {
        <<C: Internal Registry>>
        +fd / nick registry
    }

    class Client {
        <<C: Entity>>
        +identity / registration state
        +joined channel cache
        +getFullPrefix() string
        +_unsafe_setNick(nick)
        +_unsafe_joinChannel(channel)
        +_unsafe_leaveChannel(channel)
    }

    class Channel {
        <<C: Entity>>
        +members / operators
        +topic / invites
        +hasMember(client) bool
        +isOperator(client) bool
        +setOperator(client, isOperator)
        +isInvited(client) bool
        +_unsafe_addMember(client)
        +_unsafe_removeMember(client)
        +_unsafe_removeClientState(client)
    }

    class ChannelModes {
        <<C: Value-like State>>
        +modes i/t/k/l
    }

    %% === 関係線 ===
    Server *-- Connection : owns
    Server *-- ServerState : owns app state
    Server ..> Parser : uses
    Server ..> CommandDispatcher : uses
    Server ..> CommandResult : applies

    Parser ..> Message : creates
    CommandDispatcher ..> Message : receives
    CommandDispatcher ..> ServerState : facade operations
    CommandDispatcher ..> Client : reads/updates user state
    CommandDispatcher ..> Channel : reads/updates channel state
    CommandDispatcher ..> ChannelModes : reads/updates mode state
    CommandDispatcher ..> ReplyBuilder : uses
    CommandDispatcher ..> CommandResult : returns

    ReplyBuilder ..> Client : formats replies
    ReplyBuilder ..> Channel : formats replies

    ServerState *-- ClientRegistry : owns/delegates registry
    ServerState *-- Client : owns clients
    ServerState *-- Channel : owns channels
    ServerState ..> Client : syncs cache
    ServerState ..> Channel : syncs members/invites

    Client o-- Channel : joined cache
    Channel o-- Client : member refs
    Channel *-- ChannelModes : owns modes

    style Server fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Connection fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Parser fill:#50B878,stroke:#3A8A5A,color:#fff
    style Message fill:#50B878,stroke:#3A8A5A,color:#fff
    style CommandDispatcher fill:#50B878,stroke:#3A8A5A,color:#fff
    style ReplyBuilder fill:#50B878,stroke:#3A8A5A,color:#fff
    style CommandResult fill:#50B878,stroke:#3A8A5A,color:#fff
    style ServerState fill:#D9822B,stroke:#9A5A1E,color:#fff
    style ClientRegistry fill:#F8D9B0,stroke:#D9822B,color:#3A2A18
    style Client fill:#6C7A89,stroke:#45515C,color:#fff
    style Channel fill:#6C7A89,stroke:#45515C,color:#fff
    style ChannelModes fill:#D8DEE4,stroke:#6C7A89,color:#263238
```



---
