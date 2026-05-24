# 読書ガイド: 共通

> 対象: 全担当者（A, B, C1, C2）
> 作成日: 2026-05-23
> ステータス: 確定

---

## 共通で理解すべき概念

### 1. TCP/IP 基礎（書籍 第1章）

全員が最低限理解すべき概念：

| 概念 | 説明 |
|------|------|
| TCP vs UDP | TCP: 信頼性あり、コネクション型 / UDP: 信頼性なし、コネクションレス |
| IPアドレス | ホスト（マシン）を特定 |
| ポート番号 | ホスト内のプロセスを特定 |
| クライアント/サーバー | 接続を開始する側 / 待ち受ける側 |

### 2. ft_irc の全体構成

```mermaid
flowchart TB
    subgraph A["A担当: Network / IO"]
        Poll["poll()"]
        Server["Server"]
        Conn["Connection"]
    end

    subgraph B["B担当: Protocol / Command"]
        Parser["Parser"]
        Dispatcher["CommandDispatcher"]
        Reply["ReplyBuilder"]
    end

    subgraph C1["C1担当: Client / ServerState"]
        State["ServerState"]
        Client["Client"]
    end

    subgraph C2["C2担当: Channel"]
        Channel["Channel"]
        Modes["ChannelModes"]
    end

    Poll --> Server
    Server --> Conn
    Conn -->|complete line| Parser
    Parser -->|Message| Dispatcher
    Dispatcher --> State
    State --> Client
    Dispatcher --> Channel
    Channel --> Modes
    Dispatcher --> Reply
    Reply -->|CommandResult| Server

    style A fill:#E3F2FD,stroke:#64B5F6
    style B fill:#E8F5E9,stroke:#81C784
    style C1 fill:#FFF3E0,stroke:#FFB74D
    style C2 fill:#FFF3E0,stroke:#FFB74D
    style Poll fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Server fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Conn fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Parser fill:#50B878,stroke:#3A8A5A,color:#fff
    style Dispatcher fill:#50B878,stroke:#3A8A5A,color:#fff
    style Reply fill:#50B878,stroke:#3A8A5A,color:#fff
    style State fill:#F5A623,stroke:#C4841C,color:#fff
    style Client fill:#F5A623,stroke:#C4841C,color:#fff
    style Channel fill:#F5A623,stroke:#C4841C,color:#fff
    style Modes fill:#F5A623,stroke:#C4841C,color:#fff
```

### 3. データの流れ

```mermaid
sequenceDiagram
    participant Client as IRCクライアント
    participant A as A層(Network)
    participant B as B層(Protocol)
    participant C as C層(State)

    rect rgb(227, 242, 253)
        Note over Client,A: ネットワーク層（青）
        Client->>A: TCP接続 + IRCコマンド
        A->>A: recv() → バッファに蓄積
        A->>A: \r\n で1行切り出し
    end

    rect rgb(232, 245, 233)
        Note over A,B: プロトコル層（緑）
        A->>B: complete line
        B->>B: Parser: line → Message
        B->>B: CommandDispatcher: コマンド実行
    end

    rect rgb(255, 243, 224)
        Note over B,C: 状態層（オレンジ）
        B->>C: ServerState/Channel 操作
        C-->>B: 結果
    end

    rect rgb(232, 245, 233)
        Note over B,A: 返信生成
        B->>B: ReplyBuilder: 返信生成
        B->>A: CommandResult
    end

    rect rgb(227, 242, 253)
        Note over A,Client: 送信
        A->>A: send buffer に積む
        A->>Client: send()
    end
```

---

## 共通リソース

| リソース | 用途 |
|---------|------|
| [design.md](../../myIRCd/docs/design.md) | 全体設計、責務分割 |
| [interface.md](../../myIRCd/docs/interface.md) | 各クラスのインターフェース |
| [RFC 1459](https://datatracker.ietf.org/doc/html/rfc1459) | IRCプロトコル基本仕様 |
| [RFC 2812](https://datatracker.ietf.org/doc/html/rfc2812) | IRCクライアントプロトコル詳細 |

---

## 担当別ガイドへのリンク

- [A担当（Network/IO）](./reading_guide_A.md) - ★書籍メイン
- [B担当（Protocol/Command）](./reading_guide_B.md)
- [C1担当（Client/ServerState）](./reading_guide_C1.md)
- [C2担当（Channel）](./reading_guide_C2.md)
