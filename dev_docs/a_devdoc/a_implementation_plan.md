# A 層 実装計画 — 機能軸フェーズ分け

> 作成日: 2026-06-09
> 対象: `src/a/Server.{cpp,hpp}` / `src/a/Connection.{cpp,hpp}`
> 前提:
> - **B 層 `src/b/` は完成済み**（Parser / Message / CommandDispatcher / CommandResult / ReplyBuilder）
> - **C 層 `src/c/` も完成済み**（Client / ServerState / Channel / ChannelModes / ClientRegistry）
> - A 層は **WIP コミット中**（直近 `5e5c8df` で listen + poll 骨格）
> - **結合テスト合格基準: `nc` で `PING :foo` 送ったら `PONG :foo` が返る**

---

## 1. 現状到達度 (2026-06-09 時点)

| 項目 | 状態 |
|------|------|
| `socket` / `bind` / `listen` | ✅ Server コンストラクタ内で完了 |
| `_addFd(listen_fd, POLLIN)` | ✅ |
| `_pollfds` (`std::vector<struct pollfd>`) | ✅ |
| `run()` の `poll()` ループ骨格 | ✅（ただし `usleep(100ms)` の placeholder 入り） |
| `_acceptClient()` | ⚠️ `accept` + `_addFd(cs, POLLIN)` のみ。**`Connection` 生成も `ServerState::addClient` 呼び出しも未実装** |
| `Connection` メソッド群 (`readFromSocket`/`hasCompleteLine`/`popLine`/`bufferSend`/`writeToSocket`/`hasPendingOutput`) | ❌ 宣言のみ、実装空 |
| `Server` に `_connections` / `_state` / `_dispatcher` メンバー | ❌ 未追加 |
| `_handleRead` / `_handleWrite` / `applyCommandResult` | ❌ 未実装 |
| `_enablePollout` / `_disablePollout` / `_removeFd` | ❌ 未実装 |
| `_disconnectClient` | ❌ 未実装 |
| ノンブロッキング (`fcntl`) | ❌ 未実装 |
| 切断検知 (POLLERR/POLLHUP/recv=0) | ❌ 未実装 |
| 信号処理 (SIGINT/SIGPIPE) | ❌ 未実装 |

---

## 2. 機能軸でのフェーズ分け

> 各フェーズ完了時点で **コミット 1 つ**（WIP 含む）を想定。
> 「PING/PONG 結合テスト合格」マーカーがゴール。

| Phase | 名前 | 内容 | 完了で得られる動作 | 現状 |
|-------|------|------|------------------|------|
| **0** | 起動骨格 | `main.cpp` 引数処理、`Server` インスタンス化、`run()` 呼び出し | `./ircserv 6667 pass` で起動だけはできる | ✅ |
| **1** | Listen ソケット | `socket` / `bind` / `listen` / `_addFd(listen, POLLIN)` | `nc localhost 6667` で **接続だけ**できる（その後沈黙） | ✅ |
| **2** | poll ループ骨格 + Accept | `run()` の `while + poll`、POLLIN 時に `_acceptClient` で `accept` | 複数 `nc` が **同時接続できる**（沈黙）。fd は `_pollfds` に積まれる | ⚠️ WIP（`usleep` 仮置きあり、Connection 生成なし） |
| **3** | Connection クラス + 受信バッファ | `Connection::readFromSocket/hasCompleteLine/popLine` 実装、`Server::_connections` 追加、`_acceptClient` で `new Connection`、`_handleRead` で `popLine` まで | `nc` で入力した文字列が `\r\n` で行単位に切り出せる（まだサーバは何も返さない） | ❌ |
| **4** | B 層連携（recv → Parse → Dispatch） | `Server::_state`/`_dispatcher` 追加、`_acceptClient` で `_state.addClient(cs)`、`_handleRead` で `Parser::parse → _dispatcher.dispatch`、`CommandResult` 取得 | recv した行が B 層を通って `CommandResult` まで生成される（**ただしまだ送り返せない**） | ❌ |
| **5** | 🎯 送信経路 (POLLOUT + send) | `Connection::bufferSend/writeToSocket/hasPendingOutput` 実装、`Server::applyCommandResult/_enablePollout/_disablePollout/_handleWrite` 実装、`run()` で POLLOUT 分岐 | **🎯 PING/PONG 結合テスト合格**。`nc` で `PING :foo` → `PONG :foo` が返る | ❌ |
| 6 | 切断・エラー処理 | POLLERR/POLLHUP 検出、`recv == 0` 切断、`_disconnectClient`（`ServerState::removeClient` + `delete Connection` + `_removeFd`）、`shouldDisconnect`(QUIT) 処理、SIGINT/SIGPIPE | `Ctrl+C` `nc` 切断や `QUIT` 受信でクラッシュせず、`_connections` から正しく除去 | ❌ |
| 7 | ノンブロッキング化 | `fcntl(fd, F_SETFL, O_NONBLOCK)` を listen fd と accept した cs に適用、accept ループ化（多重受付）、`EAGAIN` 正常系扱い | ノンブロッキング要件を満たす（評価で 0 点回避） | ❌ |
| 8 | 仕上げ | `usleep` 削除、`#include` 整理、エラーメッセージ統一、定数の `MAX_CLIENTS` 廃止/再考、ホスト設定 (`inet_ntoa` → `Client::setHost`)、コメント整理 | 提出可能水準 | ❌ |

