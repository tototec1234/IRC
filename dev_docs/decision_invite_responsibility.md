# 設計決定: C層における INVITE 管理責務

> **ステータス**: 決定  
> **関連**: [decision_invite_and_removal.md](./decision_invite_and_removal.md), [decision_clayer_return_values.md](./decision_clayer_return_values.md)

---

## 1. 背景

C層では `Client` と `Channel` が相互に関係を持つ状態が存在する。特に JOIN / PART / KICK / QUIT / disconnect では、`Client` と `Channel` の参照関係、および寿命管理の整合性が重要になる。

INVITE も `Channel` が `Client*` を保持する状態であり、JOIN 済み member ではないものの、`Client` と `Channel` の間に独立した参照関係を作る。

そのため、INVITE の状態変更をどの層が管理するかを明確にする。

## 2. 決定事項

INVITE に伴う `Client` と `Channel` の紐付け、および解除の責務は `ServerState` に集約する。

B層は直接 `Channel` の invite list を操作しない。B層は `ServerState` の invite 用 API を呼び出す。

`Channel` 側の invite 操作用メソッドは低レベル API として残してよいが、B層から直接呼ばない契約とする。

## 3. 理由

### 1. Client と Channel の独立した参照関係を作るため

INVITE は単なるフラグ操作ではない。

`Channel` の invite list に `Client*` を保持することで、「この Client はこの Channel に入室するための invite exception を持つ」という関係を作る。

これは JOIN 済み member とは別の、`Client` と `Channel` の独立した参照関係である。そのため、`Client` / `Channel` の所有者である `ServerState` が窓口となり、参照関係の生成・解除を管理する。

### 2. 寿命管理と cleanup が必要なため

INVITE は `Client*` を保持するため、対象 client が QUIT / disconnect した場合に invite list から削除しなければならない。

この cleanup は、個別の `Channel` だけでは完結しない。全 channel を管理している `ServerState` が、client 削除時に invite list から対象 client を取り除く責務を持つ。

これにより、削除済み `Client*` が `Channel` 内に残ることを防ぐ。

### 3. B層の境界を明確にするため

B層は IRC コマンドの意味解釈、権限確認、reply / broadcast 生成を担当する。

一方で、`Client` と `Channel` の参照関係や寿命管理は C層の責務である。

B層が直接 `channel->addInvite(client)` のように操作すると、C層内部の cleanup 契約を B層が知る必要が出る。`ServerState` に Facade を置くことで、B層は「invite を付与する」という意図だけを伝え、C層内部の整合性管理を `ServerState` に任せられる。

## 4. ServerState に寄せない状態

`ServerState` は C層の全状態変更を一手に引き受けるべきではない。

以下のような状態は `Channel` 内部で完結する局所状態であり、`ServerState` が直接管理しない。

- Topic
- Channel modes
- Operator 権限
- 発言権などの member に従属する属性

### 1. Channel 内部の局所状態であるため

Topic や mode は、`Channel` 自身の値である。これらは `Client` と `Channel` の新しい参照関係を作らず、寿命管理にも影響しない。

Operator 権限は `Client` に紐づくが、member 情報に従属する属性である。member から削除されると operator 情報も同時に消えるため、INVITE のような独立した cleanup 対象ではない。

### 2. ServerState の God Object 化を避けるため

すべての channel 状態変更 API を `ServerState` に集約すると、`ServerState` が各 IRC コマンドの詳細ロジックを抱え込む。

`ServerState` は ownership、辞書、参照関係、寿命管理を担うべきであり、`Channel` 内部の局所的な状態変更まで吸い上げるべきではない。

### 3. Channel をドメインオブジェクトとして保つため

`Channel` は単なるデータ構造ではなく、channel 内の状態を管理するオブジェクトである。

Topic 更新、mode 更新、operator 付与などは、データを保持している `Channel` 自身の振る舞いとして実装する。

## 5. invite-only mode との関係

`+i` / `-i` の mode 状態と invite list は別概念として扱う。

- `+i`: JOIN 時に invite list を確認する
- `-i`: JOIN 時に invite list を確認しない
- JOIN 成功時: invite を消費して削除する
- QUIT / disconnect: invite を削除する
- channel 削除: invite list も channel とともに消える

`+i` / `-i` の mode 切替では、invite list は clear しない。未使用の招待券は、JOIN 成功、client の QUIT / disconnect、または channel 削除まで保持する。

libera.chat での確認結果と ircserv の仕様差は [knowledge/invite_ticket_policy.md](./knowledge/invite_ticket_policy.md) に整理する。

## 6. 実装詳細の扱い

この文書は、INVITE の責務境界を決めるための decision である。

具体的な関数名、戻り値方針、テスト範囲は、実装および以下の文書で扱う。

| ドキュメント | 内容 |
|-------------|------|
| [decision_clayer_return_values.md](./decision_clayer_return_values.md) | C層 API の戻り値方針 |
| [decision_invite_and_removal.md](./decision_invite_and_removal.md) | invite 命名と Client 削除 API の整理 |
| [knowledge/invite_ticket_policy.md](./knowledge/invite_ticket_policy.md) | INVITE 通知と招待券の仕様 |
| [testing/decision_clayer_tests.md](./testing/decision_clayer_tests.md) | C層 component test 方針 |

## 変更履歴

| 日付 | 内容 |
|------|------|
| 2026-06-03 | 責務境界に絞る形へ整理 |
