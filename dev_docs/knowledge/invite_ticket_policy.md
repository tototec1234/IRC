# INVITE 通知と招待券の扱い

## 背景

IRC の `INVITE` には、クライアントへ届く通知としての側面と、invite-only channel に入るための内部的な招待券としての側面がある。

この 2 つは混同しやすい。

本プロジェクトでは、サーバ内部の招待券は `Channel` の invite list として扱う。クライアントは INVITE 通知を受け取るだけで、招待券そのものを保持しない。

## libera.chat で確認した挙動

`irc.libera.chat` では、`irssi` および `openssl s_client -connect irc.libera.chat:6697` による確認で、以下の挙動が見られた。

### -i 中の INVITE

1. operator が channel を `-i` にする
2. 未参加 client に `/invite` する
3. target には INVITE 行が届く
4. inviter には `341 RPL_INVITING` が返る
5. target はまだ JOIN しない
6. operator が channel を `+i` にする
7. target が JOIN する

結果: `473 ERR_INVITEONLYCHAN`

解釈: libera.chat では、`-i` 中の INVITE は通知としては成立するが、invite-only 用の内部招待リストには載らないと考えられる。

### +i 中の INVITE と mode 切替

1. operator が channel を `+i` にする
2. 未参加 client に `/invite` する
3. target はまだ JOIN しない
4. operator が channel を `-i` にする
5. operator が channel を再び `+i` にする
6. target が JOIN する

結果: JOIN 成功

解釈: `+i` 中に付与された招待券は、`-i` / `+i` の切替だけでは失効しない。

### JOIN 成功時

1. operator が channel を `+i` にする
2. target を `/invite` する
3. target が JOIN する
4. target が PART する
5. channel が `+i` の状態で target が再度 JOIN する

結果: `473 ERR_INVITEONLYCHAN`

解釈: 招待券は JOIN 成功時に消費される。

## ircserv の仕様

本プロジェクトでは、libera.chat と一部異なる仕様を採用する。

### 1. 発行

`/invite` が成功した場合、channel が `+i` か `-i` かにかかわらず、target client を `Channel` の invite list に追加する。

つまり、`-i` 中に INVITE された client も、後から channel が `+i` になった場合に JOIN できる。

これは libera.chat の再現ではなく、通知と招待券のずれを防ぐための意図的な仕様差である。

### 2. 消費

client が当該 channel に JOIN 成功した時点で、招待券を消費し、invite list から削除する。

これは libera.chat と同じ方針である。

### 3. 保持

`+i` / `-i` の mode 切替では、invite list を clear しない。

未使用の招待券は、JOIN 成功、client の QUIT / disconnect、または channel 削除まで保持される。

これは libera.chat と同じ方針である。

## 責務

- B層は INVITE コマンドの成立条件と通知生成を担当する
- C層は invite list の状態管理を担当する
- `Channel` は invite list を保持する
- `ServerState` は client 削除時に invite list から対象 client を cleanup する
- `Client` は招待券を保持しない

## README に書くべき注意

提出用 README では、以下を明記する候補とする。

```text
INVITE 通知とサーバ内部の招待券は別概念として扱う。
本実装では、INVITE が成功した場合、channel が invite-only かどうかにかかわらず招待券を発行する。
招待券は JOIN 成功時に消費され、mode +i/-i の切替では失効しない。
```
