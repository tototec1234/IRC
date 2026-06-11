# bircd 学習カリキュラム

> 目標: bircd を出発点として、poll() + バッファリング付きサーバーを自力実装できるようになる
> 想定時間: 10-14時間
> 前提: C言語の基礎知識

> **用語:** 本ドキュメントの **Lesson** = bircd 学習カリキュラムの段階。**Phase** = ircserv A 層の実装フェーズ（[`a_implementation_plan.md`](../a_header_tmp/a_implementation_plan.md)）。混同しないこと。対応表は [bircd_lesson_ircserv_phase_map.md](bircd_lesson_ircserv_phase_map.md)。


---

## 全体像

```
bircd (select)  ───────────▶  Server.cpp相当 (poll)
     │                           │
     │  学べる                    │ 学べない（自分で埋める）   
     ├─ socket/bind/listen       ├─ poll()              
     ├─ accept                   ├─ ノンブロッキングI/O    
     ├─ recv/send                ├─ 受信バッファリング     
     ├─ select()                 ├─ 送信バッファリング     
     └─ コールバック設計            └─ POLLOUT動的制御      
```

---

## Lesson 1: bircd 完全理解（2-3時間）

### 目標
- bircd のコードを読み、処理フローを説明できる
- socket/bind/listen/accept の役割を説明できる

### 手順

#### Lesson 1.1 ビルドと動作確認（15分）

```bash
cd IRC_torinoue/bircd
make
./bircd 6667
```

`bind error (srv_create.c, 21): Address already in use` が出たときは [対処法][^bind-eaddrinuse]。

別ターミナルで:
```bash
nc localhost 6667   # クライアント1
nc localhost 6667   # クライアント2（別ターミナル）
```

クライアント1で文字を打つ → クライアント2に届く（ブロードキャスト）

[^bind-eaddrinuse]: ポート 6667 が既に使用中（前回の `bircd` が残っている、または別プロセスが LISTEN 中）。占有プロセスを確認する:

    ```bash
    # 基本（これで十分）
    lsof -i :6667

    # プロトコルまで明示（よく使う）
    lsof -iTCP:6667

    # LISTEN 中だけ（サーバプロセス特定向け）
    lsof -iTCP:6667 -sTCP:LISTEN

    # ホスト名逆引きを切る（速い・見やすい）
    lsof -nP -iTCP:6667

	ps aux | grep bircd
    ```

    出力の `PID` 列のプロセスを止める: `kill <PID>`（効かなければ `kill -9 <PID>`）。または別ポートで起動: `./bircd 6668`（`nc` 側のポートも合わせる）。`lsof` の `-i` にロングオプションはない（macOS / BSD 版）。

#### Lesson 1.2 コードリーディング（1-2時間）

**読む順番:**

| 順 | ファイル | 行数 | 内容 |
|----|---------|-----|------|
| 1 | [`bircd.h`](../../forRev/06031232/bircd/bircd.h) | 52 | データ構造の全体像 |
| 2 | [`main.c`](../../forRev/06031232/bircd/main.c) | ~20 | エントリーポイント |
| 3 | [`srv_create.c`](../../forRev/06031232/bircd/srv_create.c) | 23 | サーバーソケット作成 |
| 4 | [`main_loop.c`](../../forRev/06031232/bircd/main_loop.c) | 13 | メインループ構造 |
| 5 | [`init_fd.c`](../../forRev/06031232/bircd/init_fd.c) | 28 | fd_set 初期化 |
| 6 | [`do_select.c`](../../forRev/06031232/bircd/do_select.c) | 9 | select() 呼び出し |
| 7 | [`check_fd.c`](../../forRev/06031232/bircd/check_fd.c) | 21 | イベントディスパッチ |
| 8 | [`srv_accept.c`](../../forRev/06031232/bircd/srv_accept.c) | 22 | 接続受付 |
| 9 | [`client_read.c`](../../forRev/06031232/bircd/client_read.c) | 31 | データ受信・ブロードキャスト |

**読みながら確認すること:**
- [ ] `t_fd` 構造体の各フィールドの役割
- [ ] `t_env` 構造体の各フィールドの役割
- [ ] `fds[fd番号]` という直接インデックスの設計

#### Lesson 1.3 フロー図作成（30分）

以下を自分で図示する:

```
main()
  │
  ├─ init_env()
  ├─ get_opt()
  ├─ srv_create()
  │     └─ socket → bind → listen
  │
  └─ main_loop() [無限ループ]
        │
        ├─ init_fd()      ← fd_set に監視対象を設定
        ├─ do_select()    ← ブロック待機
        └─ check_fd()     ← イベント処理
              │
              ├─ サーバーfd POLLIN → srv_accept()
              └─ クライアントfd POLLIN → client_read()
```

