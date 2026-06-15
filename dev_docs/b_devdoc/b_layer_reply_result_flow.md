# B層 Reply / CommandResult フロー

> 本ドキュメントは、B層が返信文字列を組み立ててから A層へ送信指示を渡すまでの流れを説明する。
> 特に `OutgoingMessage` と `CommandResult` の役割を明確にする。

---

## 1. なぜ CommandResult が必要か

B層は IRC プロトコルの意味を扱うが、socket / send buffer / `send()` / `Connection` / `Server` を直接操作しない。

そのため、B層はコマンド処理の結果を `CommandResult` に詰めて A層へ返す。

```text
B層: どのfdへ、どのIRC lineを送るべきかを決める
A層: 対象fdのConnectionを探し、send bufferへ積む
```

`CommandResult` は「このコマンド処理後に A層へ依頼する送信・切断指示」である。

`OutgoingMessage` は「送信先fd + 完成済みIRC line」を表す。

```cpp
struct OutgoingMessage {
  int         fd;
  std::string message;

  OutgoingMessage(int targetFd, const std::string& text);
};

struct CommandResult {
  std::vector<OutgoingMessage> replies;
  bool                         shouldDisconnect;

  CommandResult();
  void addReply(int fd, const std::string& message);  // OutgoingMessage を replies に積む
};
```

重要なのは、`OutgoingMessage.message` は内部コマンド構造ではなく、A層がそのままsend bufferへ積める IRC wire format の文字列である点である。

---

## 2. 入力コマンド処理の流れ

A層は TCP byte stream を扱う。
TCP は message 境界を保持しないため、A層の `Connection` が recv buffer から `\r\n` 単位で complete line を切り出す。

```text
client socket
  -> A / Connection recv buffer
  -> complete line
  -> B / Parser
  -> B / CommandDispatcher
```

B層の `Parser` は complete line を `Message(command, params)` に変換する。

```text
"PRIVMSG #room :hello world\r\n"
  -> command = "PRIVMSG"
  -> params = ["#room", "hello world"]
```

B層の `CommandDispatcher` は `Message` の意味を解釈する。
例えば、未登録状態で `JOIN` しようとしていないか、`NICK` が重複していないか、`PRIVMSG` の配送先が存在するかを判定する。

状態変更が必要な場合、Dispatcher は C層の公開 API だけを使う。

```text
NICK -> ServerState::updateNick()
JOIN -> ServerState::addClientToChannel()
QUIT -> result.shouldDisconnect = true   // 削除は呼ばない。A の _disconnectClient に委ねる
```

### 2.1 実シグネチャ（A層が1行ごとに呼ぶ B層エントリポイント）

```cpp
Message       Parser::parse(const std::string& line);                          // static
CommandResult CommandDispatcher::dispatch(int fd, const Message& msg, ServerState& state);
```

`dispatch` は `Client` ではなく `int fd` と `ServerState&` を受け取り、`fd` から `Client` を内部解決する。  
現状の `dispatch` は内部で `handlePass` / `handleNick` / `handleUser` へ振り分け、登録完了時に `maybeRegister` が `001 RPL_WELCOME` を積む（JOIN / PRIVMSG 等は未実装）。

A層の呼び出し側（抜粋。全体は [a_implementation_plan.md](../a_devdoc/a_implementation_plan.md) の `Server::_handleRead`）:

```cpp
while (conn->hasCompleteLine()) {
    Message       msg    = Parser::parse(conn->popLine());
    CommandResult result = _dispatcher.dispatch(fd, msg, _state);
    applyCommandResult(result);
}
```

---

## 3. 出力 reply / notice 組み立ての流れ

`ReplyBuilder` は IRC wire format の文字列だけを作る。
送信先fdを決めたり、C層状態を変更したりしない。

```cpp
std::string text = ReplyBuilder::nickInUse("taro");
```

`CommandDispatcher` は、状態とコマンドの意味から送信先を決め、`CommandResult` に詰める。

```cpp
CommandResult result;
result.addReply(client.getFd(), ReplyBuilder::nickInUse("taro"));
return result;
```

A層は `CommandResult.replies` を走査し、各 `OutgoingMessage.fd` に対応する `Connection` の send buffer へ `OutgoingMessage.message` を積む。

```cpp
// A / Server::applyCommandResult（抜粋。全体は ../a_devdoc/a_implementation_plan.md）
for (size_t i = 0; i < result.replies.size(); ++i) {
    const OutgoingMessage& out = result.replies[i];
    std::map<int, Connection*>::iterator it = _connections.find(out.fd);
    if (it == _connections.end()) continue;   // 既に切断済みの fd は skip
    it->second->bufferSend(out.message);        // send buffer へ積むだけ（まだ send しない）
    _enablePollout(out.fd);                      // 当該 fd の POLLOUT を立てる
}
```

`bufferSend()` は文字列を `Connection::_sendBuffer` に追加するだけで `send()` は呼ばない。  
実送信は後続の `poll()` で POLLOUT が立った fd について `_handleWrite() -> Connection::writeToSocket()` が行い、送り切ったら `_disablePollout()` で POLLOUT を下ろす（部分送信対応）。

この分離により、A層は IRC コマンドの意味を知らなくてよい。
また、B層は socket や non-blocking I/O の詳細を知らなくてよい。

---

## 4. 代表例

### 4.1 自分だけに返す numeric reply

`PASS` 不一致や `NICK` 重複は、基本的にコマンド送信元へだけ返す。