---

## 3. 「PING/PONG 結合テスト」がどのフェーズで合格するか

**答え: Phase 5 完了時点**

理由:
1. **recv 経路** (Phase 3) … `nc` から `"PING :foo\r\n"` が `Connection::popLine()` で取れる
2. **B 層通過** (Phase 4) … `Parser::parse → CommandDispatcher::dispatch` で `CommandResult` が返る（B 層完成済みなので `PONG :foo\r\n` の `OutgoingMessage` が入っている）
3. **send 経路** (Phase 5) … `applyCommandResult` が `Connection` の send buffer に積み、POLLOUT で `send` する

→ Phase 6（切断処理）は**結合テスト合格には不要**。Phase 7（ノンブロッキング）も `nc` 1 本のテストではブロックしないので暫定動く。**ただし課題要件としては Phase 7 まで必須**（fcntl なし提出は 0 点）。

---

## 4. 結合テスト合格のための **最小** 追加実装

### 4.1 `include/a/Server.hpp` 追加メンバー

```cpp
#include <map>
#include "a/Connection.hpp"
#include "b/CommandDispatcher.hpp"
#include "b/CommandResult.hpp"
#include "c/ServerState.hpp"

class Server {
    // ...
private:
    int                              _listenFd;
    std::vector<struct pollfd>       _pollfds;
    std::map<int, Connection*>       _connections;   // ← Phase 3
    ServerState                      _state;         // ← Phase 4
    CommandDispatcher                _dispatcher;    // ← Phase 4

    void _acceptClient();
    void _handleRead(int fd);                        // ← Phase 4
    void _handleWrite(int fd);                       // ← Phase 5
    void _enablePollout(int fd);                     // ← Phase 5
    void _disablePollout(int fd);                    // ← Phase 5
    void applyCommandResult(const CommandResult&);   // ← Phase 5
    // ...
};
```

コンストラクタ初期化リストに `_state(password)` を追加。

### 4.2 `src/a/Connection.cpp` 実装（Phase 3 + Phase 5）

```cpp
// Phase 3
bool Connection::readFromSocket() {
    char buf[4096];
    while (true) {
        ssize_t n = recv(_fd, buf, sizeof(buf), 0);
        if (n == 0) return false;                                 // 切断
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;   // 正常
            return false;                                          // エラー
        }
        _recvBuffer.append(buf, static_cast<size_t>(n));
    }
    return true;
}
bool Connection::hasCompleteLine() const {
    return _recvBuffer.find("\r\n") != std::string::npos;
}
std::string Connection::popLine() {
    size_t pos = _recvBuffer.find("\r\n");
    std::string line = _recvBuffer.substr(0, pos);
    _recvBuffer.erase(0, pos + 2);
    return line;
}

// Phase 5
void Connection::bufferSend(const std::string& msg) { _sendBuffer += msg; }
bool Connection::hasPendingOutput() const { return !_sendBuffer.empty(); }
bool Connection::writeToSocket() {
    while (!_sendBuffer.empty()) {
        ssize_t n = send(_fd, _sendBuffer.c_str(), _sendBuffer.size(), 0);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            return false;
        }
        _sendBuffer.erase(0, static_cast<size_t>(n));
    }
    return true;
}
```

### 4.3 `src/a/Server.cpp` 追加 (Phase 3–5)