### チェックリスト

- [ ] `socket(PF_INET, SOCK_STREAM, ...)` が何を返すか説明できる

		p29 成功時ファイルディスクリプタ（この文脈ではソケットデスクリプタ）の値、失敗時　-1

	質問: ファイルディスクリプタとソケットディスクリプタは同じもの？区別ある？
	自分の回答: 同じもの、ソケットディスクリプタはネットワーク通信用のファイルディスクリプタとして捉える時の呼び方
	
	正誤: ✅ 正解
	解説: Unix/Linux では「すべてがファイル」という設計思想。ソケットも内部的にはファイルディスクリプタとして管理される。型は同じ `int`。通常ファイル (0,1,2,...) もソケットも同じ fd 空間を共有。区別は実装レベルでカーネルが管理するが、ユーザー空間では同じ整数値として扱う

- [ ] `bind()` が何をするか説明できる

		p39 bind(int socket, const struct sockaddr *address, socklen_t address_len)
		socketで指定したディスクリプタを割り当てられたソケットを、addressに関連づける関数。成功したら0、失敗で-1を返す

	質問: 「関連づける」の具体的な意味は？何と何を結びつけている？
	自分の回答:soketと外部から呼び出す時のIPアドレスおよびポート番号を結びつけている。
	
	正誤: ✅ 正解
	解説: `bind()` はソケットをローカルアドレス（IPアドレス + ポート番号）に結びつける。これにより、外部から「このサーバーの IP:port」に接続できるようになる。サーバーソケットでは必須。クライアントソケットでは通常省略（カーネルが自動割り当て）


- [ ] `listen()` の第2引数（backlog）の意味を説明できる

		p40 接続応答を同時にいくつまでリスンできるか

	質問: backlog が管理するのは何の接続？
	自分の回答:backlogが管理するのは、「まだ accept() されていない、TCPの3ウェイ・ハンドシェイクが完了して待機状態にある接続（コネクション）」 のキュー（行列）の長さ
	
	正誤: ✅ ほぼ正解
	解説: 完全に確立された接続（established connections）が格納される**完了キュー**の最大長を指定。厳密には2つのキューがある:
	1. **SYN キュー**: 3ウェイハンドシェイク途中（SYN受信、SYN-ACK送信済み）
	2. **完了キュー（accept キュー）**: ハンドシェイク完了、`accept()` 待ち ← backlog はこっち
	backlog を超えると新規接続が拒否される（ECONNREFUSED）

- [ ] `accept()` が何を返すか説明できる

     	p40 lp1222　accept(int クライアントがconnect()でコネクションを確立済みのピアソケットのfd, struct sockaddr *クライアントのアドレス, socklen_t *restrict アドレスを入力した時に実際に使用したバイト数)はクライアントを接続させた「新しいソケットのディスクリプタ」を返す（第一引数のソケットは引き続き新しい接続をリスンし続ける）。　失敗時には-1を返す。

	質問: なぜ「新しい」ソケットが必要？元のソケット使えない理由は？
	自分の回答:元のソケット（リスニングソケット）の役割は「次の新しいお客さん（接続）を待つこと」に専念するため使えない。　捕まえたお客さんには新しいソケットで対応する必要がある。
	
	正誤: ✅ 正解
	解説: 役割分担の設計。
	- **リスニングソケット（元）**: 新規接続受付専用。状態は LISTEN のまま。複数クライアント対応のため常に待機
	- **接続ソケット（新）**: 個別クライアントとの通信専用。状態は ESTABLISHED。各クライアントごとに1つ
	レストランの例: 受付係（リスニングソケット）は入口で待機、各お客に担当ウェイター（接続ソケット）を割り当て

