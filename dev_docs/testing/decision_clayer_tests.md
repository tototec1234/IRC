# C層テスト方針

## 背景

C層は `Client`、`Channel`、`ChannelModes`、`ClientRegistry`、`ServerState` からなる。
これらのは単独でも状態を持つが、実際にバグが起きやすい箇所は、`Client` と `Channel` の相互参照、`ServerState` による所有・寿命管理、nick / channel 辞書の整合性である。

そのため、C層のテストはクラス単体だけではなく、`ServerState` を入口にした C層 component test として設計する。

## 決定事項

`tests/clayer` に C層用のテスト runner を置く。
親 Makefile の `make test` から `tests/clayer` の Makefile を呼び出し、C層テストを実行できるようにする。
テストフレームワークは導入せず、C++98 の範囲で簡易 test runner を実装する。

## テスト対象

対象は C層の状態管理である。

- `Client`
- `Channel`
- `ChannelModes`
- `ClientRegistry`
- `ServerState`

特に、以下の責務を重点的に検証する。

- fd から `Client` を引けること
- nick から `Client` を引けること
- nick 更新時に `Client` 本体と nick 辞書が同期すること
- `Client` と `Channel` の所属関係が `ServerState` 経由で同期されること
- 空 channel が削除されること
- invite は `Channel` 側に保持されること
- client 削除時に member / operator / invite 参照が cleanup されること
- `ChannelModes` が `+i`, `+t`, `+k`, `+l` の状態を保持できること

## 対象外

以下は C層テストの対象外とする。

- socket / poll / send / recv
- IRC message parser
- CommandDispatcher
- ReplyBuilder
- numeric reply 文字列の正確性
- B層の権限判定ロジック
- A層の disconnect 実処理

C層テストでは、B層が公開 API を使ってプロトコル上の条件判定を行うことを前提に、C層の状態変更 API が正しく状態を更新するかを検証する。

## Component Test として扱う理由

`Client` と `Channel` は相互に参照を持つ。
個別クラスだけを stub で置き換えると、実際に問題になりやすい以下のバグを検出しづらくなる。

- `Channel` には member がいるが、`Client` の所属 channel に入っていない
- `Client` は channel に所属しているが、`Channel` 側の member から消えている
- `Client` 削除後に `Channel` の invite list に dangling pointer が残る
- 空になった channel が `ServerState` の辞書に残る
- nick 更新後に `Client` 本体と nick map がずれる

そのため、C層内部のクラスは原則として実物を使い、`ServerState` を入口にした component test として検証する。

## テスト runner の構成

```text
tests/clayer/
  Makefile
  test_main.cpp
  // optional
  test_helpers.hpp
```

## テストケース

### Client / Registry

- `addClient` 後に fd から `Client` を取得できる
- `updateNick` 後に nick から `Client` を取得できる
- `updateNick` 後に `Client::getNick()` も更新されている
- 既存 nick への `updateNick` は失敗する
- RFC1459 case mapping に従い、nick の大文字小文字相当が重複扱いになる

### Channel

- `ChannelModes` の初期値が期待通りである
- `setTopic` / `getTopic` が topic を保持する
- `setOperator` が member の operator 状態を更新する
- invite 追加・確認・削除ができる

### ServerState membership

- `addClientToChannel` が `Channel` を作成する
- `addClientToChannel` が `Channel` 側 member と `Client` 側所属 channel を同期する
- 最初の member が operator になる
- `removeClientFromChannel` が `Channel` 側 member と `Client` 側所属 channel の両方を削除する
- 最後の member が抜けた channel は削除される

### ServerState invite cleanup

- `inviteClientToChannel` が `Channel` 側 invite list に client を追加する
- `removeInviteFromChannel` が invite list から client を削除する
- `removeClient` が、client が参加していない channel の invite list からも client を削除する

### ServerState client cleanup

- `removeClient` が所属する全 channel から client を削除する
- `removeClient` が空 channel を削除する
- `removeClient` 後に fd / nick lookup から client が取得できない

## 実行コマンド

```sh
make test
```

個別に実行する場合:

```sh
make -C tests/clayer run
```