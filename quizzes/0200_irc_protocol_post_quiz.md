# IRC プロトコル 事後クイズ

> 対象: フェーズ 2（IRC プロトコル学習）完了後
> 教材: RFC 1459, RFC 2812
> 目的: 学習内容の定着確認、ペア学習での相互確認

---

## 使い方

1. RFC を読んだ**後**に回答
2. 口頭でペアに説明できるレベルを目指す
3. 回答できなければ該当セクションを再読

---

## 基本概念

### Q1: IRC とは何の略か？

**模範回答**: Internet Relay Chat

**あなたの回答**:

### Q2: IRC のアーキテクチャを説明せよ

**模範回答**:
- クライアント: ユーザーが使用するアプリケーション
- サーバー: クライアント間のメッセージを中継
- ネットワーク: 複数のサーバーが相互接続（ft_irc では不要）

**あなたの回答**:

### Q3: IRC メッセージの終端文字は？

**模範回答**: `\r\n` (CRLF, Carriage Return + Line Feed)

**あなたの回答**:

### Q4: IRC メッセージの最大長は？

**模範回答**: 512 バイト（CRLF を含む）

**あなたの回答**:

---

## メッセージ形式

### Q5: IRC メッセージの BNF 形式を説明せよ

**模範回答**:
```
message    = [ ":" prefix SPACE ] command [ params ] crlf
prefix     = servername / ( nickname [ [ "!" user ] "@" host ] )
command    = 1*letter / 3digit
params     = *14( SPACE middle ) [ SPACE ":" trailing ]
           / 14( SPACE middle ) [ SPACE [ ":" ] trailing ]
middle     = nospcrlfcl *( ":" / nospcrlfcl )
trailing   = *( ":" / " " / nospcrlfcl )
```

簡略化:
```
[:prefix] command [param1 param2 ... [:trailing]]
```

**あなたの回答**:

### Q6: prefix の役割は？

**模範回答**: メッセージの送信元を示す。サーバーがクライアントに転送する際に付与。クライアントからサーバーへのメッセージには通常不要。

例: `:alice!alice@host.com PRIVMSG #channel :Hello`

**あなたの回答**:

### Q7: 以下のメッセージを解析せよ

```
:alice!alice@localhost PRIVMSG #general :Hello, everyone!
```

**模範回答**:
- prefix: `alice!alice@localhost`
- command: `PRIVMSG`
- param1: `#general`
- trailing: `Hello, everyone!`

意味: alice が #general チャンネルに "Hello, everyone!" を送信

**あなたの回答**:

---

## 接続・登録シーケンス

### Q8: クライアント登録の手順を述べよ（ft_irc の場合）

**模範回答**:
1. TCP 接続確立
2. `PASS <password>` - サーバーパスワード（ft_irc 必須）
3. `NICK <nickname>` - ニックネーム設定
4. `USER <username> <mode> * :<realname>` - ユーザー情報設定
5. サーバーから welcome メッセージ（001 など）を受信

**あなたの回答**:

### Q9: NICK コマンドの形式と役割は？

**模範回答**:
```
NICK <nickname>
```
ニックネームを設定または変更。他のユーザーから見える表示名。

**あなたの回答**:

### Q10: USER コマンドの形式と各引数の意味は？

**模範回答**:
```
USER <username> <mode> <unused> :<realname>
```
- username: ユーザー名（ident）
- mode: 初期モード（通常 0）
- unused: 未使用（`*` を指定）
- realname: 本名（自由記述）

**あなたの回答**:

---

## チャンネル操作

### Q11: JOIN コマンドの形式は？

**模範回答**:
```
JOIN <channel> [<key>]
```
- channel: チャンネル名（`#` で始まる）
- key: チャンネルパスワード（MODE +k 設定時）

**あなたの回答**:

### Q12: PART コマンドの形式は？

**模範回答**:
```
PART <channel> [:<message>]
```
チャンネルを離脱。オプションで離脱メッセージを指定可能。

**あなたの回答**:

### Q13: KICK コマンドの形式と権限は？

**模範回答**:
```
KICK <channel> <user> [:<reason>]
```
チャンネルオペレーター（@）のみ実行可能。指定ユーザーをチャンネルから強制退出。

