# Chapter IV: Mandatory Part

> 原文: ft_irc.pdf (Version 10.0) - Pages 6-9

---

## プログラム仕様

| 項目 | 内容 |
|------|------|
| プログラム名 | `ircserv` |
| 提出ファイル | Makefile, *.{h, hpp}, *.cpp, *.tpp, *.ipp, オプションで設定ファイル |
| Makefile ルール | `NAME`, `all`, `clean`, `fclean`, `re` |
| 引数 | `port`: リッスンポート / `password`: 接続パスワード |
| 説明 | C++ 98 で実装する IRC サーバー |

*原文: Program Name: ircserv / Files to Submit: Makefile, \*.{h, hpp}, \*.cpp, \*.tpp, \*.ipp, an optional configuration file / Makefile: NAME, all, clean, fclean, re / Arguments: port: The listening port, password: The connection password / Description: An IRC server in C++ 98*

---

## 使用可能な外部関数

C++ 98 の全機能に加え、以下の関数が使用可能：

*原文: External Function: Everything in C++ 98.*

```
socket, close, setsockopt, getsockname,
getprotobyname, gethostbyname, getaddrinfo,
freeaddrinfo, bind, connect, listen, accept,
htons, htonl, ntohs, ntohl, inet_addr, inet_ntoa,
inet_ntop, send, recv, signal, sigaction,
sigemptyset, sigfillset, sigaddset, sigdelset,
sigismember, lseek, fstat, fcntl, poll (or equivalent)
```

---

## 実行方法

実行ファイルは以下のように実行される：

*原文: Your executable will be run as follows:*

```bash
./ircserv <port> <password>
```

- **port**: IRCサーバーが着信IRC接続をリッスンするポート番号

  *原文: port: The port number on which your IRC server will be listening for incoming IRC connections.*

- **password**: 接続パスワード。サーバーに接続しようとするすべてのIRCクライアントに必要

  *原文: password: The connection password. It will be needed by any IRC client that tries to connect to your server.*

---

## poll() の代替について

> **注意**: 課題書と評価スケールでは `poll()` が言及されているが、`select()`, `kqueue()`, `epoll()` などの同等の機能を使用してもよい。

*原文: Even though poll() is mentioned in the subject and the evaluation scale, you may use any equivalent such as select(), kqueue(), or epoll().*

---

## IV.1 Requirements（要件）

### 複数クライアントの同時処理

サーバーは複数のクライアントを同時に処理でき、ハングしてはならない。

*原文: The server must be capable of handling multiple clients simultaneously without hanging.*

### Fork 禁止・ノンブロッキング I/O

Fork は禁止。すべての I/O 操作はノンブロッキングでなければならない。

*原文: Forking is prohibited. All I/O operations must be non-blocking.*

### poll() の使用

すべての操作（read, write, listen 等）を処理するために、**1つの poll()（または同等のもの）のみ** を使用できる。

*原文: Only 1 poll() (or equivalent) can be used for handling all these operations (read, write, but also listen, and so forth).*

> **警告**: ノンブロッキングファイルディスクリプタを使用する必要があるため、`poll()`（または同等のもの）を使用せずに `read/recv` や `write/send` を使用することは技術的に可能であり、サーバーはブロックしない。しかし、これはより多くのシステムリソースを消費する。したがって、`poll()`（または同等のもの）を使用せずにいかなるファイルディスクリプタに対しても `read/recv` や `write/send` を試みた場合、評価は **0点** となる。

*原文: Because you have to use non-blocking file descriptors, it is possible to use read/recv or write/send functions with no poll() (or equivalent), and your server wouldn't be blocking. However, it would consume more system resources. Therefore, if you attempt to read/recv or write/send in any file descriptor without using poll() (or equivalent), your grade will be 0.*

### リファレンスクライアント

複数のIRCクライアントが存在する。その中から1つを**リファレンス**として選択すること。リファレンスクライアントは評価プロセスで使用される。

*原文: Several IRC clients exist. You have to choose one of them as a reference. Your reference client will be used during the evaluation process.*

リファレンスクライアントはエラーなくサーバーに接続できなければならない。

*原文: Your reference client must be able to connect to your server without encountering any error.*

### 通信プロトコル

クライアントとサーバー間の通信は **TCP/IP (v4 または v6)** で行うこと。

*原文: Communication between client and server has to be done via TCP/IP (v4 or v6).*

### 実装すべき機能

リファレンスクライアントとサーバーの使用は、公式IRCサーバーでの使用と同様でなければならない。ただし、以下の機能のみを実装すること：

*原文: Using your reference client with your server must be similar to using it with any official IRC server. However, you only have to implement the following features:*

#### 基本機能

- 認証、ニックネーム設定、ユーザー名設定、チャンネル参加、リファレンスクライアントを使用したプライベートメッセージの送受信ができること

  *原文: You must be able to authenticate, set a nickname, a username, join a channel, send and receive private messages using your reference client.*

- 1つのクライアントからチャンネルに送信されたすべてのメッセージは、そのチャンネルに参加している他のすべてのクライアントに転送されること

  *原文: All the messages sent from one client to a channel have to be forwarded to every other client that joined the channel.*

