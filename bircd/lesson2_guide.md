# Lesson 2: select → poll 変換ハンズオン

> **作成者**: torinoue  
> **前提:** [Lesson 1](bircd_learning_curriculum.md) 必修。`select()` / `fd_set` が読めること。  
> **ブランチ:** `learn/bircd-curriculum` / **タグ:** `lesson-1`（開始）→ `lesson-2`（完成）  
> **Git 初見:** タグの説明は [README.md — Git タグ入門](README.md#git-タグ入門メタ学習)  
> **用語:** 本ガイドの **Lesson** = bircd 学習段階。ircserv の **Phase** とは別（[対応表](bircd_lesson_ircserv_phase_map.md)）。  
> **答え合わせ:** [bircd_learning_curriculum_ans.md](bircd_learning_curriculum_ans.md) Lesson 2 セクション（**本ガイドに完成コードは載せない**）

---

## 目標

- `select()` と `poll()` の違いを説明できる
- bircd の I/O 多重化を `poll()` に書き換えられる
- `struct pollfd` の `fd` / `events` / `revents` を使える

---

## 0. 開始地点の確認

Lesson 1 完了時点（select 版）= タグ **`lesson-1`**。

### 0.1 模範解答を見ないで自力実装する（推奨）

```bash
git fetch origin --tags
git checkout -b my-lesson2-work lesson-1
```

`detached HEAD` を避けるため、**必ず `-b` で作業ブランチを切る**（理由は [README](README.md#detached-head-とは注意)）。

### 0.2 select 版のコードを確認するだけ

```bash
git show lesson-1:bircd/init_fd.c | head -20
git diff lesson-1 lesson-2 -- bircd/init_fd.c   # Lesson 2 で何が変わるか予習（ネタバレ注意）
```

### 0.3 タグのメタ確認（1分）

```bash
git tag
git show lesson-1 --oneline --no-patch
git show lesson-2 --oneline --no-patch
```

`lesson-1` と `lesson-2` が **別のコミット** を指していることを確認する。

---

## 1. poll フラグの実験（15分）

### 1.1 ビルド

`poll_check.c` は bircd 本体とは別の小さなプログラムです。

```bash
cd bircd
gcc -Wall -Wextra -Werror -o poll_check poll_check.c
```

### 1.2 実行

```bash
./poll_check
```

### 1.3 確認すること

出力例（環境で 16 進値は同じはず）:

```
POLLIN  : 0000 0001  (0x0001)
POLLOUT : 0000 0100  (0x0004)
POLLERR : 0000 1000  (0x0008)
POLLHUP : 0001 0000  (0x0010)
```

メモ:

| フラグ | `events` に書く？ | 意味 |
|--------|-------------------|------|
| `POLLIN` | ✓ | 読み取り可能（`accept` / `recv`） |
| `POLLOUT` | ✓ | 書き込み可能（`send`） |
| `POLLERR` | ✗（結果のみ） | エラー |
| `POLLHUP` | ✗（結果のみ） | 相手切断 |

**ポイント:** 値は **ビットフラグ**。`events \|= POLLOUT` で「読み + 書き」を同時監視できる。

---

## 2. select → poll 変換表

| select | poll |
|--------|------|
| `fd_set fd_read` | `pollfds[i].events = POLLIN` |
| `FD_SET(fd, &fd_write)` | `events \|= POLLOUT` |
| `select(max+1, &fd_read, &fd_write, ...)` | `poll(pollfds, nfds, -1)` |
| `FD_ISSET(fd, &fd_read)` | `pollfds[i].revents & POLLIN` |

---

## 3. 変更ファイル一覧

| ファイル | やること |
|----------|----------|
| `bircd.h` | `pollfd` 配列・`nfds` 追加、`fd_set` 削除、関数ポインタに型を付ける |
| `init_fd.c` | `FD_ZERO/SET` → `pollfds[]` 構築 |
| `do_select.c` | `select()` → `poll()` |
| `check_fd.c` | `FD_ISSET` → `revents &` |

`main_loop.c` / `srv_create.c` 等は **基本触らない**。

---

## 4. Step-by-step 手順

### Step 1: `bircd.h`

追加・変更の要点（**自分で書く**）:

```c
#include <poll.h>

#define MAX_CLIENTS 42

struct s_env;   // 前方宣言（関数ポインタの型で使う）

// t_fd: 関数ポインタに引数型を付ける（-Wdeprecated-non-prototype 対策）
void (*fct_read)(struct s_env *, int);
void (*fct_write)(struct s_env *, int);

// t_env:
struct pollfd pollfds[MAX_CLIENTS + 1];  // listen 1 + client 最大 42
int         nfds;
// fd_set fd_read / fd_write は削除
```

**確認問題:**

- なぜ `MAX_CLIENTS + 1` か？
- `nfds` と `MAX_CLIENTS + 1` の違いは？

---

### Step 2: `init_fd.c`

select 版の対応関係:

```
FD_SET(i, &fd_read)              →  events = POLLIN        （使用中 fd は全員）
FD_SET(i, &fd_write) [buf非空]   →  events |= POLLOUT     （送信待ちだけ）
```

穴埋めスケルトン:

```c
void init_fd(t_env *e)
{
    int i;

    e->nfds = 0;
    i = 0;
    while (i < e->maxfd)
    {
        if (e->fds[i].type != FD_FREE)
        {
            e->pollfds[e->nfds].fd = /* ??? */;
            e->pollfds[e->nfds].events = /* ??? */;
            e->pollfds[e->nfds].revents = 0;

            if (strlen(e->fds[i].buf_write) > 0)
                e->pollfds[e->nfds].events |= /* ??? */;

            e->nfds++;
        }
        i++;
    }
}
```

**設計メモ:** bircd は `fds[i]` の **i = fd 番号**。poll 配列は **使用中 fd だけ詰める**。`pollfds[j].fd` に fd 番号を入れる。

---

### Step 3: `do_select.c`

```c
void do_select(t_env *e)
{
    e->r = poll(/* 第1引数 */, /* 第2引数 */, /* 第3引数 */);
}
```

| 引数 | 値 |
|------|-----|
| 第1 | `e->pollfds` |
| 第2 | `e->nfds` |
| 第3 | `-1`（select 版と同じ無限待ち） |

関数名 `do_select` はそのままでもよい（後で `do_poll` にリネーム可）。

---

### Step 4: `check_fd.c`

select 版は `0 .. maxfd-1` を走査し `FD_ISSET`。poll 版は **`0 .. nfds-1`** を走査。

```c
void check_fd(t_env *e)
{
    int i;
    int fd;

    i = 0;
    while (i < e->nfds)
    {
        fd = e->pollfds[i].fd;

        if (e->pollfds[i].revents & POLLIN)
            e->fds[fd].fct_read(/* ??? */, /* ??? */);

        if (e->pollfds[i].revents & POLLOUT)
            e->fds[fd].fct_write(/* ??? */, /* ??? */);

        i++;
    }
}
```

**削除するもの:** select 版の `e->r--` ループ（poll では不要）。

`POLLERR` / `POLLHUP` は Lesson 3 以降で追加可。

---

### Step 5: ビルド

bircd は **C プロジェクト**。`Makefile` は `gcc` を使うこと。

```bash
cd bircd
make re
```

`check_fd.c: passing arguments to a function without a prototype` が出たら → Step 1 の関数ポインタ型を見直す。

---

### Step 6: 動作確認
minitalk とは違うが、複数端末で「送った文字がどこに届くか」を確かめる。

- **minitalk:** client が送った文字列の**終点は server**（server の画面に表示。一方向・シグナル）
- **bircd:** client が送ったデータは server が**中継**し、**送信元以外の全 client** の画面に届く（TCP・複数接続）。server 自身はメッセージ本文を表示しない

**ターミナル 1:**

```bash
./bircd 6667
```

**ターミナル 2・3:**

```bash
nc localhost 6667
```

- 接続メッセージが出る
- 1 クライアントの入力が他クライアントに届く（bircd のブロードキャスト）

---

## 5. 自己チェック

口に出して答えられるか:

1. `nfds` は何を表す？
2. `pollfds[i].fd` と `fds[]` の添字の関係は？
3. なぜ毎ループ `init_fd()` で配列を作り直す？
4. `POLLOUT` を常時監視しない理由は？
5. `select` の `max + 1` に相当するものは poll にあるか？（→ **ない**。`nfds` が監視個数）

---

## 6. 答え合わせ

自分の実装と模範解答（タグ **`lesson-2`**）を比較:

```bash
# 自分の変更（lesson-1 からの差分）
git diff lesson-1 -- bircd/

# 模範解答との差分
git diff my-lesson2-work lesson-2 -- bircd/

# Lesson 1 → Lesson 2 の模範 diff 全体（参考）
git diff lesson-1 lesson-2 -- bircd/
```

文章での解答・メモ: [bircd_learning_curriculum_ans.md](bircd_learning_curriculum_ans.md)

### Git メタ自己チェック

- [ ] `git diff lesson-1 lesson-2` と `git diff lesson-1..lesson-2` の違いを調べた（通常は同じ範囲）
- [ ] 模範 diff を見たあと `git checkout learn/bircd-curriculum` で戻れる

---

## 7. ft_irc への橋渡し

Lesson 2 が終わったら A 層を読む。

| 本 Lesson（C） | ft_irc（C++98） |
|---------------|-----------------|
| `pollfds[MAX_CLIENTS + 1]` 固定配列 | `std::vector<struct pollfd> _pollfds` |
| `nfds` を毎回 0 から再計算 | `push_back` / `erase` |
| `init_fd()` 一括再構築 | `_addFd()` / `_removeFd()` |
| 関数ポインタ `fct_read` | `Server::run()` 内の分岐 |

参照: [../src/a/Server.cpp](../src/a/Server.cpp)
---

## 8. 次の Lesson

Lesson 3（バッファリング）では:

- `recv()` が `\r\n` 単位で返ってこない問題
- 送信バッファ + `POLLOUT` の動的 ON/OFF

を追加する。Lesson 2 の poll 基盤の上に載せる。

---

## 補助リソース

- `man 2 poll`
- [bircd_learning_curriculum.md](bircd_learning_curriculum.md) Lesson 2 節
- [README.md](README.md) — ブランチ・タグの使い方（Git タグ入門）
