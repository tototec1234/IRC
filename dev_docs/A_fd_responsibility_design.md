# fd と Connection の責務分離

> 本ドキュメントは、Server と Connection における fd 管理の設計意図を説明する。

---

## 1. 設計の要点

| クラス | 保持するもの | 責務 |
|--------|-------------|------|
| **Server** | `std::map<int, Connection*> _connections` | fd から Connection を**検索**する |
| **Connection** | `int _fd` | 自分がどの fd を担当しているか**自己完結**で知る |

---

## 2. なぜ両方で fd を持つのか

### Server の辞書（検索の責務）

```cpp
// fd をキーに Connection を探す
Connection* conn = _connections[fd];
```

- `poll()` から返された fd に対応する Connection を特定する
- O(log n) で検索可能（`std::map` 使用）

### Connection の `_fd`（自己完結の責務）

```cpp
// Connection は自分の fd を知っている
int Connection::getFd() const {
    return _fd;
}
```

- Server の辞書に依存せず、自分の fd にアクセス可能
- `pollfd` 配列構築時に使用
- ログ出力、デバッグに使用

---

## 3. `getFd()` の具体的な使用場面

### 3.1 pollfd 配列の構築

`poll()` システムコールを呼ぶ前に、監視対象の fd 一覧を構築する。

```cpp
// C++98 準拠
std::vector<struct pollfd> fds;

// _connections を走査
for (std::map<int, Connection*>::iterator it = _connections.begin();
     it != _connections.end(); ++it)
{
    Connection* conn = it->second;
    
    struct pollfd pfd;
    pfd.fd = conn->getFd();  // Connection から fd を取得
    pfd.events = POLLIN;
    
    if (conn->hasPendingOutput()) {
        pfd.events |= POLLOUT;
    }
    pfd.revents = 0;
    
    fds.push_back(pfd);
}

// poll() 呼び出し
int ret = poll(&fds[0], fds.size(), -1);
```

### 3.2 なぜ `it->first` を使わないのか

辞書のキー `it->first` でも fd は取得できるが：

1. **自己完結**: Connection が Server の辞書構造に依存しない
2. **柔軟性**: 辞書以外のコンテキスト（ベクタ等）でも fd にアクセス可能
3. **一貫性**: 他のクラス（Client 等）と同様に getter を提供

---

## 4. 設計の根拠

`design.md` Section 2.2:

> `Connection` と `Client` はfdで紐づくが、責務は明確に分離する。

`ref_interface.md` Section 5.2:

> `Connection` はIRCコマンドの意味を知らない。

Connection は純粋に I/O の責務に集中し、Server の内部構造（辞書の実装）を知らない設計。

---

## 5. C++98 での注意点

| C++11以降 | C++98 での代替 |
|-----------|----------------|
| `auto it = ...` | `std::map<int, Connection*>::iterator it = ...` |
| `for (auto& pair : _connections)` | `for (it = begin(); it != end(); ++it)` |
| `fds.data()` | `&fds[0]` |
| `nullptr` | `NULL` |

---

## 6. 参考資料

### システムコール

| 資料 | 内容 |
|------|------|
| `man poll(2)` | poll システムコールの仕様 |
| `man 7 socket` | ソケットプログラミング概要 |

### 書籍

| タイトル | 該当箇所 |
|----------|----------|
| The Linux Programming Interface | Chapter 63: Alternative I/O Models |
| UNIX Network Programming Vol.1 (Stevens) | Chapter 6: I/O Multiplexing |

### Web

| URL | 内容 |
|-----|------|
| [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) | Section 7.2: poll() |
| [IBM Developer - Using poll()](https://developer.ibm.com/articles/l-async/) | 非同期I/O解説 |

---

## 7. 関連ドキュメント

| ドキュメント | 内容 |
|-------------|------|
| `design.md` | Section 2.2: TCP接続とIRC状態の分離 |
| `ref_interface.md` | Section 5.2: Connection インターフェース |
| `diagrams/class_overview_diagram.md` | クラス図（Server, Connection） |
