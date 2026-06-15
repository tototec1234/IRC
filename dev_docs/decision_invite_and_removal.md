# 設計決定: invite 命名と Client 削除の内部化

> **ステータス**: 決定（2026-05-29）  
> **セッション**: #0006  
> **関連**: [interface.md](./interface.md), [b_implementation_reader.md](./b_implementation_reader.md), [decision_error_handling.md](./decision_error_handling.md)

---

## 1. `removeClientFromAllChannels` — private 化（確定）

### 1.1 調査結果

| 調査対象 | 結果 |
|----------|------|
| `dev_docs/` 全 md | B 層の公開契約（`interface.md`）に `removeClientFromAllChannels` **なし** |
| `b_implementation_reader.md` | 公開 API 表に誤って記載（0004 で「例:」文言に修正済みだが行は残存） |
| A 層が呼ぶ API | **`removeClient(int fd)` のみ**（disconnect lifecycle） |
| B 層の典型シーン | **QUIT** — `CommandResult.shouldDisconnect=true` を立てる。Client 削除は A 層の `_disconnectClient` に委ねる |
| 参考実装 | `references/ft_IRC-InternetRelayChat-`: Quit から `removeClientFromAllChannels` を呼ぶが、本設計では `removeClient` に集約 |
| 過去チャット | [0004 設計整合性チャット](79d56ff9-fd4a-4f05-821a-7458b04dcdac): プライベートメソッドとして記述に留める合意 |

### 1.2 決定

| 項目 | 内容 |
|------|------|
| 公開 API | `ServerState::removeClient(int fd)` のみ |
| `removeClientFromAllChannels` | **private**。実装フェーズで `.hpp` に記載 |
| `removeClientFromAllInvites` | **private/internal cleanup**。`removeClient(fd)` 内部からのみ呼ぶ |
| A 層 | disconnect 時に `removeClient(fd)` を呼ぶ |
| B 層 | QUIT 時も `removeClient(fd)` を呼ばない。`CommandResult.shouldDisconnect=true` で A 層へ切断を依頼する |
| ドキュメント | 公開 API 表から削除。削除ルール節に「内部 private メソッド（例: …）」として言及 |

### 1.3 QUIT / disconnect の削除経路

```text
Client が QUIT 送信
  → B: CommandDispatcher が QUIT 処理
  → B: （任意）QUIT メッセージを CommandResult に追加
  → B: CommandResult.shouldDisconnect=true
  → A: CommandResult 適用後、_disconnectClient(fd)
  → A: state.removeClient(fd)
       └─ ServerState 内部: removeClientFromAllChannels / removeClientFromAllInvites → delete Client → 辞書更新
```

**PART / KICK** では Client 本体は削除しない。Channel から外すだけ（`Channel::removeMember` 等）。

---

## 2. invite 系メソッドの命名（確定: 案 A）

### 2.1 現状 — 3 つの `invite` 関連名

| 名前 | 層 | クラス | やること |
|------|-----|--------|----------|
| `addInvite(Client*)` | C2 | `Channel` | `_invited` 集合に Client* を追加（**状態変更**） |
| `invite(actor, target, channel)` | B | `ReplyBuilder` | INVITE **通知 IRC 文字列**を生成（**文字列生成**） |
| `invite(channel, target)` | B | `ChannelService`（optional） | 権限チェック + `addInvite` 呼び出し等（**業務ロジック**） |

名前が似ているが、**層と責務が異なる**。0004 で `Channel.addInvite` を正とする判断済み（`design_review.md` L242）。

### 2.2 検討した 2 案

#### 案 A: 現名称維持 + 責務注釈（**採用**）

| メリット | デメリット |
|----------|-----------|
| 0004 / design_review との整合 | 初見で名前が紛らわしい |
| `add*` は Channel の状態 API パターン（`addMember`, `addOperator`）と一致 | 3 層を跨ぐと `invite` が並ぶ |
| リネームによる全ドキュメント・スタブの手戻りなし | — |
| optional の `ChannelService` 分離時も自然 | — |

#### 案 B: リネームして衝突回避

例:

| 現名 | 案 B の例 |
|------|-----------|
| `Channel.addInvite` | `Channel.addInvitedClient` |
| `ReplyBuilder.invite` | `ReplyBuilder.buildInviteNotice` |
| `ChannelService.invite` | `ChannelService.performInvite` |

| メリット | デメリット |
|----------|-----------|
| 名前だけで層が分かる | 0004 合意（`addInvite` 正）と矛盾 |
| grep 時に混同しにくい | interface / クラス図 / スタブ全体の改名コスト |
| — | `addMember` / `addOperator` との命名一貫性が崩れる |

### 2.3 決定

**案 A を採用。** リネームは行わない。

実装・レビュー時は次の区別を使う:

```text
addInvite   → C2 状態（「誰を invited に載せたか」）
ReplyBuilder.invite → B 文字列（「クライアントに何を送るか」）
ChannelService.invite → B ロジック（optional、「INVITE コマンドの手順」）
```

MVP では `ChannelService` 未分離。`CommandDispatcher` が `canInvite` 判定 → `channel.addInvite()` → `ReplyBuilder.invite()` の順で呼ぶ。

---

## 3. ドキュメント反映

| ファイル | 変更 |
|----------|------|
| `b_implementation_reader.md` | `removeClientFromAllChannels` を公開 API 表から削除 |
| `interface.md` | 本決定へのリンク、invite 命名注釈、`removeClientFromAllInvites` の private/internal cleanup 化 |
| `class_overview_diagram.md` | `removeClientFromAllInvites` を層間 public API から除外 |
| `decision_error_handling.md` | 所有権・削除経路 |

---

## 変更履歴

| 日付 | 内容 |
|------|------|
| 2026-05-29 | 初版（セッション #0006。B 層調査 + 0004 合意の確定） |
| 2026-06-15 | #27 / #28: Client 削除を A 層 lifecycle に集約し、`removeClientFromAllInvites` を private/internal cleanup として整理 |