- [ ] `FD_ZERO`, `FD_SET`, `FD_ISSET` の役割を説明できる

		p109 いずれもディスクリプタ（以下、fdと略）のリストを操作するマクロ
		FD_ZERO(fd_set *fdset);		:fdsetにあるfd全て削除
		FD_SET(fd, fd_set *fdset);	:fdをfdsetに追加
		FD_ISSET(fd, fd_set *fdset);:fdがfdsetにあるかチェック

	質問: この説明で合ってる？select() の前後でどう使う？
	自分の回答:
	`select()` を呼ぶ前（準備）
		1.  `FD_ZERO(&readfds);` でセットをきれいに初期化
		2. `FD_SET(fd1, &readfds);`, `FD_SET(fd2, &readfds);` で、監視したい fd をセットに登録
	`select()` を呼んだ後（結果確認）
		`s`elect()` から戻ってくると、readfds の中身は「データが届いた fd だけ」に書き換わっている（届かなかった fd は勝手に削除される）。
	そのため、`FD_ISSET` を使ってループで確認。	
	
	正誤: ✅ 正解
	解説: 完璧な理解。`select()` は fd_set を**破壊的に変更**する。そのため実用パターンは:
	```c
	fd_set master_set, working_set;
	FD_ZERO(&master_set);
	FD_SET(fd1, &master_set);
	FD_SET(fd2, &master_set);
	
	while (1) {
	    working_set = master_set;  // コピー（select が書き換えるので）
	    select(maxfd + 1, &working_set, NULL, NULL, NULL);
	    
	    for (int i = 0; i <= maxfd; i++) {
	        if (FD_ISSET(i, &working_set)) {
	            // このfdにデータあり
	        }
	    }
	}
	```

- [ ] `select()` の引数と戻り値を説明できる

		p84 lp1401 
		select(int 監視対象のfdの個数 + 1,
		 fd_set データがすぐに入力可能である場合にチェックされるfdのリスト,
		 fd_set 書き込み可能かを検査したいfdのセット,
         fd_set 例外条件の発生を検査したいfdのセット,
		 struct timeval いずれかのfdがI/O可能になるまでの待機時間);
		 戻り値は、I/Oが可能になったfdの個数、タイムアウトの場合0 、エラーの場合-1を返す。
		
	質問: 第1引数が「個数 + 1」である理由は？なぜそのまま個数ではない？
	自分の回答: fdも配列も0スタートだから
	
	正誤: △ 不十分
	解説: 正しいが説明が不足。正確には:
	- 第1引数 `nfds` = **監視する最大 fd 値 + 1**（個数ではない）
	- `select()` は **0 から nfds-1 まで**の fd をスキャンする
	
	例:
	```c
	// fd = 3, 5, 10 を監視したい
	FD_SET(3, &readfds);
	FD_SET(5, &readfds);
	FD_SET(10, &readfds);
	
	select(11, &readfds, ...);  // 10 + 1 = 11
	// → 0〜10 の範囲をチェック（3個ではなく11が必要）
	```
	
	fd の個数（3個）ではなく、**最大値（10）+ 1** を渡す。これは `select()` の実装が fd をビットマップとして 0 から順にスキャンするため。効率化のため、最大値より上はスキャンしない

### 補助リソース

- 書籍「TCP/IPソケットプログラミング C言語編」2章
- `man 2 socket`, `man 2 bind`, `man 2 listen`, `man 2 accept`
- `man 2 select`

---

## Lesson 2: select → poll 変換（2-3時間）

### 目標
- select() と poll() の違いを説明できる
- bircd を poll() 版に書き換えられる

### Lesson 2.1 select vs poll 比較（30分）

| 項目 | select() | poll() |
|------|----------|--------|
| fd上限 | FD_SETSIZE (1024) | なし |
| 監視対象 | `fd_set` (ビットマスク) | `struct pollfd` 配列 |
| イベント指定 | 読み/書き別の fd_set | `events` フィールド |
| 結果取得 | `FD_ISSET()` | `revents` フィールド |

**変換表:**

```c
// select
fd_set fd_read;
FD_ZERO(&fd_read);
FD_SET(fd, &fd_read);
select(max + 1, &fd_read, NULL, NULL, NULL);
if (FD_ISSET(fd, &fd_read)) { ... }

// poll
struct pollfd fds[MAX];
fds[i].fd = fd;
fds[i].events = POLLIN;
poll(fds, nfds, -1);
if (fds[i].revents & POLLIN) { ... }
```

### Lesson 2.2 poll() のイベントフラグ（15分）

| フラグ | events | revents | 意味 |
|--------|--------|---------|------|
| `POLLIN` | ✓ | ✓ | 読み取り可能 |
| `POLLOUT` | ✓ | ✓ | 書き込み可能 |
| `POLLERR` | - | ✓ | エラー発生 |
| `POLLHUP` | - | ✓ | 切断 |
| `POLLNVAL` | - | ✓ | 無効なfd |

### Lesson 2.3 演習: bircd を poll 版に書き換え（1.5-2時間）

**変更が必要な箇所:**

| ファイル | 変更内容 |
|---------|---------|
| `bircd.h` | `fd_set` → `struct pollfd` 配列 |
| `init_fd.c` | `FD_ZERO/SET` → pollfd配列の初期化 |
| `do_select.c` | `select()` → `poll()` |
| `check_fd.c` | `FD_ISSET` → `revents` チェック |

**ヒント:**

```c
// bircd.h に追加
#include <poll.h>
#define MAX_FDS 1024

