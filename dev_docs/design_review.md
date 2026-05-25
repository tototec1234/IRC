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

## 5. myIRCdとの整合性調査（2026-05-25追記）

### 5.1 調査目的

1. IRC_torinoue内の設計図がmyIRCdの設計を正しく反映しているか確認
2. 改善のヒントをmyIRCdから見つける

### 5.2 設計図とmyIRCdの比較

| 項目 | IRC_torinoue設計図 | myIRCd実装 | myIRCd/design.md |
|------|-------------------|-----------|------------------|
| Connection | あり | なし（Server直接管理） | 必須（分離予定） |
| ServerState詳細 | あり | 空（未実装） | あり |
| Channel詳細 | あり | 空（未実装） | あり |
| **InviteList** | **なし** | なし | **あり** |
| **Client._realname** | **なし** | あり | **あり** |
| PING/PONG | なし | なし | なし |

### 5.3 発見した問題点

#### 問題1: InviteListの欠落

myIRCd/docs/design.md Section 7:
```
Channel
├─ members    (参加中のClient一覧)
├─ operators  (Operator権限を持つClient一覧)
├─ invited    (招待されたClient一覧)  ← これが設計図に無い
└─ modes      (チャンネルのモード状態)
```

**対応:** 設計図のChannelクラスに `_inviteList` を追加必須

#### 問題2: Client._realnameの欠落

myIRCd/docs/design.md Section 5.2:
```
Client が持つもの:
- fd
- nick
- username
- realname  ← これが設計図に無い
- PASS 成功状態
- 登録完了状態
```

myIRCd/includes/Client.hpp:
```cpp
std::string _realname;  // 実装済み
```

**対応:** 設計図のClientクラスに `_realname` を追加必須

### 5.4 myIRCd実装の現状

myIRCdは**モック状態**（design.md Section 10）:

| クラス | 状態 |
|--------|------|
| Server | 実装済み（Connectionの責務を内包） |
| Connection | **未分離**（計画のみ） |
| Parser | 実装済み |
| Message | 実装済み |
| Client | 実装済み |
| ChannelModes | 実装済み |
| Channel | **空** |
| ServerState | **空** |
| CommandDispatcher | **空** |
| ReplyBuilder | **空** |
| CommandResult | 実装済み |

**結論:** IRC_torinoue設計図はdesign.mdの計画を反映している。myIRCd実装はまだ計画を完全には実装していない。

### 5.5 myIRCdから得た改善ヒント

#### ヒント1: Messageの便利メソッド

myIRCd/includes/Message.hpp:
```cpp
size_t getParamCount() const;
const std::string& getSingleParam(size_t index) const;
bool hasParam(size_t index) const;
```

設計図には `command()`, `params()` のみ。上記メソッドを追加すると便利。

#### ヒント2: t_reply構造体

myIRCd/includes/CommandResult.hpp:
```cpp
struct t_reply {
    int fd;
    std::string reply;
};
```

設計図の `OutgoingMessage` 相当。明示的な構造体定義があると分かりやすい。

#### ヒント3: ChannelModesの詳細

myIRCd/includes/ChannelModes.hpp:
```cpp
bool _inviteOnly;
bool _topicRestricted;
bool _memberLimited;      // 設計図の _limit に対応
bool _channelProtected;   // MODE +k の有無
int _maxMember;           // 設計図の _limit に対応
std::string _channelPass; // 設計図の _key に対応
```

`_channelProtected` フラグで「パスワード設定の有無」を明示的に管理。設計図より明確。

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
  - Client: `_realname`, `realname()` 追加
  - Channel: `_invited`, `addInvite()`, `isInvited()` 追加
  - Message: `getParamCount()`, `hasParam()`, `getSingleParam()` 追加
- `dev_docs/onboarding_B.md`:
  - PING/PONG コマンドを「接続維持系」として追加

---

## 7. 参照ドキュメント

- 評価基準: `docs/eval/chapter1_introduction.md`, `chapter2_general_rules.md`, `chapter4_mandatory_part.md`, `evalsheet_42evalhub.md`
- ハンズオン: `dev_docs/irssi_handson_common.md`
- 設計図: `dev_docs/diagrams/class_overview_diagram.md`, `data_flow_diagram.md`, `dependency_diagram.md`
- myIRCd設計: `myIRCd/docs/design.md`
