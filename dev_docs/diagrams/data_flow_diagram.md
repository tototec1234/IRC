# データフロー図

> 作成日: 2026-05-23
> 用途: MTG資料（印刷用ペラ1枚）

---

## 受信→処理→送信の流れ

```mermaid
sequenceDiagram
    participant Client as IRCクライアント
    participant A as A層<br/>Network/IO
    participant B as B層<br/>Protocol/Command
    participant C1 as C1層<br/>Client/ServerState
    participant C2 as C2層<br/>Channel

    rect rgb(227, 242, 253)
        Note over Client,A: ネットワーク層
        Client->>A: TCP接続 + IRCコマンド
        A->>A: recv() → recvバッファに蓄積
        A->>A: \r\n で1行切り出し
    end

    rect rgb(232, 245, 233)
        Note over A,B: プロトコル層
        A->>B: complete line
        B->>B: Parser: line → Message
        B->>B: CommandDispatcher: コマンド判定
    end

    rect rgb(255, 243, 224)
        Note over B,C2: アプリケーション状態層
        alt NICK/USER/PASSコマンド
            B->>C1: Client状態更新
            C1-->>B: 結果
        else JOIN/KICK/MODEコマンド
            B->>C2: Channel状態更新
            C2-->>B: 結果
        end
    end

    rect rgb(232, 245, 233)
        Note over B,A: 返信生成
        B->>B: ReplyBuilder: 返信文字列生成
        B->>A: CommandResult
    end

    rect rgb(227, 242, 253)
        Note over A,Client: 送信
        A->>A: sendバッファに積む
        A->>A: POLLOUT時にsend()
        A->>Client: 返信
    end
```

---

## 層間インターフェース

```mermaid
flowchart LR
    subgraph A_layer["A層"]
        recv["recv()"]
        send["send()"]
        buffer["バッファ管理"]
    end

    subgraph B_layer["B層"]
        parser["Parser"]
        dispatcher["CommandDispatcher"]
        reply["ReplyBuilder"]
    end

    subgraph C_layer["C1/C2層"]
        client["Client"]
        state["ServerState"]
        channel["Channel"]
    end

    recv -->|"complete line<br/>(std::string)"| parser
    parser -->|"Message"| dispatcher
    dispatcher -->|"Client操作"| client
    dispatcher -->|"辞書操作"| state
    dispatcher -->|"Channel操作"| channel
    client -->|"結果"| dispatcher
    channel -->|"結果"| dispatcher
    dispatcher -->|"返信データ"| reply
    reply -->|"CommandResult"| send

    style recv fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style send fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style buffer fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style parser fill:#50B878,stroke:#3A8A5A,color:#fff
    style dispatcher fill:#50B878,stroke:#3A8A5A,color:#fff
    style reply fill:#50B878,stroke:#3A8A5A,color:#fff
    style client fill:#F5A623,stroke:#C4841C,color:#fff
    style state fill:#F5A623,stroke:#C4841C,color:#fff
    style channel fill:#F5A623,stroke:#C4841C,color:#fff
```

---

## 主要データ型

| 境界 | データ型 | 内容 |
|------|----------|------|
| A→B | `std::string` | complete line（`\r\n`除去済） |
| B内部 | `Message` | パース済IRCメッセージ（command, params） |
| B→A | `CommandResult` | 送信先fd + 送信文字列のリスト |
| B↔C1 | `Client*` | ユーザー状態へのポインタ |
| B↔C2 | `Channel*` | チャンネル状態へのポインタ |

---

## 色凡例

| 色 | 意味 |
|----|------|
| 🔵 青 | A層: Network/IO |
| 🟢 緑 | B層: Protocol/Command |
| 🟠 オレンジ | C1/C2層: アプリケーション状態 |