typedef struct s_env {
    t_fd           *fds;
    struct pollfd  pollfds[MAX_FDS];  // 追加
    int            nfds;               // 追加
    int            port;
    int            maxfd;
    // fd_set は削除
} t_env;
```

### チェックリスト

- [ ] `struct pollfd` の3フィールド（fd, events, revents）を説明できる
自分の答え:
```cpp
struct pollfd {
    int   fd;        // ← ここに「FDの番号（例: 3や4）」がそのまま入る
    short events;    // ← ここが「ビット列」（何を監視するか）
    short revents;   // ← ここも「ビット列」（何が起きたか）
};
```
- [ ] `POLLIN`, `POLLOUT`, `POLLERR`, `POLLHUP` の違いを説明できる

poll_check.c で実験

| フラグ | events に書く？ | 意味 | IRC サーバーでの典型 |
|--------|-----------------|------|----------------------|
| `POLLIN` (`0000 0001`) | 書く | 読み取り可能 | `accept()` / `recv()` のタイミング |
| `POLLOUT` (`0000 0100`) | 書く | 書き込み可能 | 送信バッファあり → `send()` 続行 |
| `POLLERR` (`0000 1000`) | 書かない | fd でエラー（異常） | 切断処理へ |
| `POLLHUP` (`0001 0000`) | 書かない | ハングアップ（相手切断など） | クライアント落ち。切断処理へ |

- [ ] bircd の poll 版が動作する

### 補助リソース

- `man 2 poll`
- 書籍「TCP/IPソケットプログラミング」5章（書籍には select のみだが、概念は同じ）

---

## Lesson 3: バッファリング追加とディスパッチ骨格の読解（5-6時間）★最難関

### 目標
- TCP がバイトストリームであることを体験的に理解する
- bircd の **FD イベントのディスパッチ機構**（関数ポインタ）の配線を読解できる
- 「呼ばれないコード」（`client_write`）を証拠の連鎖で特定できる
- 受信バッファで `\r\n` 切り出しを実装できる
- 送信バッファで非同期送信を実装できる

> **2026-06-12 改訂:** Lesson 3.1 の答え合わせセッションで2つの論点（FD イベントディスパッチ = 手作り仮想関数、`client_write` = 穴埋め問題の骨格）を発見したため、3.2 / 3.4 を新設し旧 3.2→3.3、旧 3.3→3.5 に繰り下げた。発見の文脈（C++ 対応表）は [Lesson 5.3](#lesson-53-関数ポインタ--仮想関数の対応30分) に収録。

### Lesson 3.1 TCP ストリーム特性の体験（30分）

**問題を体験する実験:**

```bash
# サーバー側（bircd を起動）
./bircd 6667

# クライアント側（大量データ送信）
yes "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcde" | head -1000 | nc localhost 6667
```

> **注意（2026-06-12 実験で判明）:** 分割は**別クライアントの受信データでは目視できない**。
> TCP はバイトストリームであり、recv() の切れ目はデータにマーカーとして残らないため、
> 受信側には継ぎ目なく連結されたバイト列が届くだけ（= それ自体が「境界が消える」ことの証明）。
>
> 分割を観察するなら**サーバー側ログ**で見る:
>
> 1. `client_read.c` の recv() 直後に一時的に `fprintf(stderr, "recv %d bytes\n", r);` を入れる
> 2. `bircd.h` の `BUF_SIZE` を一時的に小さく（例: `100`）すると分割が確実に起きる
> 3. 改行なしの塊を送る: `python3 -c "import sys; sys.stdout.buffer.write(b'A' * 100000)" | nc localhost 6667`
>
> 結果例: サーバー側に `recv 100 bytes` が 1000 回出る（100000 バイトが 1000 分割）。
> 受信側を `nc localhost 6667 > LOG` で保存すると LOG はちょうど 100000 バイト・切れ目なし。
> 最後の `recv 0 bytes` は送信側 nc の切断（EOF）。

**この実験で得られた最大の知見（= Lesson 3 の核心）:**

- 境界がデータに残らない → **アプリ層で境界を作るしかない** → IRC の `\r\n`
- `recv` 1回 = メッセージ1個は**絶対に成立しない**（今回 1メッセージが 1000 回に割れた）
- だから受信バッファに**溜めて** `\r\n` で切り出す実装が必須

**重要な理解:**
```
送信: "PRIVMSG #ch :Hello\r\nPRIVMSG #ch :World\r\n"
        ↓ TCP（バイトストリーム）
