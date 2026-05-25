# RFC 1459 prefix仕様の分析

> 作成日: 2026-05-25
> 目的: `getFullPrefix()` 実装の根拠となるRFC記述の原文・和訳・解釈

---

## 1. 分析対象

RFC 1459 Section 2.3 "Messages" より、prefix に関する記述を抽出・分析する。

**参照URL:** https://www.rfc-editor.org/rfc/rfc1459#section-2.3

---

## 2. RFC 1459 Section 2.3 原文引用

### 2.1 Message format in 'pseudo' BNF

```
<message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
<prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
```

### 2.2 Prefix の役割（原文）

> The prefix is used by servers to indicate the true origin of the message.
> If the prefix is missing from the message, it is assumed to have originated
> from the connection from which it was received.

### 2.3 クライアントからのprefix（原文）

> Clients should not use prefix when sending a message from themselves;
> if they do, the only valid prefix is the registered nickname associated
> with the client.

### 2.4 Note 6（原文）

> Use of the extended prefix (['!' <user> ] ['@' <host> ]) must not be used
> in server to server communications and is only intended for server to client
> messages in order to provide clients with more useful information about who
> a message is from without the need for additional queries.

---

## 3. 和訳

### 3.1 Message format

```
<message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
<prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
```

- メッセージは任意でprefixを持つ（`[':' <prefix> <SPACE> ]`）
- prefixの形式は「サーバー名」または「nick」＋任意で「!user」「@host」

### 3.2 Prefix の役割

> prefixは**サーバーが**メッセージの真の発信元を示すために使用する。
> メッセージにprefixがない場合、そのメッセージは**受信した接続から発信された**と見なす。

### 3.3 クライアントからのprefix

> クライアントは自身からメッセージを送信する際、prefixを使用**すべきではない**。
> 使用する場合、有効なprefixはそのクライアントに関連付けられた登録済みニックネームのみである。

### 3.4 Note 6

> 拡張prefix（`['!' <user> ] ['@' <host> ]`）はサーバー間通信では使用**してはならず**、
> **サーバーからクライアントへのメッセージ専用**である。
> これは、クライアントが追加のクエリなしにメッセージの送信元についてより有用な情報を
> 得られるようにするためである。

---

## 4. 解釈の導出

### 4.1 「prefixはサーバーがメッセージの真の発信元を示すために使用」

| 原文 | 解釈 |
|------|------|
| "The prefix is **used by servers**" | 主語は「サーバー」 |
| "to indicate the true origin" | 目的は「真の発信元を示す」 |

**結論:** prefixの付与はサーバーの責務。

### 4.2 「prefixがない場合、受信した接続から発信されたと見なす」

| 原文 | 解釈 |
|------|------|
| "If the prefix is missing" | prefixがない場合 |
| "assumed to have originated from the connection" | 接続元が発信元と推定 |

**結論:** クライアントはprefixなしで送信可能。サーバーは接続から発信元を特定する。

### 4.3 「クライアントはprefixを使用すべきではない」

| 原文 | 解釈 |
|------|------|
| "Clients **should not** use prefix" | 推奨レベルで「使用すべきでない」 |
| "the only valid prefix is the registered nickname" | 使う場合でもnickのみ有効 |

**結論:** クライアントからの拡張prefix（`!user@host`）は無効。サーバーは無視してよい。

### 4.4 「拡張prefixはサーバー→クライアント専用」

| 原文 | 解釈 |
|------|------|
| "must not be used in server to server communications" | サーバー間では禁止 |
| "only intended for server to client messages" | サーバー→クライアント専用 |
| "without the need for additional queries" | WHOISなしで情報取得可能 |

**結論:** `nick!user@host` 形式はサーバーがクライアントに送信する際に使用する。

---

## 5. 実装への適用

### 5.1 導かれる設計要件

| 要件 | RFC根拠 | 実装 |
|------|---------|------|
| サーバー→クライアントで拡張prefixを付与 | Section 2.3, Note 6 | `Client.getFullPrefix()` |
| クライアント→サーバーのprefixは無視可 | Section 2.3 "should not use" | `Parser`で無視 |
| prefix形式は `nick!user@host` | BNF定義 | `getFullPrefix()` の戻り値形式 |

### 5.2 getFullPrefix() の仕様

```cpp
// Client.hpp
std::string getFullPrefix() const;
// 戻り値: "nick!user@host"
// 例: "foo!bar@127.0.0.1"
```

### 5.3 使用場面

以下のコマンドの返信でprefixが必要:

| コマンド | 例 |
|----------|-----|
| PRIVMSG | `:foo!bar@host PRIVMSG #ch :hello` |
| JOIN | `:foo!bar@host JOIN #ch` |
| PART | `:foo!bar@host PART #ch :bye` |
| KICK | `:foo!bar@host KICK #ch target :reason` |
| NICK | `:oldnick!user@host NICK newnick` |
| QUIT | `:foo!bar@host QUIT :reason` |
| MODE | `:foo!bar@host MODE #ch +o target` |
| TOPIC | `:foo!bar@host TOPIC #ch :new topic` |
| INVITE | `:foo!bar@host INVITE target #ch` |

---

## 6. 結論

RFC 1459 Section 2.3の記述から、以下が導かれる:

1. **prefixはサーバーの責務** - "used by servers"
2. **クライアントはprefixなしで送信** - "should not use prefix"
3. **拡張prefix（`!user@host`）はサーバー→クライアント専用** - Note 6

これらを満たすため、`Client.getFullPrefix()` メソッドを実装し、
サーバーからクライアントへの全ての通知メッセージで使用する。

---

## 参考文献

- [RFC 1459 - Internet Relay Chat Protocol](https://www.rfc-editor.org/rfc/rfc1459)
  - Section 2.3: Messages
- [RFC 2812 - Internet Relay Chat: Client Protocol](https://www.rfc-editor.org/rfc/rfc2812)
  - Section 2.3: Messages（RFC 1459と同様の記述）