```cpp
// Phase 3: _acceptClient() に Connection 生成を追加
void Server::_acceptClient() {
    // ... 既存 accept ...
    Connection* conn = new Connection(cs);
    _connections[cs] = conn;
    _addFd(cs, POLLIN);
    _state.addClient(cs);   // Phase 4 のため C 層に登録
}

// Phase 4
void Server::_handleRead(int fd) {
    Connection* conn = _connections[fd];
    if (!conn->readFromSocket()) return; // Phase 6 で切断処理
    while (conn->hasCompleteLine()) {
        Message msg = Parser::parse(conn->popLine());
        CommandResult result = _dispatcher.dispatch(fd, msg, _state);
        applyCommandResult(result);
    }
}

// Phase 5
void Server::applyCommandResult(const CommandResult& result) {
    for (size_t i = 0; i < result.replies.size(); ++i) {
        int targetFd = result.replies[i].fd;
        std::map<int, Connection*>::iterator it = _connections.find(targetFd);
        if (it == _connections.end()) continue;
        it->second->bufferSend(result.replies[i].message);
        _enablePollout(targetFd);
    }
}
void Server::_handleWrite(int fd) {
    Connection* conn = _connections[fd];
    conn->writeToSocket();
    if (!conn->hasPendingOutput()) _disablePollout(fd);
}
void Server::_enablePollout(int fd) {
    for (size_t i = 0; i < _pollfds.size(); ++i)
        if (_pollfds[i].fd == fd) { _pollfds[i].events |=  POLLOUT; return; }
}
void Server::_disablePollout(int fd) {
    for (size_t i = 0; i < _pollfds.size(); ++i)
        if (_pollfds[i].fd == fd) { _pollfds[i].events &= ~POLLOUT; return; }
}

// run() の中で fd ごとに振り分け
void Server::run() {
    while (true) {
        int ret = poll(&_pollfds[0], _pollfds.size(), -1);
        if (ret < 0) break;
        // ループ中に _pollfds が変わるためインデックス操作に注意
        for (size_t i = 0; i < _pollfds.size(); ++i) {
            short rev = _pollfds[i].revents;
            int   fd  = _pollfds[i].fd;
            if (!rev) continue;
            if (fd == _listenFd) { if (rev & POLLIN) _acceptClient(); continue; }
            if (rev & POLLIN)  _handleRead(fd);
            if (rev & POLLOUT) _handleWrite(fd);
        }
    }
}
```

---

## 5. 「PING/PONG 結合テスト」確認手順（Phase 5 完了後）

```bash
make re
./ircserv 6667 pass &
SERVER=$!
(printf 'PING :foo\r\n'; sleep 0.5) | nc 127.0.0.1 6667
# 期待: PONG :foo
kill $SERVER
```

これが通ったら Phase 5 合格。次は Phase 6（切断・エラー処理）に進む。

---

## 6. 残りのリスク・宿題（Phase 6–8）

| 項目 | フェーズ | 重要度 |
|------|---------|--------|
| ノンブロッキング `fcntl(O_NONBLOCK)` | 7 | **必須（評価 0 点回避）** |
| accept ループ化（多重 SYN 同時受付） | 7 | 推奨 |
| POLLERR/POLLHUP 検出と `_disconnectClient` | 6 | 必須 |
| `recv == 0` での切断検知 | 6 | 必須 |
| `QUIT` 後の `shouldDisconnect` ハンドリング | 6 | 必須 |
| SIGPIPE 無視 (`signal(SIGPIPE, SIG_IGN)`) | 6 | 必須（クラッシュ防止） |
| ループ中の `_pollfds` インデックス安全化 | 6 | 中（**走査中の `erase` 問題**） |
| `usleep(100ms)` の削除 | 8 | 必須（無意味かつパフォーマンス劣化） |
| `inet_ntoa` で `Client::setHost` | 8 | 推奨（B 層 ReplyBuilder の `getFullPrefix` 用） |
| `MAX_CLIENTS` 廃止 → `vector` 動的に | 8 | 推奨 |

---

## 7. コミット粒度の提案

| コミット | フェーズ | メッセージ案 |
|---------|---------|-------------|
| 次の 1 つ | 2 仕上げ | `feat(A): poll ループから usleep を削除、_acceptClient で fd 登録の純化` |
| その次 | 3 | `feat(A): Connection に recv バッファと popLine を実装。Server に _connections マップ` |
| その次 | 4 | `feat(A): B 層連携。_handleRead で Parser→Dispatcher→CommandResult` |
| その次 | 5 🎯 | `feat(A): 送信経路完成。applyCommandResult と POLLOUT 制御。PING/PONG 結合テスト通過` |
| その次 | 6 | `feat(A): 切断とエラー処理。_disconnectClient と SIGPIPE` |
| その次 | 7 | `feat(A): 全 fd を fcntl(O_NONBLOCK) に。accept ループ化` |
| その次 | 8 | `chore(A): 仕上げ。include 整理、メッセージ統一` |

---

## 8. まとめ

- **現在は Phase 2 の途中**（poll ループ骨格はできたが、`Connection` が空、Server に `_connections`/`_state`/`_dispatcher` 未追加）
- **PING/PONG 結合テストが通るのは Phase 5 完了時点**
- 必要な最小差分は **Connection 実装 + Server に 3 メンバー追加 + 5 メソッド追加 + run() の振り分け修正**
- Phase 6（切断）と Phase 7（fcntl）は結合テスト後でよいが、**提出までには必須**