recv 1回目: "PRIVMSG #ch :Hel"
recv 2回目: "lo\r\nPRIVMSG #ch"
recv 3回目: " :World\r\n"
```

→ `recv()` の1回 ≠ メッセージの1つ

### Lesson 3.2 FD イベントのディスパッチ骨格の読解（30分）

> **ディスパッチ対象の明示（冗長を承知で）:** ここでディスパッチするのは **FD イベント**（= カーネルが通知するトランスポート層 TCP の状態変化。POLLIN / POLLOUT）。
> **B 層の `CommandDispatcher` とは別物**。あちらのディスパッチ対象は**アプリケーション層（L7）の IRC コマンド**（`PING` / `NICK` / `JOIN`...）。

**課題: `client_read` はどのソースファイルのどこから呼ばれているか？**

**答え: 直接呼ぶ箇所は無い。関数ポインタ経由で2段構え。**

データ構造（`bircd.h`）:

```c
typedef struct s_fd {
    int   type;                              // FD_FREE / FD_SERV / FD_CLIENT
    void  (*fct_read)(struct s_env *, int);  // FD ごとの読み処理スロット
    void  (*fct_write)(struct s_env *, int); // FD ごとの書き処理スロット
    ...
} t_fd;
```

**登録（2箇所）** — FD の種類が決まった瞬間にスロットへ格納:

```c
// srv_create.c:26 — listen ソケット
e->fds[s].fct_read = srv_accept;

// srv_accept.c:19-20 — クライアントソケット
e->fds[cs].fct_read = client_read;
e->fds[cs].fct_write = client_write;
```

**呼び出し（1箇所）** — `check_fd.c` が poll の結果でディスパッチ:

```c
if (e->pollfds[i].revents & POLLIN)
    e->fds[fd].fct_read(e, fd);    // ← 実際の呼び出し元はここだけ
if (e->pollfds[i].revents & POLLOUT)
    e->fds[fd].fct_write(e, fd);
```

**ポイント: 呼び出し側は「どの関数か」を知らない。FD ごとのスロットに入った関数を呼ぶだけ。**
ft_irc で C++ に移すとき、これが仮想関数（ポリモーフィズム）に対応する。C++ 対応表は [Lesson 5.3](#lesson-53-関数ポインタ--仮想関数の対応30分) 参照。

**ircserv での層の責任:** この FD イベントディスパッチは **A 層の責任範囲**。ircserv A 層では fd 種類が2つ（listen / client）しかないため、関数ポインタではなく if 分岐で実現する:

```cpp
if (fd == _listenFd)
    _acceptClient();      // = srv_accept 相当
else
    _handleRead(fd);      // = client_read 相当
```

#### 脚注: FD イベントの正体（層モデルでの位置）

FD イベントは「プロトコル」ではない。**トランスポート層（L4 / TCP）の状態変化を、カーネルがソケット API 経由で通知したもの。**

なお IRC は**プレゼンテーション層ではなくアプリケーション層（L7）**。RFC 1459/2812 はアプリケーション層プロトコル。OSI のプレゼンテーション層（L6）は文字コード変換や TLS 等で、TCP/IP 4層モデルでは L5-L7 をまとめて「アプリケーション層」と呼ぶ。

各 poll イベントは TCP（L4）の状態変化と対応する:

| poll イベント | 裏で起きた TCP（L4）の出来事 |
|---|---|
| POLLIN（listen ソケット） | 3-way ハンドシェイク完了、接続キューに溜まった |
| POLLIN（クライアント fd） | データセグメント到着、カーネル受信バッファに溜まった |
| POLLIN で `recv == 0` | FIN 受信（相手が正常クローズ） |
| POLLOUT | カーネル送信バッファに空きができた（ACK が返って掃けた） |
| POLLERR / POLLHUP | RST 受信や接続破壊 |

ポイント: **fd・poll・POLLIN はどの層のプロトコルにも属さない**。これらは OS のシステムコール API（ソケット API）であって、ネットワーク上を流れるビット列ではない。層モデルは「ワイヤ上のプロトコル」の分類。FD イベントは「カーネル内の TCP 実装が L4 の出来事をアプリに教えるための通知機構」。

```
[アプリ層 L7]  IRC コマンド ──→ CommandDispatcher がディスパッチ（B層）
                 ↑ \r\n で切り出し（A層 popLine）
─── ソケット API（層モデルの外。カーネルとの境界）───
[トランスポート層 L4]  TCP の状態変化 ──→ poll が FD イベントとして通知
                                          → A層が fd 種別でディスパッチ
[ネットワーク層 L3]  IP
```

一次資料: `man 2 poll`、RFC 1122（Requirements for Internet Hosts — 層モデル）、RFC 793（TCP）

### Lesson 3.3 受信バッファリング設計（1時間）

**必要なデータ構造:**

```c
typedef struct s_fd {
    int   type;
    void  (*fct_read)();
    void  (*fct_write)();
    char  buf_read[BUF_SIZE + 1];   // ← これを累積バッファとして使う
    char  buf_write[BUF_SIZE + 1];
    int   buf_read_len;             // 追加: 現在のバッファ長
} t_fd;
```

**処理フロー:**

```
recv() でデータ受信
    │
    ▼
buf_read に追記（累積）
    │
    ▼