**あなたの回答**:

### Q14: INVITE コマンドの形式と用途は？

**模範回答**:
```
INVITE <nickname> <channel>
```
ユーザーをチャンネルに招待。MODE +i（招待専用）のチャンネルに入るために必要。

**あなたの回答**:

### Q15: TOPIC コマンドの形式は？

**模範回答**:
```
TOPIC <channel> [:<topic>]
```
- topic なし: 現在のトピックを表示
- topic あり: トピックを設定（MODE +t の場合オペレーターのみ）

**あなたの回答**:

---

## MODE コマンド

### Q16: ft_irc で実装すべき MODE オプションをすべて挙げよ

**模範回答**:
| オプション | 意味 |
|-----------|------|
| +i / -i | 招待専用チャンネル |
| +t / -t | TOPIC をオペレーターに制限 |
| +k / -k | チャンネルパスワード |
| +o / -o | オペレーター権限付与/剥奪 |
| +l / -l | ユーザー数制限 |

**あなたの回答**:

### Q17: 以下のコマンドの意味を説明せよ

```
MODE #channel +o alice
```

**模範回答**: #channel で alice にオペレーター権限を付与

**あなたの回答**:

### Q18: 以下のコマンドの意味を説明せよ

```
MODE #channel +k secretpass
```

**模範回答**: #channel にパスワード "secretpass" を設定。以降、JOIN 時にパスワードが必要。

**あなたの回答**:

---

## メッセージ送信

### Q19: PRIVMSG の形式は？

**模範回答**:
```
PRIVMSG <target> :<message>
```
- target: ユーザー名 または チャンネル名
- message: 送信するメッセージ

**あなたの回答**:

### Q20: PRIVMSG でチャンネルに送信した場合、誰がメッセージを受け取るか？

**模範回答**: そのチャンネルに参加している**自分以外**の全ユーザー

**あなたの回答**:

---

## 数値リプライ

### Q21: 以下の数値リプライの意味を述べよ

| コード | 意味 |
|--------|------|
| 001 | |
| 331 | |
| 332 | |
| 403 | |
| 433 | |
| 461 | |

**模範回答**:
| コード | 意味 |
|--------|------|
| 001 | RPL_WELCOME - 登録完了 |
| 331 | RPL_NOTOPIC - トピック未設定 |
| 332 | RPL_TOPIC - トピックあり |
| 403 | ERR_NOSUCHCHANNEL - チャンネルが存在しない |
| 433 | ERR_NICKNAMEINUSE - ニックネームが既に使用中 |
| 461 | ERR_NEEDMOREPARAMS - パラメータ不足 |

**あなたの回答**:

---

## 実践問題

### P1: 以下のシーケンスで何が起こるか説明せよ

```
Client -> Server: PASS mypassword
Client -> Server: NICK alice
Client -> Server: USER alice 0 * :Alice Smith
Server -> Client: :server 001 alice :Welcome to the IRC Network alice!alice@localhost
Client -> Server: JOIN #general
Server -> Client: :alice!alice@localhost JOIN #general
Server -> Client: :server 332 alice #general :Welcome to #general!
Client -> Server: PRIVMSG #general :Hello!
Server -> All in #general except alice: :alice!alice@localhost PRIVMSG #general :Hello!
```

**あなたの回答**:

### P2: irssi で以下を実行し、サーバーの応答を記録せよ

1. サーバーに接続
2. ニックネームを設定
3. チャンネルに参加
4. メッセージを送信
5. チャンネルを離脱

**あなたの回答**:

---

## 理解度チェック

| 項目 | 確認 |
|------|------|
| メッセージの形式（prefix, command, params）を説明できる | ☐ |
| 登録シーケンス（PASS, NICK, USER）を説明できる | ☐ |
| チャンネル操作（JOIN, PART, KICK, INVITE, TOPIC）を説明できる | ☐ |
| MODE の各オプション（+i, +t, +k, +o, +l）を説明できる | ☐ |
| 主要な数値リプライの意味を知っている | ☐ |

---

## ペア学習用

上記の質問を使って、torinoue と sohyamaz で相互に質問し合う。
特に MODE コマンドは複雑なので、実際に irssi で試しながら確認することを推奨。
