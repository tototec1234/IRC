# 実装タイムライン図

> 作成日: 2026-05-23
> 用途: MTG資料（印刷用ペラ1枚）

---

## 実装順序（ガントチャート風）

```mermaid
gantt
    title ft_irc 実装タイムライン
    dateFormat  YYYY-MM-DD
    
    section A層
    Server基盤 poll           :a1, 2026-05-27, 5d
    Connection バッファ       :a2, after a1, 3d
    結合 調整                 :a3, after b2, 2d
    
    section B層
    Parser Message            :b1, 2026-05-27, 3d
    CommandDispatcher         :b2, after b1, 5d
    ReplyBuilder              :b3, after b2, 3d
    
    section C1層
    Client                    :c1, 2026-05-27, 3d
    ServerState               :c2, after c1, 4d
    
    section C2層
    Channel                   :c3, 2026-05-27, 3d
    ChannelModes              :c4, after c3, 3d
    
    section 統合
    MVP統合テスト             :t1, after a3, 3d
    コマンド実装              :t2, after t1, 7d
    最終テスト                :t3, after t2, 3d
```

---

## フェーズ別作業

```mermaid
flowchart LR
    subgraph PHASE1["Phase 1: 基盤"]
        direction TB
        A1["A: Server/poll()"]
        B1["B: Parser/Message"]
        C1_1["C1: Client"]
        C2_1["C2: Channel"]
    end
    
    subgraph PHASE2["Phase 2: 中核"]
        direction TB
        A2["A: Connection"]
        B2["B: Dispatcher"]
        C1_2["C1: ServerState"]
        C2_2["C2: ChannelModes"]
    end
    
    subgraph PHASE3["Phase 3: 統合"]
        direction TB
        MVP["MVP統合<br/>PASS/NICK/USER<br/>JOIN/PRIVMSG"]
    end
    
    subgraph PHASE4["Phase 4: 完成"]
        direction TB
        CMD["全コマンド実装<br/>KICK/INVITE<br/>TOPIC/MODE"]
    end
    
    PHASE1 --> PHASE2
    PHASE2 --> PHASE3
    PHASE3 --> PHASE4

    style A1 fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style A2 fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style B1 fill:#50B878,stroke:#3A8A5A,color:#fff
    style B2 fill:#50B878,stroke:#3A8A5A,color:#fff
    style C1_1 fill:#F5A623,stroke:#C4841C,color:#fff
    style C1_2 fill:#F5A623,stroke:#C4841C,color:#fff
    style C2_1 fill:#795548,stroke:#5D4037,color:#fff
    style C2_2 fill:#795548,stroke:#5D4037,color:#fff
    style MVP fill:#9C27B0,stroke:#7B1FA2,color:#fff
    style CMD fill:#9C27B0,stroke:#7B1FA2,color:#fff
```

---

## 並行作業マトリクス

| Phase | torinoue (A+B) | taro (C1) | hanako (C2) | 並行可能？ |
|-------|----------------|-----------|-------------|-----------|
| 1 | Server基盤, Parser | Client | Channel | ✅ 全員並行 |
| 2 | Connection, Dispatcher | ServerState | ChannelModes | ✅ 全員並行 |
| 3 | 結合・調整 | レビュー | レビュー | ⚠️ torinoue主導 |
| 4 | コマンド実装 | テスト支援 | テスト支援 | ⚠️ torinoue主導 |

---

## クリティカルパス（PDM図）