buf_read 内に "\r\n" があるか？
    │
    ├─ Yes → "\r\n" までを1メッセージとして切り出し
    │         残りは buf_read に残す
    │         メッセージを処理
    │         (複数あれば繰り返し)
    │
    └─ No → 次の recv() を待つ
```

**実装例:**

```c
void client_read(t_env *e, int cs)
{
    char    tmp[BUF_SIZE];
    int     r;
    char    *pos;

    r = recv(cs, tmp, BUF_SIZE - e->fds[cs].buf_read_len - 1, 0);
    if (r <= 0) {
        close(cs);
        clean_fd(&e->fds[cs]);
        return;
    }

    // 受信データを累積
    strncat(e->fds[cs].buf_read, tmp, r);
    e->fds[cs].buf_read_len += r;

    // \r\n を探して切り出し
    while ((pos = strstr(e->fds[cs].buf_read, "\r\n")) != NULL) {
        *pos = '\0';  // 一旦終端
        
        // ここで1メッセージを処理（今はブロードキャスト）
        broadcast_message(e, cs, e->fds[cs].buf_read);
        
        // 処理済み部分を削除
        int msg_len = pos - e->fds[cs].buf_read + 2;  // +2 for \r\n
        memmove(e->fds[cs].buf_read, pos + 2, 
                e->fds[cs].buf_read_len - msg_len + 1);
        e->fds[cs].buf_read_len -= msg_len;
    }
}
```

### Lesson 3.4 呼ばれない client_write を読む（30分）

**課題: `client_write` は実行時に呼ばれるか？呼ばれないなら、その証拠の連鎖を示せ。**

**答え: 一度も呼ばれない。ただし「呼ばれる配線」は全部ある。**

証拠の連鎖（4段）:

1. **登録はされる**: `srv_accept.c:20` で `fct_write = client_write`
2. **呼ぶコードもある**: `check_fd.c:37-38` で `revents & POLLOUT` なら `fct_write(e, fd)`
3. **だが POLLOUT 監視は条件付き**: `init_fd.c:59-60` — `strlen(buf_write) > 0` のときだけ `events |= POLLOUT`
4. **そして `buf_write` に書き込むコードがどこにも無い**: `client_read` は `recv` → 即 `send` の直結で `buf_write` を経由しない

→ `buf_write` 常に空 → POLLOUT 監視されない → `client_write` 永遠に呼ばれない。中身も空（`client_write.c` は空関数）。

**`client_read` の即 `send` がアンチパターンである理由:**

- 送信先のソケットバッファが満杯なら `send` は**ブロック**する（1クライアントの詰まりが全体を止める）
- ノンブロッキングなら部分送信 / `EAGAIN` が起きるが、対処コードが無い
- ft_irc の評価要件「**全ての read/write は poll（等価物）を1回だけ通す**」に違反する

**42 的な教育的意義 — これは「穴埋め問題」:**

bircd は正解インフラ（`buf_write` スロット、`fct_write` ディスパッチ、条件付き POLLOUT）だけ用意して、実装を空にしている。「設計図だけ見せて、実装はお前がやれ」という形。`client_write.c` の空関数はその印。ft_irc では `client_write` 相当（送信キュー + POLLOUT 駆動の flush）を必ず書く。

**「空なら監視しない」の理由も教材:** POLLOUT は送信バッファに空きがあれば常に立つ。送るものが無いのに監視すると poll が毎回即返ってきて busy loop になる。`init_fd.c:59` の条件はその対策。

### Lesson 3.5 送信バッファリング設計 + 実装演習（1.5時間）

**なぜ必要か:**
- `send()` は全データを送信できるとは限らない（部分送信）
- 送信先が詰まっていると `EAGAIN` が返る

**処理フロー:**

```
送信したいデータ発生
    │
    ▼
buf_write に追記
    │
    ▼
POLLOUT を監視対象に追加
    │
    ▼
poll() で POLLOUT 発生
    │
    ▼
send() で buf_write から送信
    │
    ▼
送信できた分だけ buf_write から削除
    │
    ▼
buf_write が空になったら POLLOUT 監視解除
```

**実装例:**

```c
// メッセージをキューに追加
void queue_message(t_env *e, int cs, const char *msg)
{
    int len = strlen(msg);
    if (e->fds[cs].buf_write_len + len >= BUF_SIZE)
        return;  // バッファ溢れ防止
    
    strcat(e->fds[cs].buf_write, msg);
    e->fds[cs].buf_write_len += len;
    // → init_fd で buf_write_len > 0 なら POLLOUT を設定
}