- オペレーターと一般ユーザーが存在すること

  *原文: You must have operators and regular users.*

#### チャンネルオペレーター専用コマンド

以下のコマンドはチャンネルオペレーター専用として実装すること：

*原文: Then, you have to implement the commands that are specific to channel operators:*

| コマンド | 機能 |
|----------|------|
| **KICK** | クライアントをチャンネルから追放する |
| **INVITE** | クライアントをチャンネルに招待する |
| **TOPIC** | チャンネルトピックを変更または表示する |
| **MODE** | チャンネルモードを変更する |

*原文: KICK - Eject a client from the channel / INVITE - Invite a client to a channel / TOPIC - Change or view the channel topic / MODE - Change the channel's mode*

#### MODE コマンドのオプション

| オプション | 機能 |
|------------|------|
| `i` | 招待専用チャンネルの設定/解除 |
| `t` | TOPIC コマンドをチャンネルオペレーターに制限する設定/解除 |
| `k` | チャンネルキー（パスワード）の設定/解除 |
| `o` | チャンネルオペレーター権限の付与/剥奪 |
| `l` | チャンネルのユーザー数制限の設定/解除 |

*原文: i: Set/remove Invite-only channel / t: Set/remove the restrictions of the TOPIC command to channel operators / k: Set/remove the channel key (password) / o: Give/take channel operator privilege / l: Set/remove the user limit to channel*

### クリーンコード

当然ながら、クリーンなコードを書くことが期待される。

*原文: Of course, you are expected to write a clean code.*

---

## IV.2 For MacOS only

MacOS は他の Unix OS と同じ方法で `write()` を実装していないため、`fcntl()` の使用が許可される。

*原文: Since MacOS does not implement write() in the same way as other Unix OSes, you are permitted to use fcntl().*

ノンブロッキングモードでファイルディスクリプタを使用し、他の Unix OS と同様の動作を得ること。

*原文: You must use file descriptors in non-blocking mode in order to get a behavior similar to the one of other Unix OSes.*

ただし、`fcntl()` は以下の形式でのみ使用可能：

*原文: However, you are allowed to use fcntl() only as follows:*

```c
fcntl(fd, F_SETFL, O_NONBLOCK);
```

他のフラグは禁止。

*原文: Any other flag is forbidden.*

---

## IV.3 Test example（テスト例）

受信データの部分受信や低帯域幅など、あらゆるエラーや問題を検証すること。

*原文: Verify every possible error and issue, such as receiving partial data, low bandwidth, etc.*

サーバーが送信されたすべてのデータを正しく処理することを確認するため、`nc` を使用した以下の簡単なテストを実行できる：

*原文: To ensure that your server correctly processes all data sent to it, the following simple test using nc can be performed:*

```bash
$> nc -C 127.0.0.1 6667
com^Dman^Dd
$>
```

`ctrl+D` を使用してコマンドを複数の部分に分けて送信する：'com'、次に 'man'、次に 'd\n'。

*原文: Use ctrl+D to send the command in several parts: 'com', then 'man', then 'd\n'.*

コマンドを処理するには、まず受信したパケットを集約して再構築する必要がある。

*原文: In order to process a command, you have to first aggregate the received packets in order to rebuild it.*

---

## 技術用語

| 英語 | 日本語 | 説明 |
|------|--------|------|
| non-blocking I/O | ノンブロッキング I/O | I/O操作が完了を待たずに即座に戻る方式 |
| poll() | poll() | 複数のファイルディスクリプタを監視するシステムコール |
| file descriptor (fd) | ファイルディスクリプタ | OSがファイルやソケットを識別する整数 |
| channel operator | チャンネルオペレーター | チャンネル管理権限を持つユーザー |
| reference client | リファレンスクライアント | 動作確認の基準とするIRCクライアント |
| TCP/IP | TCP/IP | インターネットの標準通信プロトコル |

---

## 実装必須項目チェックリスト

### 基本要件

- [ ] 複数クライアント同時接続（ハングなし）
- [ ] Fork 禁止
- [ ] すべての I/O はノンブロッキング
- [ ] poll()（または同等）を1つだけ使用
- [ ] TCP/IP (v4 or v6) で通信
- [ ] リファレンスクライアントがエラーなく接続可能

### 基本機能

- [ ] 認証（PASS コマンド）
- [ ] ニックネーム設定（NICK コマンド）
- [ ] ユーザー名設定（USER コマンド）
- [ ] チャンネル参加（JOIN コマンド）
- [ ] プライベートメッセージ送受信（PRIVMSG コマンド）
- [ ] チャンネルメッセージの全員への転送

### オペレーター/ユーザー

- [ ] オペレーターと一般ユーザーの区別

### チャンネルオペレーターコマンド

- [ ] KICK（追放）
- [ ] INVITE（招待）
- [ ] TOPIC（トピック変更/表示）
- [ ] MODE i（招待専用）
- [ ] MODE t（TOPIC制限）
- [ ] MODE k（チャンネルキー）
- [ ] MODE o（オペレーター権限）
- [ ] MODE l（ユーザー数制限）

### 部分データ処理

- [ ] 部分受信データの集約・再構築
