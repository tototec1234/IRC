# 設計レビュー記録

> レビュー日: 2026-05-25
> 対象: dev_docs/diagrams/ 配下の設計図（class_overview, data_flow, dependency）
> 評価基準: docs/eval/chapter1-4, evalsheet_42evalhub.md

---

## 1. 評価要件との整合性

| 要件 | 設計での対応 | 状態 |
|------|-------------|------|
| poll()が1つだけ | `Server._pollfds` | ✅ OK |
| ノンブロッキングI/O | `Connection._recvBuffer/_sendBuffer` | ✅ OK |
| 部分受信データ処理 | `hasCompleteLine()`, `popLine()` | ✅ OK |
| 複数クライアント同時処理 | `map<int, Connection*>` | ✅ OK |
| 必須コマンド対応 | B層→C1/C2層の構造 | ✅ 対応可 |

**「対応可」の意味:** 設計構造は揃っているが、具体的なコマンド実装ロジックは未定義。

---

## 2. 改善アクション（確定）

### 2.1 InviteList追加（必須）

MODE +i（招待制チャンネル）実装に必要。

```
Channel または ChannelModes に追加:
    set<Client*> _inviteList
    +addInvite(client)
    +removeInvite(client)
    +isInvited(client) bool
```

### 2.2 PING/PONG追加（必須）

クライアント生存確認・接続維持に必要。irssiはPING/PONGを期待する。

```
CommandDispatcher に追加:
    handlePing(fd, msg, state) → PONG返信
    handlePong(fd, msg, state) → 記録のみ（タイムアウト管理用）
```

---

## 3. 設計判断（現状維持）

### 3.1 CommandDispatcher分割

| 判断 | 現状維持 |
|------|---------|
| 理由 | 予想行数400-600行、境界付近 |
| 再評価タイミング | MODEコマンド実装時 |
| 分割候補 | ModeHandlerのみ分離 |

**判断基準:**
- 〜500行: 分割不要
- 500-800行: 分割検討
- 800行〜: 分割必須

### 3.2 ServerState責務分離（ChannelRegistry）

| 判断 | 現状維持 |
|------|---------|
| 理由 | 現設計で責務は既に分離済み |

ServerStateは `_channels` 辞書を持つが、Channelの実装はC2層（hanako担当）に閉じている。

### 3.3 Client/Connection fd二重保持

| 判断 | 現状維持 |
|------|---------|
| 理由 | 層分離の意図あり |

- Connection._fd: ネットワーク層の関心（バッファ、read/write状態）
- Client._fd: アプリケーション層の関心（逆引き用）

統合も選択肢だが、層分離を重視するなら妥当な設計。

---

## 4. 備忘録（実装時決定）

### 4.1 ReplyBuilderのサーバー名設定

**問題:**
```cpp
// 返信例
:myserver.local 001 nick :Welcome to the IRC Network
  ↑ サーバー名が必要
```

staticメソッドのみの現設計では設定値の注入が困難。

**解決案:**

| 案 | 方式 | 評価 |
|----|------|------|
| A | グローバル/シングルトン | ⚠️ 非推奨 |
| B | インスタンス化 | ✅ 推奨 |
| C | 引数で渡す | ✅ 許容 |

実装時に案Bまたは案Cを採用する。

---

---

## 5. 統合前の外部実装との整合性調査（2026-05-25 追記）

> **アーカイブ（2026-05-29）:** 以下は設計統合前の比較記録。**現 SSOT は `IRC_torinoue/dev_docs/` のみ。** Phase 4 以降、外部リポジトリのコードは参照しない。

### 5.1 調査目的（当時）

1. IRC_torinoue 内の設計図が、統合前の外部 design と一致しているか確認
2. 改善ヒントの抽出（結果は §6 に反映済み）

### 5.2 設計図と統合前実装の比較