// POLLOUT 発生時
void client_write(t_env *e, int cs)
{
    int sent = send(cs, e->fds[cs].buf_write, e->fds[cs].buf_write_len, 0);
    if (sent <= 0) {
        close(cs);
        clean_fd(&e->fds[cs]);
        return;
    }
    
    // 送信済み部分を削除
    memmove(e->fds[cs].buf_write, 
            e->fds[cs].buf_write + sent,
            e->fds[cs].buf_write_len - sent + 1);
    e->fds[cs].buf_write_len -= sent;
}
```

**実装演習（Lesson 3.4 の穴埋め問題を実際に埋める）:**

1. `bircd.h` の `t_fd` に `buf_write_len` を追加
2. `client_read` の直 `send` を `queue_message`（`buf_write` 追記）に置き換え
3. `client_write.c` の空関数を POLLOUT 駆動の flush として実装（上の実装例参照）
4. `init_fd.c` の条件付き POLLOUT は既にあるので変更不要（配線が生きる瞬間を確認）
5. 動作確認: `nc localhost 6667` を2枚開き、relay が動くこと。`client_write` に `fprintf(stderr, ...)` を仕込んで「初めて呼ばれた」ことを確認

### チェックリスト

- [ ] 「TCP はバイトストリーム」の意味を説明できる
- [ ] `recv()` が1回で完全なメッセージを返さない理由を説明できる
- [ ] **FD イベントのディスパッチ**と **IRC コマンドのディスパッチ**（B 層 `CommandDispatcher`）の違い（対象・層・キー）を説明できる
- [ ] `client_read` の登録箇所と呼び出し箇所をファイル名・行番号で挙げられる
- [ ] `client_write` が呼ばれない理由を証拠の連鎖（4段）で説明できる
- [ ] `client_read` の即 `send` がアンチパターンである理由を3つ挙げられる
- [ ] `\r\n` でメッセージを切り出す処理を実装できる
- [ ] 部分送信が起こる理由を説明できる
- [ ] POLLOUT を動的に制御する理由を説明できる
- [ ] 演習: `buf_write` 経由の送信経路を実装し、`client_write` が呼ばれることを確認した

### 補助リソース

- 書籍「TCP/IPソケットプログラミング」6章（TCP ソケットの舞台裏）
- `bircd_analysis.md` の「部分データ処理の欠如」セクション

---

## Lesson 4: ノンブロッキングI/O（1-2時間）

### 目標
- ノンブロッキングI/O の必要性を説明できる
- `fcntl()` でノンブロッキング設定できる
- `EAGAIN` を正しくハンドリングできる

### Lesson 4.1 ブロッキング vs ノンブロッキング（30分）

| 状況 | ブロッキング | ノンブロッキング |
|------|-------------|-----------------|
| recv() データなし | 待機（ブロック） | -1, errno=EAGAIN |
| send() バッファ満杯 | 待機（ブロック） | -1, errno=EAGAIN |
| accept() 接続なし | 待機（ブロック） | -1, errno=EAGAIN |

**なぜノンブロッキングが必要か:**

poll() は「読める可能性がある」ことを教えるだけ。
実際に recv() したら 0バイトだった、という状況がありうる。
ブロッキングだとそこで止まってしまう。

### Lesson 4.2 fcntl() でノンブロッキング設定（30分）
p83,85,86
```c
#include <fcntl.h>

// ソケット作成後すぐに設定
int fd = socket(AF_INET, SOCK_STREAM, 0);
fcntl(fd, F_SETFL, O_NONBLOCK);

// accept() で得た fd にも設定
int client_fd = accept(server_fd, ...);
fcntl(client_fd, F_SETFL, O_NONBLOCK);
```

### Lesson 4.3 EAGAIN ハンドリング（30分）

```c
void client_read(t_env *e, int cs)
{
    char tmp[BUF_SIZE];
    int r = recv(cs, tmp, BUF_SIZE, 0);
    
    if (r < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // データがないだけ、エラーではない
            return;
        }
        // 本当のエラー
        close(cs);
        clean_fd(&e->fds[cs]);
        return;
    }
    if (r == 0) {
        // 正常切断
        close(cs);
        clean_fd(&e->fds[cs]);
        return;
    }
    // r > 0: データ受信成功
    // ...
}
```

### チェックリスト

- [ ] ノンブロッキングI/O が必要な理由を説明できる
- [ ] `fcntl(fd, F_SETFL, O_NONBLOCK)` を書ける
- [ ] `EAGAIN` と本当のエラーを区別できる

### 補助リソース

- `man 2 fcntl`
- `man 2 recv` の EAGAIN の説明

---

## Lesson 5: C++98化（2時間）

### 目標
- Lesson 1-4 の成果物を C++98 で書き直せる

### Lesson 5.1 データ構造の変換（1時間）

| C (bircd) | C++98 |
|-----------|-------|
| `t_fd fds[MAX]` | `std::map<int, Client>` または `std::vector<pollfd>` |
| `char buf_read[SIZE]` | `std::string recv_buffer` |
| 関数ポインタ | 仮想関数 or 単純な if 分岐 |
| `malloc/free` | `new/delete` or コンテナ |

### Lesson 5.2 クラス設計例（1時間）

```cpp
class Server {
private:
    int _serverFd;
    int _port;
    std::vector<struct pollfd> _pollfds;
    std::map<int, std::string> _recvBuffers;
    std::map<int, std::string> _sendBuffers;

public:
    Server(int port);
    ~Server();
    
