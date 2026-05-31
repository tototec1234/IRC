# 担当間依存関係図

> 作成日: 2026-05-23  
> 最終確定: 2026-05-29（Phase 3-4 完了、セッション #0006）  
> 用途: MTG資料（印刷用ペラ1枚）

---

## 実装依存関係
> 【実装】: どのクラスが何に依存するか

```mermaid
flowchart TB
    subgraph APP["アプリケーション状態層"]
        C1["C1: Client/ServerState<br/>（taro）"]
        C2["C2: Channel<br/>（hanako）"]
    end
    
    subgraph PROTO["プロトコル層"]
        B["B: Protocol/Command<br/>（torinoue）"]
    end
    
    subgraph NET["ネットワーク層"]
        A["A: Network/IO<br/>（torinoue）"]
    end

    A -->|"complete line"| B
    B -->|"状態操作"| C1
    B -->|"状態操作"| C2
    C1 -.->|"Channel辞書参照"| C2
    B -->|"CommandResult"| A

    style A fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style B fill:#50B878,stroke:#3A8A5A,color:#fff
    style C1 fill:#F5A623,stroke:#C4841C,color:#fff
    style C2 fill:#795548,stroke:#5D4037,color:#fff
    style APP fill:#FFF3E0,stroke:#FFB74D
    style PROTO fill:#E8F5E9,stroke:#81C784
    style NET fill:#E3F2FD,stroke:#64B5F6
```

---

## 依存関係の詳細

| 依存元 | 依存先 | 内容 | ブロッカー度 |
|--------|--------|------|-------------|
| B | A | complete lineを受け取る | ★★★ Aが先に必要 |
| B | C1 | Client/ServerState操作 | ★★☆ インターフェース合意必要 |
| B | C2 | Channel操作 | ★★☆ インターフェース合意必要 |
| C1 | C2 | ServerStateがChannel辞書を持つ | ★☆☆ 疎結合、後から統合可 |
| A | B | CommandResultを受け取る | ★★☆ インターフェース合意必要 |

---

## 並行開発可能な範囲
> 【開発】: 上記の実装依存関係を前提に、誰がいつ何を作るか

```mermaid
flowchart LR
    subgraph PARALLEL["並行開発可能"]
        direction TB
        C1_work["C1: Client/ServerState<br/>クラス実装"]
        C2_work["C2: Channel/ChannelModes<br/>クラス実装"]
    end
    
    subgraph SEQUENTIAL["順次作業"]
        A_work["A: Network基盤"]
        B_work["B: Parser/Dispatcher"]
    end
    
    A_work --> B_work
    B_work --> PARALLEL

    style C1_work fill:#F5A623,stroke:#C4841C,color:#fff
    style C2_work fill:#795548,stroke:#5D4037,color:#fff
    style A_work fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style B_work fill:#50B878,stroke:#3A8A5A,color:#fff
```

**注:** 【仕様】を満たす範囲で【実装】を変更することで、【開発】のクリティカルパスを短縮できる可能性がある。

---

## 色凡例

| 色 | 意味 |
|----|------|
| 🔵 青 | A層: Network/IO |
| 🟢 緑 | B層: Protocol/Command |
| 🟠 オレンジ | C1層: Client/ServerState |
| 🤎 茶 | C2層: Channel/ChannelModes |
