# A 層向け ConnectionHealthMonitor 統合ガイド

> 対象: `src/a/Server.{hpp,cpp}` を担当する実装者
> 目的: PING/PONG 生存確認インフラを、poll ループ側へ最小変更で組み込めるようにする。
> 注意: この文書は統合手順であり、現在の `Server.cpp` を直接変更するものではない。

## 1. 使うコンポーネント

PING/PONG の状態管理は `ConnectionHealthMonitor` が持つ。

```cpp
#include "b/DisconnectEvent.hpp"
#include "b/DisconnectNotifier.hpp"
#include "lifecycle/ConnectionHealthMonitor.hpp"
```

主な API:

| API | 用途 |
|---|---|
| `updateActivity(fd)` | クライアントから受信できたことを記録する |
| `generatePing(fd)` | 対象 fd 宛ての `PING` を生成し、PONG 待ち状態にする |
| `markPongReceived(fd, token)` | 正しい `PONG` を受け取ったら PONG 待ち状態を解除する |
| `hasTimedOut(fd)` | 対象 fd が PONG timeout したか判定する |
| `collectTimedOutClients()` | timeout 候補 fd 一覧を返す |
| `removeClient(fd)` | 切断済み fd の PING/PONG state を破棄する |

`ConnectionHealthMonitor` は socket 操作をしない。`send()` / `close()` / fd 削除は A 層の責務のままにする。

timeout した fd の channel 通知は `DisconnectNotifier` が作る。実際の切断、fd 削除、`ServerState` cleanup は A 層の `_disconnectClient(fd)` が担当する。

| 型 / API | 用途 |
|---|---|
| `DisconnectEvent(fd, reason)` | 切断元 fd と理由を 1 つのイベントにまとめる |
| `DisconnectNotifier::build(event, state)` | QUIT 通知だけを作り、`CommandResult` を返す |

## 2. Server.hpp に追加するもの

`Server` が lifecycle state を直接実装しないように、メンバーとして 1 個だけ保持する。

```cpp
#include "b/DisconnectNotifier.hpp"
#include "lifecycle/ConnectionHealthMonitor.hpp"

class Server {
 private:
  ConnectionHealthMonitor _healthMonitor;
  DisconnectNotifier _disconnectNotifier;
};
```

timeout 秒数を明示したい場合は、コンストラクタ初期化リストで指定する。

```cpp
Server::Server(int port, const std::string& pw)
    : _listenFd(-1), _state(pw), _healthMonitor(120) {
  // existing setup
}
```

既定値を使うなら `_healthMonitor` の明示初期化は不要。`DisconnectNotifier` は状態を持たないので明示初期化不要。

## 3. 受信経路への組み込み

### 3.1 recv 成功時に activity を更新する

`Connection::readFromSocket()` が成功した直後に呼ぶ。

```cpp
bool Server::_handleRead(int fd) {
  Connection* conn = _connections[fd];
  if (!conn->readFromSocket()) {
    return false;
  }

  _healthMonitor.updateActivity(fd);

  while (conn->hasCompleteLine()) {
    std::string line = conn->popLine();
    Message msg = Parser::parse(line);
    CommandResult result = _dispatcher.dispatch(fd, msg, _state,
                                                _healthMonitor);
    applyCommandResult(result);
  }
  return true;
}
```

重要点:

- `PONG` を lifecycle に反映するため、既存の `dispatch(fd, msg, _state)` ではなく、`dispatch(fd, msg, _state, _healthMonitor)` を使う。
- `PING` 受信時の通常応答はこれまで通り B 層が `PONG` を返す。
- `PONG` 受信時は B 層が `_healthMonitor.markPongReceived()` を呼ぶ。A 層で token を解析しない。

## 4. PING 送信の組み込み

定期的に接続確認したい fd に対して、A 層から `generatePing(fd)` を呼ぶ。

```cpp
CommandResult pingResult = _healthMonitor.generatePing(fd);
applyCommandResult(pingResult);
```

これにより:

