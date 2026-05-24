# 読書ガイド: B担当（Protocol / Command）

> 対象: B担当（Parser, CommandDispatcher, ReplyBuilder, Message）
> 作成日: 2026-05-23
> ステータス: **次回セッションで詳細化予定**

---

## 概要

B担当はIRCプロトコル層を担当。ソケット書籍は**直接関係しない**。
主な学習リソースはRFC 1459/2812とdesign.md/interface.md。

---

## 書籍との関係

| 章 | 関係度 | 理由 |
|----|--------|------|
| 1章（概念） | ★☆☆ | TCP/IPの基礎理解として |
| 6.1（バッファリング） | ★☆☆ | メッセージ境界が保持されない理由の理解 |
| その他 | ☆☆☆ | 直接関係なし |

**ソケット書籍は必須ではない。** A担当の実装を理解するための背景知識程度。

---

## 主要リソース

| リソース | 該当箇所 | 内容 |
|---------|----------|------|
| RFC 1459 | Section 2 | メッセージフォーマット（prefix, command, params） |
| RFC 2812 | Section 2 | 詳細なメッセージ仕様 |
| interface.md | Section 6 | Parser, CommandDispatcher, ReplyBuilder の仕様 |
| design.md | Section 3.2 | Protocol/Command層の責務 |

---

## 担当クラス

```mermaid
flowchart LR
    Line["complete line"] --> Parser
    Parser --> Message["Message構造体"]
    Message --> Dispatcher["CommandDispatcher"]
    Dispatcher --> Reply["ReplyBuilder"]
    Reply --> Result["CommandResult"]

    style Line fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Parser fill:#50B878,stroke:#3A8A5A,color:#fff
    style Message fill:#50B878,stroke:#3A8A5A,color:#fff
    style Dispatcher fill:#50B878,stroke:#3A8A5A,color:#fff
    style Reply fill:#50B878,stroke:#3A8A5A,color:#fff
    style Result fill:#50B878,stroke:#3A8A5A,color:#fff
```

| クラス | 役割 |
|--------|------|
| Message | パース済IRCメッセージ構造体 |
| Parser | 1行 → Message変換 |
| CommandDispatcher | コマンド実行、CommandResult返却 |
| ReplyBuilder | Numeric Reply等の返信文字列生成 |

---

## TODO: 次回セッションで詳細化

- [ ] RFC 1459/2812 の読むべきセクション特定
- [ ] 各コマンド（PASS, NICK, USER, JOIN, PRIVMSG等）の仕様整理
- [ ] Numeric Reply の一覧作成
- [ ] B担当向けクイズ作成