| 項目 | IRC_torinoue 設計図 | 統合前の実装（2026-05-25） | 統合前 design.md |
|------|---------------------|---------------------------|------------------|
| Connection | あり | なし（Server 直接管理） | 必須（分離予定） |
| ServerState 詳細 | あり | 空（未実装） | あり |
| Channel 詳細 | あり | 空（未実装） | あり |
| InviteList | ✅ あり（`_invited`） | なし | あり |
| Client._realname | ✅ あり | あり | あり |
| Client._host | ✅ あり | - | - |
| Client.getFullPrefix() | ✅ あり | - | - |
| PING/PONG | ✅ あり | なし | なし |

### 5.3 修正済みの問題点（2026-05-25 対応完了）

#### 問題1: InviteList の欠落 → **解決済み**

統合前 design.md Section 7 より:

```
Channel
├─ members
├─ operators
├─ invited    ← _invited として追加済み
└─ modes
```

**対応:** 設計図の Channel クラスに `_invited` を追加 ✅

#### 問題2: Client._realname の欠落 → **解決済み**

**対応:** 設計図の Client クラスに `_realname` を追加 ✅

### 5.4 統合前実装の現状（2026-05-25 時点）

| クラス | 状態 |
|--------|------|
| Server | 実装あり（Connection 責務を内包） |
| Connection | 未分離 |
| Parser / Message / Client / ChannelModes / CommandResult | 一部実装 |
| Channel / ServerState / CommandDispatcher / ReplyBuilder | 未実装 |

**結論:** IRC_torinoue 設計図は `dev_docs/design.md` の計画を反映。以降は dev_docs を SSOT として `IRC_torinoue/src/` に新規実装する。

### 5.5 当時得た改善ヒント（§6 反映済み）

- Message: `getParamCount()`, `hasParam()`, `getSingleParam()` を設計に追加
- CommandResult: `OutgoingMessage` 構造体として明示
- ChannelModes: `hasKey()` / `key()` 等で +k 状態を明示管理する案を検討

---

## 6. 改善アクション（確定）まとめ

| 優先度 | 項目 | 対応 | 状態 |
|--------|------|------|------|
| **高** | InviteList | Channel/ChannelModesに追加 | ✅ 完了（2026-05-25） |
| **高** | PING/PONG | CommandDispatcherに追加 | ✅ 完了（2026-05-25） |
| **高** | Client._realname | Clientクラスに追加 | ✅ 完了（2026-05-25） |
| 中 | Message便利メソッド | getParamCount(), hasParam()等追加 | ✅ 完了（2026-05-25） |
| 低 | t_reply構造体 | OutgoingMessageとして明示化 | 保留（実装時決定） |

### 6.1 完了した変更（2026-05-25）

- `dev_docs/diagrams/class_overview_diagram.md`:
  - Client: `_realname`, `_host`, `getRealname()`, `getHost()`, `getFullPrefix()`, `getFd()` 追加
  - Channel: `_invited`, `addInvite()`, `isInvited()` 追加
  - Message: `getParamCount()`, `hasParam()`, `getSingleParam()` 追加
- `dev_docs/onboarding_B.md`:
  - PING/PONG コマンドを「接続維持系」として追加

---

## 7. 参照ドキュメント

- 評価基準: `docs/eval/chapter1_introduction.md`, `chapter2_general_rules.md`, `chapter4_mandatory_part.md`, `evalsheet_42evalhub.md`
- ハンズオン: `dev_docs/irssi_handson_common.md`
- 設計図: `dev_docs/diagrams/class_overview_diagram.md`, `data_flow_diagram.md`, `dependency_diagram.md`
- ドキュメント体制（2026-06-01 MTG 確定）:
  - 公開 API・クラス関係 SSOT: `dev_docs/diagrams/class_overview_diagram.md`
  - 契約憲章 SSOT: `dev_docs/interface.md`
  - 実装読み物（B層主読者、非 SSOT）: `dev_docs/b_implementation_reader.md`
  - 全体設計: `dev_docs/design.md`