1. `:irc.local PING :irc.local-<fd>-<timestamp>\r\n` が `CommandResult` に入る。
2. 対象 fd は PONG 待ち状態になる。
3. 実際の送信は既存の `applyCommandResult()` → `bufferSend()` → `POLLOUT` 経路で行われる。

`generatePing()` は送信を直接行わないため、A 層の既存送信経路を壊さない。

## 5. timeout 切断の組み込み

timeout fd は `ConnectionHealthMonitor` で検出し、切断通知は `DisconnectNotifier` に委譲する。実際の切断と C 層 cleanup は A 層の `_disconnectClient(fd)` に集約する。

```cpp
std::vector<int> timedOut = _healthMonitor.collectTimedOutClients();
for (std::vector<int>::iterator it = timedOut.begin();
     it != timedOut.end(); ++it) {
  int fd = *it;

  DisconnectEvent event(fd, "Ping timeout");
  CommandResult result = _disconnectNotifier.build(event, _state);
  applyCommandResult(result);

  _disconnectClient(fd);
  _healthMonitor.removeClient(fd);
}
```

処理順はこの順番にする。

1. `collectTimedOutClients()` で fd 候補を取得する。
2. `DisconnectEvent(fd, "Ping timeout")` を作る。
3. `DisconnectNotifier::build()` を呼ぶ。
4. 返ってきた `CommandResult` を `applyCommandResult()` に渡す。
5. A 層の `_disconnectClient(fd)` で socket / pollfd / `Connection` / `ServerState` を片付ける。
6. 最後に `_healthMonitor.removeClient(fd)` で lifecycle 側の fd state を破棄する。

理由:

- `DisconnectNotifier::build()` は、切断 client が所属していた channel を見て QUIT 通知先を決める。
- `DisconnectNotifier` は `ServerState::removeClient(fd)` を呼ばない。
- 先に `_disconnectClient(fd)` を呼ぶと、`ServerState` から client が消えて通知先を作れない。
- `_healthMonitor.removeClient(fd)` は通知生成後かつ `_disconnectClient(fd)` と同じ cleanup 経路で必ず呼び、fd 再利用時に古い PONG 待ち状態を残さない。

### 5.1 `_disconnectClient(fd)` が唯一の removeClient 呼び出し元

現在の `_disconnectClient(fd)` は内部で `_state.removeClient(fd)` を呼んでいる。この方針を維持する。

timeout / QUIT / recv==0 / POLLHUP など、切断理由が何であっても、実際の client 削除は A 層の `_disconnectClient(fd)` へ寄せる。`DisconnectNotifier` は通知生成のみ担当する。

将来的に graceful close を入れる場合は、A 層側で以下の 2 段階に分けるとよい。

```cpp
void Server::_closeConnectionOnly(int fd);  // close + _removeFd + delete Connection
void Server::_disconnectClient(int fd);     // _closeConnectionOnly + _state.removeClient
```

ただし現時点では、A 層担当者の実装負荷を増やさないため `_disconnectClient(fd)` に集約してよい。

### 5.2 recv==0 / POLLHUP も DisconnectEvent に寄せる場合

接続断の通知も統一したい場合は、recv==0 や `POLLHUP` でも同じ流れを使える。

```cpp
DisconnectEvent event(fd, "Connection reset");
CommandResult result = _disconnectNotifier.build(event, _state);
applyCommandResult(result);
_disconnectClient(fd);
```

ただし、相手がすでに切断済みの場合、本人には送れない。`DisconnectNotifier` が作る通知は主に channel の他メンバー向けである。

## 6. poll timeout との関係

PING を定期送信するには、`poll()` が永久待ち `-1` のままだと周期処理を実行しにくい。

統合時の選択肢:

```cpp
int ret = poll(&_pollfds[0], _pollfds.size(), 1000);
```

このように 1 秒などの有限 timeout にすると、I/O イベントがなくても定期的に lifecycle check を実行できる。

推奨する流れ:

1. `poll()` から戻る。
2. 通常の `POLLIN` / `POLLOUT` / `POLLERR` 処理を行う。
3. ループ末尾で必要に応じて `generatePing()`、`collectTimedOutClients()`、`DisconnectNotifier::build()` を呼ぶ。

注意:

- `poll()` timeout 値の調整は A 層のイベントループ設計に関わるため、この lifecycle コンポーネント側には入れない。
- timeout 検出後は、`DisconnectNotifier::build()` → `applyCommandResult()` → `_disconnectClient()` → `_healthMonitor.removeClient()` の順にする。
- 通知を完全に flush してから close したい場合は、A 層側で graceful close 用の状態が別途必要。現状の最小統合では通知を send buffer に積んでから close する設計になるため、送信保証は限定的。

## 7. 最小統合チェックリスト

- [x] `Server.hpp` に `ConnectionHealthMonitor _healthMonitor;` を追加する。
- [x] `Server.hpp` に `DisconnectNotifier _disconnectNotifier;` を追加する。
- [ ] `_handleRead()` の `readFromSocket()` 成功後に `_healthMonitor.updateActivity(fd);` を呼ぶ。
- [x] dispatcher 呼び出しを `dispatch(fd, msg, _state, _healthMonitor)` に変更する。
- [ ] PING を出したいタイミングで `_healthMonitor.generatePing(fd)` を呼び、返った `CommandResult` を `applyCommandResult()` に渡す。
- [x] timeout 検出では `_healthMonitor.collectTimedOutClients()` を呼ぶ。
- [x] timeout fd ごとに `DisconnectEvent(fd, "Ping timeout")` を作る。
- [x] `_disconnectNotifier.build(event, _state)` の戻り値を `applyCommandResult()` に渡す。
- [x] 通知生成後に `_disconnectClient(fd)` で socket / pollfd / `Connection` / `ServerState` を片付ける。
- [x] `_disconnectClient(fd)` と同じ cleanup 経路で `_healthMonitor.removeClient(fd)` を呼ぶ。
- [x] この段階では `Server` に PING/PONG の内部状態を追加しない。
- [x] この段階では `ConnectionHealthMonitor` から `send()` / `close()` を呼ばせない。


## 8. 動作確認

B/lifecycle 単体確認:

```sh
make test
```

全体ビルド:

```sh
make
```

Server 統合後の手動確認例:

```sh
./ircserv 6667 pw
```

別ターミナル:

```sh
nc 127.0.0.1 6667
```

手動入力:

```text
PING :hello
```

期待:

```text
:irc.local PONG irc.local :hello
```

サーバから `PING` が送られる統合まで入れた場合、irssi や一般 IRC クライアントは自動で `PONG` を返す。`PONG` が返れば `ConnectionHealthMonitor` の waiting state が解除される。

## 9. QUIT reason 文字列（A 層 `_notifyAndDisconnect` / `applyCommandResult`）

| reason | 経路 | 根拠 |
|--------|------|------|
| `Client Quit` | B層 `shouldDisconnect`（明示 QUIT） | irc.libera.chat 観測: 他クライアントへ `:nick!user@host QUIT :Client Quit`。自前 nc 結合テストでも再現（PR #50） |
| `Connection reset` | recv==0 / 行長超過 / POLLHUP・ERR / send 失敗 | 本 doc §5.2 サンプル。recv/HUP は doc 準拠。POLLOUT/send 失敗は同じ非自発切断ポリシーで統一（2026-07-04） |
| `Ping timeout` | `_healthMonitor.collectTimedOutClients()` | 本 doc §5。libera は `Ping timeout: N seconds` 形式（[irssi_handson_common.md](../onboarding_docs/irssi_handson_common.md) §PING timeout） |

注意:
- reason は `ReplyBuilder::quit` 経由で `:prefix QUIT :reason` になる。評価必須の固定文言ではない。
- libera の Ping timeout は秒数付きだが、ft_irc 現実装は `"Ping timeout"` のみ（B層 `DisconnectNotifier` が A から渡された reason をそのまま使用）。