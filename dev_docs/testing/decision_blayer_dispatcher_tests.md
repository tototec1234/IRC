# B層 Dispatcher 境界テスト方針

## 背景

B層は `Parser`、`Message`、`CommandDispatcher`、`ReplyBuilder`、`CommandResult` からなる。
このうち C層との境界で最もバグが起きやすい箇所は、`CommandDispatcher` が IRC コマンドの意味を判定し、C層公開 API を使って状態を変更し、`CommandResult` を返す部分である。

C層は現ブランチの `include/` と `src/c/` を正とし、B層は C層の内部実装ではなく公開 API だけを使う。

## 決定事項

`tests/blayer` に B層用のテスト runner を置く。
テストフレームワークは導入せず、C層テストと同じく C++98 の範囲で簡易 test runner を実装する。

Dispatcher 境界テストでは、C層の実装を stub に置き換えず、実C層を使う。
これは、C層 API は component test 済みであるという前提に立ち、B層がその公開 API を正しく使えるかを検証するためである。

## テスト対象

主対象は B層から見た C側境界である。

- `CommandDispatcher`
- `CommandResult`
- `ReplyBuilder` が生成する主要 reply
- Dispatcher が利用する範囲の `Message`

特に、以下の責務を重点的に検証する。

- `Message` に対して正しい command handler が選ばれること
- パラメータ不足などの IRC プロトコルエラーが numeric reply になること
- 登録系コマンドが `Client` 状態を公開 API 経由で更新すること
- nick 更新が `ServerState::updateNick()` 経由で行われること
- JOIN / PART / QUIT などが `ServerState` のFacade APIを使うこと
- 状態変更後に正しい `CommandResult` が返ること
- broadcast の送信先 fd が `CommandResult` に詰められること

## 対象外

以下は Dispatcher 境界テストの主対象外とする。

- socket / poll / send / recv
- A層の `Connection` / `Server`
- C層内部の map / set 構造
- C層の所有権実装そのもの
- Parser の網羅的な構文テスト

Parser は別途単体テストで検証する。
Dispatcher 境界テストでは、原則として `Message` を直接作り、Parser のバグを混ぜない。

## テスト runner の構成

```text
tests/blayer/
  Makefile
  test_main.cpp
```

## 初期テストケース

- `Parser` が trailing parameter を1つの parameter として扱う
- `PING` が `PONG` reply を返す
- `PASS` / `NICK` / `USER` により登録が完了し、`001` が返る
- nick 重複時に `433` が返り、対象 client の nick が更新されない

## 今後追加するテストケース

### 登録系

- `PASS` パラメータ不足は `461`
- password 不一致は `464`
- 登録後の `PASS` / `USER` は `462`
- `NICK` 変更時に nick 辞書と `Client` 本体が同期する

### チャンネル系

- 未登録 client の `JOIN` は `451`
- `JOIN` 成功時に `ServerState::addClientToChannel()` 相当の状態変化が起きる
- invite-only channel に未招待 client が入れない
- key / limit による JOIN 失敗が numeric reply になる
- `PART` 成功時に channel member から外れ、必要なら空 channel が削除される

### メッセージ配送

- `PRIVMSG` to nick が対象 fd にだけ配送される
- `PRIVMSG` to channel が送信者以外の member に配送される
- 存在しない nick / channel は numeric reply になる

### operator系

- `KICK` / `INVITE` / `TOPIC` / `MODE` は operator 権限を確認する
- `MODE +i`, `+t`, `+k`, `+l`, `+o` が C層状態に反映される
- `TOPIC #channel` と `MODE #channel` は照会として reply を返す

## 境界違反チェック

B層は以下を行わない。

- `_unsafe_*` を呼ばない
- `ClientRegistry` を直接触らない
- `send()` を呼ばない
- A層の `Server` / `Connection` / `Poller` に依存しない

## 実行コマンド

```sh
make -C tests/blayer run
```

親 Makefile の `make test` から C層テストと B層テストの両方を呼び出す。