```text
input:
  fd 10 -> "NICK taro"

state:
  "taro" is already in use

CommandResult:
  replies = [
    { fd: 10, message: ":irc.local 433 * taro :Nickname is already in use\r\n" }
  ]
  shouldDisconnect = false
```

この場合、`OutgoingMessage` は1件だけでよい。

### 4.2 複数人へ送る broadcast

`JOIN`, `PRIVMSG #channel`, `KICK` などは、同じ IRC line を複数fdへ送ることがある。

```text
input:
  fd 10 -> "PRIVMSG #room :hello"

state:
  #room members = fd 10, fd 11, fd 12

CommandResult:
  replies = [
    { fd: 11, message: ":nick!user@host PRIVMSG #room :hello\r\n" },
    { fd: 12, message: ":nick!user@host PRIVMSG #room :hello\r\n" }
  ]
```

B層は `Channel::getMembers()` などから配送先を決める。
A層は各fdの send buffer に積むだけで、channel membership を知らない。

### 4.3 切断要求

`QUIT` では、B層は `shouldDisconnect=true` を立てて A層へ切断を依頼するだけで、`removeClient` などの C 状態 cleanup は呼ばない。Client 削除は A層の `_disconnectClient` が全切断経路（QUIT / `recv==0` / POLLHUP）で一元的に行う（reader「`removeClient` は A の lifecycle」原則）。

```text
input:
  fd 10 -> "QUIT :bye"

B / Dispatcher:
  result.shouldDisconnect = true   // C 状態の削除は呼ばない。A の _disconnectClient に委ねる

A / Server（Phase 6）:
  1. applyCommandResult(result)        // replies を各 fd の send buffer へ積む
  2. result.shouldDisconnect == true なら source fd を _disconnectClient(fd)
     // _disconnectClient = ServerState::removeClient(fd) + delete Connection + _removeFd(fd)
```

`CommandResult.shouldDisconnect` は「B層からA層への切断要求」である。
B層は `close()` を呼ばない。
`QUIT` は通常 reply を持たないため即時切断で問題ない。reply を伴う切断（将来の `ERROR` 等）では、send buffer を flush してから閉じる判断が要る。

### 4.4 返信なしの成功

すべての成功コマンドが reply を返すとは限らない。
`PASS` は登録手続きの一部であり、成功時専用の reply は定義されていない。
そのため `PASS` 成功のみでは認証状態だけを更新し、`NICK` / `USER` も揃って登録完了した時点で `001 RPL_WELCOME` を返す。

```text
input:
  fd 10 -> "PASS secret"

CommandResult:
  replies = []
  shouldDisconnect = false
```

この空の `CommandResult` は「何もしない」ではなく、「状態変更は完了したが、A層へ送信・切断依頼はない」という意味である。

---

## 5. OutgoingMessage は「コマンド」ではない

`OutgoingMessage` は parser に戻すための command object ではない。

`OutgoingMessage.message` は、A層がそのまま send buffer へ積める完成済み IRC line である。

```text
CommandDispatcher
  -> command semantics handled
  -> state update done
  -> reply string built
  -> OutgoingMessage(fd, completedIrcLine)
```

A層は `OutgoingMessage.message` の中身を解釈しない。
つまり、A層にとって `PRIVMSG`, `433`, `JOIN`, `KICK` はすべて同じ「送信する文字列」である。

この設計により、責務は以下のように分かれる。

| 層 / 型 | 責務 |
|---------|------|
| `ReplyBuilder` | IRC wire format の文字列を作る |
| `CommandDispatcher` | 状態変更、送信先決定、`CommandResult` 組み立て |
| `CommandResult` | A層への送信・切断指示を保持 |
| `OutgoingMessage` | 送信先fd + 完成済みIRC line |
| A層 `Server` / `Connection` | send buffer へ積み、non-blocking sendする |

---

## 6. RFC 根拠

IRC message は prefix、command、parameters から構成される。
詳細は [RFC 1459 Section 2.3](https://datatracker.ietf.org/doc/html/rfc1459#section-2.3) を参照。

message BNF、CRLF 終端、trailing parameter の扱いは [RFC 1459 Section 2.3.1](https://datatracker.ietf.org/doc/html/rfc1459#section-2.3.1) に整理されている。

numeric reply は server prefix、3桁numeric、reply target を含む通常の IRC message として送られる。
詳細は [RFC 1459 Section 2.4](https://datatracker.ietf.org/doc/html/rfc1459#section-2.4) を参照。

numeric reply の一覧は [RFC 1459 Section 6](https://datatracker.ietf.org/doc/html/rfc1459#section-6) にある。

client protocol 側の numeric reply 説明は [RFC 2812 Section 2.4](https://datatracker.ietf.org/doc/html/rfc2812#section-2.4) も参照する。

---

## 7. 実装時の判断基準

### ReplyBuilder に入れるもの

- server prefix を含む numeric reply
- client / channel へ通知する IRC line
- `\r\n` を含む完成済み文字列

### CommandDispatcher に入れるもの

- command ごとの parameter validation
- registration / channel / operator 権限の判定
- C層公開 API による状態変更
- 誰に送るかの決定
- `CommandResult` への追加

### A層に入れるもの

- fd から `Connection` を探す
- send buffer へ文字列を積む
- `POLLOUT` を有効化する
- `shouldDisconnect` を見て安全に切断する

この線を越えないことが、B層とA層の結合を小さく保つための基本方針である。