```mermaid
flowchart LR
    subgraph CRITICAL["クリティカルパス（60h）"]
        direction LR
        A1["A: Server基盤<br/>8h"] --> A2["A: Connection<br/>7h"]
        A2 --> B1["B: Parser<br/>5h"]
        B1 --> B2["B: Dispatcher<br/>10h"]
        B2 --> B3["B: ReplyBuilder<br/>5h"]
        B3 --> INT["統合テスト<br/>5h"]
        INT --> CMD["コマンド実装<br/>15h"]
        CMD --> TEST["最終テスト<br/>5h"]
    end

    subgraph PARALLEL_C1["C1パス（10h）"]
        direction LR
        C1_1["C1: Client<br/>4h"] --> C1_2["C1: ServerState<br/>6h"]
    end

    subgraph PARALLEL_C2["C2パス（8h）"]
        direction LR
        C2_1["C2: Channel<br/>4h"] --> C2_2["C2: ChannelModes<br/>4h"]
    end

    C1_2 -->|"待ち"| INT
    C2_2 -->|"待ち"| INT

    style CRITICAL fill:#FFEBEE,stroke:#EF5350
    style A1 fill:#FF5722,stroke:#E64A19,color:#fff
    style A2 fill:#FF5722,stroke:#E64A19,color:#fff
    style B1 fill:#FF5722,stroke:#E64A19,color:#fff
    style B2 fill:#FF5722,stroke:#E64A19,color:#fff
    style B3 fill:#FF5722,stroke:#E64A19,color:#fff
    style INT fill:#9C27B0,stroke:#7B1FA2,color:#fff
    style CMD fill:#FF5722,stroke:#E64A19,color:#fff
    style TEST fill:#FF5722,stroke:#E64A19,color:#fff
    
    style PARALLEL_C1 fill:#FFF3E0,stroke:#FFB74D
    style PARALLEL_C2 fill:#EFEBE9,stroke:#A1887F
    style C1_1 fill:#F5A623,stroke:#C4841C,color:#fff
    style C1_2 fill:#F5A623,stroke:#C4841C,color:#fff
    style C2_1 fill:#795548,stroke:#5D4037,color:#fff
    style C2_2 fill:#795548,stroke:#5D4037,color:#fff
```

---

## パス別工数

| パス | タスク | 工数 | 累計 |
|------|--------|------|------|
| **クリティカル** | A: Server基盤 | 8h | 8h |
| | A: Connection | 7h | 15h |
| | B: Parser | 5h | 20h |
| | B: Dispatcher | 10h | 30h |
| | B: ReplyBuilder | 5h | 35h |
| | 統合テスト | 5h | 40h |
| | コマンド実装 | 15h | 55h |
| | 最終テスト | 5h | **60h** |
| **C1パス** | C1: Client | 4h | 4h |
| | C1: ServerState | 6h | **10h** |
| **C2パス** | C2: Channel | 4h | 4h |
| | C2: ChannelModes | 4h | **8h** |

---

## 統合ポイントと待ち関係

```mermaid
flowchart TB
    subgraph WAIT["統合待ち状況"]
        direction TB
        B_done["B: ReplyBuilder完了<br/>（35h時点）"]
        C1_done["C1: ServerState完了<br/>（10h時点）"]
        C2_done["C2: ChannelModes完了<br/>（8h時点）"]
        
        B_done --> MERGE["統合開始"]
        C1_done -->|"25h待ち"| MERGE
        C2_done -->|"27h待ち"| MERGE
    end

    style B_done fill:#50B878,stroke:#3A8A5A,color:#fff
    style C1_done fill:#F5A623,stroke:#C4841C,color:#fff
    style C2_done fill:#795548,stroke:#5D4037,color:#fff
    style MERGE fill:#9C27B0,stroke:#7B1FA2,color:#fff
```

**結論:**
- C1/C2は**早期完了**するが、B層完了まで統合待ち
- C1/C2担当者は待ち時間に**レビュー・テスト支援**に回れる
- **ボトルネック: torinoue（A+B）** — ここの遅延が全体に直結

---

## リスクと対策

| リスク | 影響 | 対策 |
|--------|------|------|
| A+B担当の過負荷 | 全体遅延 | C1/C2からのレビュー・テスト支援 |
| インターフェース不一致 | 統合時の手戻り | 早期にinterface.md確認・合意 |
| RFC理解不足 | コマンド仕様誤り | クイズ・チェックリストで確認 |

---

## 色凡例

| 色 | HEX | 意味 |
|----|-----|------|
| 🔴 赤 | #FF5722 | クリティカルパス上のタスク |
| 🟠 オレンジ | #F5A623 | C1層作業（並行パス） |
| 🤎 茶 | #795548 | C2層作業（並行パス） |
| 🟣 紫 | #9C27B0 | 統合ポイント |
| 🟢 緑 | #50B878 | B層完了マイルストーン |
