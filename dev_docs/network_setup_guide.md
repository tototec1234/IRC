# ircserv ネットワーク接続ガイド

> 対象: アサイン後の開発・テスト用
> 環境: 42校舎（Ubuntu on iMac）

---

## 1. 1台のPC内で接続（基本）

同一PC内でターミナルを2つ開いて接続する。

### ターミナル1: サーバー起動

```bash
./ircserv 4242 password
```

### ターミナル2: クライアント接続

```bash
irssi -c localhost -n test_nick
```

または:
```bash
irssi -c 127.0.0.1 -n test_nick
```

### PASSコマンド送信（irssi起動後）

```
/quote PASS password
```

---

## 2. 3台のPC間で接続

校舎内の3台のPCを使って接続する。複数人でチャットするIRCの体験ができる。

### 構成

```
                    PC-A（サーバー）
                   ┌─────────────┐
                   │  ircserv    │
                   │  port 4242  │
                   └──────┬──────┘
                          │
            ┌─────────────┼─────────────┐
            │             │             │
            ▼             │             ▼
┌─────────────┐           │   ┌─────────────┐
│ PC-B        │           │   │ PC-C        │
│ irssi       │           │   │ irssi       │
│ nick: taro  │           │   │ nick: hanako│
└─────────────┘           │   └─────────────┘
                          │
                     TCP/IP接続
```

### PC-A（サーバー側）の手順

#### 1. IPアドレスを確認

```bash
# Ubuntu
ip addr show | grep "inet "
# または
hostname -I
```

出力例:
```
inet 10.11.12.34/24 ...
```

`10.11.12.34` がPC-AのIPアドレス。

#### 2. サーバー起動

```bash
./ircserv 4242 password
```

#### 3. PC-B, PC-CにIPアドレスを伝える

「`10.11.12.34` に接続して」と伝える。

---

### PC-B, PC-C（クライアント側）の手順

#### 1. 接続確認（オプション）

サーバーに到達できるか事前確認:

```bash
nc -zv 10.11.12.34 4242
```

成功時:
```
Connection to 10.11.12.34 4242 port [tcp/*] succeeded!
```

失敗時:
```
nc: connect to 10.11.12.34 port 4242 (tcp) failed: Connection refused
```

#### 2. irssiで接続

PC-B:
```bash
irssi -c 10.11.12.34 -n taro
```

PC-C:
```bash
irssi -c 10.11.12.34 -n hanako
```

**注意:** ニックネームは被らないようにする

#### 3. PASSコマンド送信（irssi起動後）

```
/quote PASS password
```

#### 4. チャンネルに参加して会話

PC-B, PC-C両方で:
```
/join #test
```

PC-Bで発言:
```
hello from taro!
```

→ PC-Cにメッセージが届く

---

## 3. トラブルシューティング

### 接続できない場合

| 症状 | 原因 | 対処 |
|------|------|------|
| `Connection refused` | サーバーが起動していない | PC-Aでircservを起動 |
| `Connection timed out` | ファイアウォールでブロック | 別ポート番号を試す |
| `No route to host` | ネットワーク設定の問題 | 同一ネットワークか確認 |

### 別ポートを試す

```bash
# PC-A
./ircserv 8080 password

# PC-B
nc -zv 10.11.12.34 8080
irssi -c 10.11.12.34 -p 8080 -n test_nick
```

### ファイアウォール確認（Ubuntu）

```bash
# 現在の状態確認（SUDO不要）
sudo ufw status
# ※ SUDO権限がない場合は実行不可
```

SUDO権限がない場合、ファイアウォール設定は変更できない。別ポートを試すか、1台のPC内での接続に切り替える。

---

## 4. 42コードレビュー時の想定

レビュー時は通常1台のPC内でテストする:

```bash
# ターミナル1: サーバー
./ircserv 4242 password

# ターミナル2: クライアント1
irssi -c localhost -n reviewer

# ターミナル3: クライアント2
irssi -c localhost -n reviewee

# ターミナル4: ncで生プロトコル確認
nc localhost 4242
```

### ncでの基本動作確認

```bash
nc localhost 4242
PASS password
NICK test_nc
USER test 0 * :Test User
JOIN #test
PRIVMSG #test :Hello from nc!
QUIT :bye
```

---

## 5. コマンドまとめ

| 操作 | コマンド |
|------|----------|
| IPアドレス確認（Ubuntu） | `hostname -I` または `ip addr show` |
| サーバー起動 | `./ircserv 4242 password` |
| 接続確認 | `nc -zv <IP> 4242` |
| irssi接続（localhost） | `irssi -c localhost -n <nick>` |
| irssi接続（リモート） | `irssi -c <IP> -n <nick>` |
| irssi接続（ポート指定） | `irssi -c <IP> -p <port> -n <nick>` |
| PASS送信（irssi内） | `/quote PASS password` |