    void run();  // メインループ
    
private:
    void setupSocket();
    void acceptNewClient();
    void receiveData(int clientFd);
    void sendData(int clientFd);
    void disconnectClient(int clientFd);
    void queueResponse(int clientFd, const std::string& msg);
};
```

### Lesson 5.3 関数ポインタ → 仮想関数の対応（30分）

> **発見の文脈（2026-06-12）:** Lesson 3.1 の答え合わせ中、「`client_read` はどこから呼ばれるか？」を追ったところ、`bircd.h` の `fct_read` / `fct_write` スロット（[Lesson 3.2](#lesson-32-fd-イベントのディスパッチ骨格の読解30分)）が **C で手作りした抽象メソッド + 動的ディスパッチ**だと気づいた。その対応表をここに収録する。

bircd の関数ポインタディスパッチを C++ で書き直すとこうなる:

```cpp
class FdHandler {
public:
    virtual void onRead(Env& e, int fd) = 0;   // = fct_read
    virtual void onWrite(Env& e, int fd) = 0;  // = fct_write
};

class ServerSocket : public FdHandler {
    void onRead(Env& e, int fd) { /* srv_accept 相当 */ }
};
class ClientSocket : public FdHandler {
    void onRead(Env& e, int fd) { /* client_read 相当 */ }
};
```

対応関係:

| bircd (C) | C++ |
|---|---|
| `t_fd` 構造体 | オブジェクト |
| `fct_read` / `fct_write` スロット | vtable のエントリ |
| `srv_accept.c:19` での代入 | コンストラクタで型が決まる |
| `e->fds[fd].fct_read(e, fd)` | `handler->onRead(e, fd)` 仮想関数呼び出し |
| `type` フィールド（FD_SERV/FD_CLIENT） | クラスそのもの（型で区別） |

C++ の仮想関数も内部実装は同じ。オブジェクトが vtable（関数ポインタの表）を持ち、呼び出し側は表を引くだけ。bircd はその表を構造体に直接埋めている。

注意点:

- 厳密には bircd のは「メソッド」ではない。C に this は無いから `t_env *` と `int fd` を毎回手渡ししている。C++ ならこれが暗黙の this になる
- **ircserv A 層では仮想関数を採用しない**。fd 種類が2つ（listen / client）しかないので if 分岐で十分、という設計判断（[Lesson 3.2](#lesson-32-fd-イベントのディスパッチ骨格の読解30分) 参照）

### チェックリスト

- [ ] `std::vector<struct pollfd>` で fd 管理できる
- [ ] `std::map<int, std::string>` でバッファ管理できる
- [ ] C++98 の範囲で実装できる（C++11機能は使わない）
- [ ] bircd の関数ポインタスロットと C++ vtable の対応を説明できる

---

## 最終チェックリスト

全 Lesson 完了後、以下を全て説明・実装できることを確認:

### 知識

- [ ] TCP がバイトストリームである理由
- [ ] select() と poll() の違い
- [ ] ノンブロッキングI/O の必要性
- [ ] POLLOUT を動的に制御する理由
- [ ] 受信バッファリングの必要性
- [ ] 送信バッファリングの必要性

### 実装

- [ ] socket/bind/listen/accept でサーバー起動
- [ ] poll() でI/O多重化
- [ ] `\r\n` でメッセージ切り出し
- [ ] 非同期送信（POLLOUT 制御）
- [ ] ノンブロッキング + EAGAIN ハンドリング
- [ ] クライアント切断検知（POLLERR, POLLHUP, recv=0）

---

## 補助リソースまとめ

| Lesson | 必須リソース |
|-------|-------------|
| 1 | 書籍2章, man socket/bind/listen/accept/select |
| 2 | man poll |
| 3 | 書籍6章, bircd_analysis.md |
| 4 | man fcntl, man recv |
| 5 | C++98 リファレンス |

**書籍**: 「TCP/IPソケットプログラミング C言語編」
- URL: https://www.ohmsha.co.jp/book/9784274065194/
- 特に 2章、5章、6章

---

## 関連ドキュメント

- `bircd_analysis.md` - bircd の構造分析
- `reading_guide_A.md` - A担当（Network/IO）向け読書ガイド
- `dev_docs/design.md` §10.2 - A 層実装要件（poll / buffer / POLLOUT）
