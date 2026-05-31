# 読書ガイド: A担当（Network / IO）

> 対象: A担当（Server, Connection, Poller）
> 作成日: 2026-05-23
> ステータス: 確定

---

## 概要

A担当は書籍「TCP/IPソケットプログラミング C言語編」が**最も重要**。
ソケット操作、ノンブロッキングI/O、poll()による多重化を担当する。

---

## 書籍の学習順序

```mermaid
flowchart LR
    Ch1["1章<br/>概念"] --> Ch2["2章<br/>TCP基礎"]
    Ch2 --> Ch3["3章<br/>バイト順"]
    Ch3 --> Ch5_3["5.3.1<br/>ノンブロッキング"]
    Ch5_3 --> Ch5_5["5.5<br/>多重化"]
    Ch5_5 --> Ch6["6章<br/>TCP挙動"]
    Ch6 --> Practice["実践<br/>design.md §10"]

    style Ch1 fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Ch2 fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Ch3 fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Ch5_3 fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Ch5_5 fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Ch6 fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Practice fill:#4A90D9,stroke:#2E5A8B,color:#fff
```

---

## 読むべき章・節（詳細）

| 優先度 | 章/節 | 内容 | 時間目安 | チェック |
|--------|-------|------|----------|----------|
| ★★★ | **2章 全体** | socket/bind/listen/accept/send/recv | 3h | [ ] |
| ★★★ | **5.5 多重化** | select()の概念 → poll()へ読み替え | 3h | [ ] |
| ★★☆ | **5.3.1 ノンブロッキングソケット** | fcntl(O_NONBLOCK), EAGAIN | 2h | [ ] |
| ★★☆ | **6.1 TCPにおけるバッファリング** | send/recvバッファの仕組み | 1h | [ ] |
| ★★☆ | **6.4 TCPソケットのライフサイクル** | 3-wayハンドシェイク、切断検知 | 1h | [ ] |
| ★☆☆ | 1章 全体 | TCP/IP基礎概念 | 1h | [ ] |
| ★☆☆ | 3.1-3.2 | htons/ntohs、バイト順 | 1h | [ ] |

---

## スキップする章・節

| 章/節 | 理由 |
|-------|------|
| **第4章 全体** | UDP。ft_ircはTCPのみ |
| 5.3.2 非同期I/O | SIGIO。poll()を使う |
| 5.3.3 タイムアウト | setsockopt。ノンブロッキング+poll()で対応 |
| 5.4.1-5.4.2 | fork/thread。ft_ircはfork禁止 |
| 5.6 | ブロードキャスト/マルチキャスト。IRC不要 |
| 第7章 | DNS。必須ではない |

---

## サンプルコード対応

### 読むべきコード

| ファイル | 内容 | URL |
|---------|------|-----|
| TCPEchoServer.c | socket/bind/listen/accept基礎 | [Link](http://cs.baylor.edu/~donahoo/practical/CSockets/code/TCPEchoServer.c) |
| HandleTCPClient.c | recv/send基礎 | [Link](http://cs.baylor.edu/~donahoo/practical/CSockets/code/HandleTCPClient.c) |
| TCPEchoServer-Select.c | 多重化の概念 | [Link](http://cs.baylor.edu/~donahoo/practical/CSockets/code/TCPEchoServer-Select.c) |

### スキップするコード

UDP系、Fork系、Thread系、SIGIO系、Broadcast/Multicast系

---

## 最重要: poll() 実践（bircd カリキュラム + design.md §10）

**書籍には poll() のサンプルがない。** 次の順で学ぶ。

1. `bircd/bircd_learning_curriculum.md` — C の bircd で poll / バッファを体験
2. `design.md` Section 10.2 — A 層の実装要件（POLLOUT、EAGAIN、責務分離）
3. `ref_interface.md` Section 5 — Server / Connection の API
4. `IRC_torinoue/src/` — 上記に従い新規実装

### 書籍の章と A 層実装の対応

| 書籍の章 | A 層で実装する箇所（ref_interface / design.md §10） |
|---------|-----------------------------------------------------|
| 2章: ソケット基礎 | Server 起動（socket / bind / listen） |
| 2章: accept | accept → Connection 生成 |
| 2章: send/recv | `Connection::readFromSocket()` / `writeToSocket()` |
| 3章: バイト順 | `htons()` 等 |
| 5章: ノンブロッキング | 全 fd で `fcntl(O_NONBLOCK)` |
| 5章: 多重化 | `Server::run()` 内の単一 `poll()` |
| 6章: バッファリング | Connection の recv/send buffer |
| 6章: 切断検知 | `POLLERR` / `POLLHUP` / recv=0 → disconnect |

---

## 重要な概念図

### 送受信バッファ

```
アプリケーション（ユーザー空間）
        │
        │ send(fd, data, len, 0)
        ▼
┌─────────────────────────────┐
│  カーネル空間                 │
│  ┌───────────────────────┐  │
│  │ TCP送信バッファ         │  │  ← send()でここにコピー
│  └───────────────────────┘  │
│        │                    │
│        │ カーネルが送信       │
│        ▼                    │
│    ネットワーク               │
└─────────────────────────────┘
```

### poll() ループの流れ

```mermaid
flowchart TD
    Start["poll()"] --> Check{"イベント発生?"}
    Check -->|listenFd| Accept["accept() → 新規fd追加"]
    Check -->|clientFd POLLIN| Recv["recv() → バッファに蓄積"]
    Check -->|clientFd POLLOUT| Send["send() → バッファから送信"]
    Check -->|POLLERR/POLLHUP| Disconnect["切断処理"]
    Accept --> Start
    Recv --> Start
    Send --> Start
    Disconnect --> Start

    style Start fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Check fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Accept fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Recv fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Send fill:#4A90D9,stroke:#2E5A8B,color:#fff
    style Disconnect fill:#FF5722,stroke:#E64A19,color:#fff
```

---

## リソース

| リソース | URL |
|---------|-----|
| オーム社 書籍ページ | https://www.ohmsha.co.jp/book/9784274065194/ |
| 原著者サンプルコード | http://cs.baylor.edu/~donahoo/practical/CSockets/textcode.html |
| man poll | `man 2 poll` |
| bircd カリキュラム | `dev_docs/../bircd/bircd_learning_curriculum.md` |
| design.md §10 | `dev_docs/design.md` |
| ref_interface §5 | `dev_docs/ref_interface.md` |
