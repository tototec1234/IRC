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

## 【スコープ】通信構成（C2S: Client to Server）
> アーキテクチャ: どういった構成で使用（レビュー）される？

```mermaid
flowchart LR
    subgraph Clients["IRCクライアント"]
        irssi["irssi<br/>（リファレンス）"]
        nc["nc<br/>（テスト用）"]
    end

    subgraph ircserv["ircserv（本課題の成果物）"]
        Internal["内部処理<br/>（下図参照）"]
    end

    subgraph NotImplemented["実装禁止"]
        OtherServer["他IRCサーバー<br/>（S2S通信）"]
    end

    irssi <-->|TCP/IP<br/>IRC Protocol| ircserv
    nc <-->|TCP/IP<br/>IRC Protocol| ircserv
    ircserv x--x|S2S禁止| OtherServer
```

## 【仕様】ircserv 内部構成
> アーキテクチャ: どう構成する？ 
```mermaid
flowchart TB
    subgraph A["A担当: Network / IO"]
        Poll["poll()"]
        Server["Server<br/>イベント振り分け"]
        Conn["Connection<br/>recv/send buffer"]
    end

    subgraph B["B担当: Protocol / Command"]
        Parser["Parser<br/>line → Message"]
        Dispatcher["CommandDispatcher<br/>command実行"]
        Reply["ReplyBuilder<br/>返信生成"]
    end

    subgraph C1["C1担当: Client / ServerState"]
        State["ServerState<br/>fd/nick/channel辞書"]
        Client["Client<br/>登録状態/nick/user"]
    end

    subgraph C2["C2担当: Channel"]
        Channel["Channel<br/>members/operators/topic"]
        Modes["ChannelModes<br/>+i/+t/+k/+l"]
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
    Server -->|queueSend| Conn
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
