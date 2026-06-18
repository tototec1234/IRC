# B 層 実装計画 — 現状ギャップ分析と機能追加順

> 作成日: 2026-06-14
> 対象: `src/b/` (`Parser` / `Message` / `CommandDispatcher` / `CommandResult` / `ReplyBuilder`) と `include/b/`
> 基準仕様: `dev_docs/interface.md`（契約憲章。コマンド/numeric は `b_implementation_reader.md` §6.4・`design.md` §11・`onboarding_B.md` §4 から導出）
> スコープ: **B 層責務のみ**（A 層=ソケット/poll、C 層=`ServerState`/`Client`/`Channel` は範囲外。境界呼び出しのみ記録）
> 前提: 現状は `tests/blayer/test_main.cpp` の 6 ケースが通ることのみが保証された、C 層挙動確認用スタブ段階

---

## まとめ（caveman）

test_main.cppが保証する範囲 = Parser + PASS/NICK/USER/PING/PONG + welcome(001) + エラー(464,433) だけ。チャンネル系・配送系・切断系は未検証。

B層の現状

- ✅ 完成: `Message` / `CommandResult`（境界型契約）、登録フロー(PASS/NICK/USER)、PING、未知→421
- △ 部分: `Parser`（エラー型/長さ上限なし）、`ReplyBuilder`（numeric 6種+PONG+421のみ）
- ❌ 未実装: JOIN/PART/PRIVMSG/NOTICE/QUIT/KICK/INVITE/TOPIC/MODE、451ガード、`shouldDisconnect`、複数fd broadcast

追加すべき機能（優先度）

- P0(MVP): 451ガード → JOIN → PRIVMSG → QUIT
- P1: PART、ReplyBuilder拡充(403/404/401/471/473/475 + join/privmsg通知)、PASS系テスト追加
- P2(提出必須): KICK → INVITE → TOPIC → MODE
- P3: NOTICE、PONG受信、JOIN後のNAMES(353/366)

重要な発見

- C層Facadeは channel操作に必要な分が既に揃っている → B層の blocker なし。主作業は ReplyBuilder拡充 + Dispatcher handler 追加。
推奨着手順: 451ガード → JOIN/PRIVMSG → QUIT → PART → operator系。

---

## 0. このドキュメントの位置づけ

- `tests/blayer/test_main.cpp` が**何をテストしているか**の整理（§1）
- B 層実装の**現状インベントリ**（§2）
- `interface.md` が B 層に**要求している機能**（§3）
- **ギャップ表**（§4）
- **優先度付きの追加機能リスト**（§5）
- 調査出典: explore subagent 報告（2026-06-14）

---

## 1. `tests/blayer/test_main.cpp` が保証していること

自前の極小テストハーネス（`EXPECT_TRUE/EQ/CONTAINS` + `runTest`）。実 C 層（`ServerState`/`Client`）を使い、**B 層の入出力契約**を検証。6 ケース。

| # | テスト名 | 検証内容 | 確認している B 層責務 |
|---|----------|----------|----------------------|
| 1 | parser basic message | `Parser::parse("PRIVMSG #room :hello world\r\n")` → command / param 数 / 各 param | パース（コマンド抽出、trailing `:` 結合） |
| 2 | ping returns pong | `PING token` → `shouldDisconnect=false`、返信 1 件、宛先 fd、本文に ` PONG ` と `token` | PING 応答生成 |
| 3 | registration flow | `PASS pw`→`NICK taro`→`USER ...` で登録完了。途中返信 0、完了で `001 taro`、C 状態反映 | 登録シーケンス、C 状態更新、welcome(001) |
| 4 | nick before pass | PASS 前 `NICK` → `464`、状態未変更 | 登録ガード（順序強制） |
| 5 | user before pass | PASS 前 `USER` → `464`、状態未変更 | 同上 |
| 6 | nick conflict | `taro` 登録済で別 fd が `TARO` → `433`（大小無視衝突）、nick 未設定維持 | nick 重複検出（大小無視） |

**保証範囲**: Parser + PASS/NICK/USER/PING(PONG)/welcome(001)/エラー数値(464,433) のみ。チャンネル系・配送系・切断系は**未検証**。

---

## 2. B 層 現状実装インベントリ

| コンポーネント | 状態 |
|----------------|------|
| `CommandResult` / `OutgoingMessage`（境界型） | ✅ 実装済（interface §1 準拠） |
| `Message` | ✅ 実装済（interface §3.1 準拠。prefix は保持しない設計） |
| `Parser` | △ 部分（基本パース可。エラー型・長さ上限なし） |
| `ReplyBuilder` | △ 部分（numeric 6 種 + PONG + 421 のみ） |
| `CommandDispatcher` | △ 部分（PASS/NICK/USER/PING + 未知→421） |
| チャンネル/配送/切断系 | ❌ 未実装（全て 421 または空応答） |

### 2.1 Parser (`src/b/Parser.cpp`)

| 能力 | 状況 | 根拠 |
|------|------|------|
| 末尾 CRLF 除去 | ✅ | `9:14:src/b/Parser.cpp` |
| prefix スキップ（保持はしない） | ✅ | `34:49:src/b/Parser.cpp` |
| command 抽出 + 大文字化 | ✅ | `17:24:src/b/Parser.cpp` |
| middle params（空白区切り） | ✅ | `52:59:src/b/Parser.cpp` |
| trailing `:`（以降全体を 1 param） | ✅ | `70:72:src/b/Parser.cpp` |
| 空行/不正行 → 空 `Message()` | ✅ | `85:86:src/b/Parser.cpp` |
| パースエラー型/例外 | ❌ なし | — |
| メッセージ長上限 | ❌ なし | — |

### 2.2 Message (`src/b/Message.cpp`)

- ✅ `getCommand()` / `getParams()` / `getParamCount()` / `getSingleParam(i)` / `hasParam(i)`
- ❌ prefix 保持なし（Parser が捨てる設計）

### 2.3 CommandResult (`src/b/CommandResult.cpp`)

- ✅ `replies` / `shouldDisconnect` / `addReply(fd, message)`
- ❌ `shouldDisconnect = true` を設定する handler が**どこにも無い**（QUIT 未実装のため）

### 2.4 ReplyBuilder (`src/b/ReplyBuilder.cpp`)

**実装済**（共通フォーマット `:irc.local <code> <target> ... :text\r\n`、`41:49:src/b/ReplyBuilder.cpp`）:

| メソッド | コード | 根拠 |
|----------|--------|------|
| `pong(token)` | — | `54:57:src/b/ReplyBuilder.cpp` |
| `welcome(client)` | 001 | `59:62` |
| `needMoreParams()` | 461 | `64:68` |
| `alreadyRegistered()` | 462 | `70:73` |
| `passwordMismatch()` | 464 | `75:77` |
| `nickInUse(nick)` | 433 | `79:82` |
| `unknownCommand()` | 421 | `84:88` |

**TODO コメントのみ（未実装）**: 324, 331, 332, 341, 353, 366, 401, 403, 404, 431, 441, 442, 451, 471, 473, 475, 482（`9:32:src/b/ReplyBuilder.cpp`）。
**非 numeric ブロードキャスト未実装**: `join()`, `privmsg()`, `kick()`, `invite()`, `topic()`, `mode()`（ヘッダ宣言も無し）。

### 2.5 CommandDispatcher (`src/b/CommandDispatcher.cpp`)

| コマンド | 分類 | 詳細 |
|----------|------|------|
| PASS | 完全（登録前限定） | 461/462/464、成功時 reply なし、`setPassOk(true)` |
| NICK | 完全（基本） | PASS 必須(464)、433 重複、`updateNick()` 経由、完了で 001 |
| USER | 完全（基本） | param≥4、PASS 必須、462 再登録拒否、完了で 001 |
| PING | 完全 | 461 不足 / PONG 返却 |
| JOIN/PART/PRIVMSG/NOTICE/QUIT/KICK/INVITE/TOPIC/MODE/PONG | 未対応 | 一律 421 |
| 空 command | no-op | 空 `CommandResult` |

**設計ルール遵守（interface §5）**: B は send/close しない ✅ / nick は `updateNick()` 経由 ✅ / `_unsafe_*` 不使用 ✅ / `shouldDisconnect`(QUIT) ❌ 未使用 / channel API ❌ 未使用。

**現在使用中の C 層 API**: `getClientByFd`, `getPassword`, `updateNick`, `Client::{isPassOk,isRegistered,canRegister,markRegistered,setPassOk,setUsername,setRealname,getFd,getNick,getFullPrefix}`。
**未使用（channel/切断系で必要）**: `addClientToChannel`, `removeClientFromChannel`, `removeClient`, `getClientByNick`, `getChannel`, `getOrCreateChannel`, `inviteClientToChannel`, `Channel::*`, `ChannelModes::*`。

---

## 3. `interface.md` が B 層に要求する機能

> interface 本体は契約憲章でコマンド列挙は薄い。クラス責務(§3) + C 層 API(§4) + ルール(§5) + 参照ドキュメントから整理。

### 3.1 境界（interface 直接記載）

- A→B: complete line（`\r\n` 区切り）→ `Parser::parse()`
- B→A: `CommandResult`（fd + 完成 IRC line、`shouldDisconnect`）
- B↔C: `ServerState` facade 経由
- B 禁止: Network/IO 依存、直接 send、`_unsafe_*`、`ClientRegistry` 直接

### 3.2 要求コマンド（出典別・MVP 優先度）

| コマンド | 出典 | 優先度 |
|----------|------|--------|
| PASS / NICK / USER | interface 暗黙 + onboarding_B §4 | MVP（✅ 実装済） |
| PING | onboarding_B §4, design §11 | MVP（✅ 実装済） |
| JOIN | interface §5.3, design §11 | MVP |
| PRIVMSG | design §11, onboarding_B §4 | MVP |
| PART | onboarding_B §4, interface §5.3 | 高（phase_plan 5-5） |
| QUIT | interface §5.3/§5.4, b_layer_reply_result_flow §4.3 | 高（切断 cleanup） |
| KICK / INVITE / TOPIC / MODE | design §11「MVP 後・提出必須」 | 中（最終提出必須） |
| NOTICE | phase_plan 5-6 | 低（PRIVMSG と同型） |
| PONG（受信） | onboarding_B §4 | 低（no-op 可） |

### 3.3 要求 numeric / 非 numeric

- 登録・汎用: 001, 421, 433, 451, 461, 462, 464
- チャンネルエラー: 401, 403, 404, 441, 442, 471, 473, 475, 482
- チャンネル情報: 324, 331, 332, 341, 353, 366
- 非 numeric: JOIN/PRIVMSG/KICK/INVITE/TOPIC/MODE 通知行（prefix 付き）、PONG

### 3.4 要求される振る舞い

| 振る舞い | 要求 |
|----------|------|
| 未登録 client の channel/msg コマンド | 451 |
| PASS 成功 | reply なし |
| 登録完了 | PASS+NICK+USER 揃いで 001 を 1 回 |
| JOIN 成功 | `addClientToChannel()` + member へ JOIN broadcast（初参加者は C が operator 付与） |
| PART / KICK | `removeClientFromChannel()` |
| QUIT | `shouldDisconnect=true` + QUIT broadcast（Client 削除は A の `_disconnectClient` が `removeClient(fd)` を呼ぶ） |
| TOPIC `#ch` / MODE `#ch`（param なし） | 照会（331/332, 324）。状態変更なし |
| PRIVMSG channel | 送信者以外の member へ broadcast |
| PRIVMSG nick | 対象 fd のみ |
| invite-only / +k / +l JOIN | `isInvited()` or 473 / key 検証 → 475 / limit → 471 |

---

## 4. ギャップ表

### 4.1 コマンド × 実装

| コマンド | 要求 | 実装 | 根拠 |
|----------|------|------|------|
| PASS | 完全 | ✅ | `39:59:src/b/CommandDispatcher.cpp` |
| NICK | 完全 | ✅（形式検証なし） | `62:84:src/b/CommandDispatcher.cpp` |
| USER | 完全 | ✅ | `86:108:src/b/CommandDispatcher.cpp` |
| PING | 完全 | ✅ | `21:26:src/b/CommandDispatcher.cpp` |
| 未知 cmd | 421 | ✅ | `33:34:src/b/CommandDispatcher.cpp` |
| PONG | no-op 可 | ❌ 421 | 同上 |
| JOIN | MVP | ❌ 421 | 同上 |
| PRIVMSG | MVP | ❌ 421 | 同上 |
| QUIT | 必須 | ❌ 421 + shouldDisconnect 未 | 同上 |
| PART | 必須 | ❌ 421 | 同上 |
| NOTICE | 推奨 | ❌ 421 | 同上 |
| KICK/INVITE/TOPIC/MODE | 提出必須 | ❌ 421 | 同上 |

### 4.2 numeric × 実装

| 実装済 | 未実装 |
|--------|--------|
| 001, 421, 433, 461, 462, 464 | 324, 331, 332, 341, 353, 366, 401, 403, 404, 431, 441, 442, 451, 471, 473, 475, 482 |

### 4.3 コンポーネント × 実装

| 要求 | 実装 |
|------|------|
| CommandResult 契約 / Message API | ✅ |
| Parser 基本パース | ✅ |
| Parser エラー報告・長さ上限 | ❌ |
| ReplyBuilder 全 numeric / broadcast | ❌ 部分 |
| Dispatcher 全コマンド | ❌ 部分 |
| shouldDisconnect / 未登録ガード(451) / 複数 fd broadcast | ❌ |

### 4.4 テストギャップ（`decision_blayer_dispatcher_tests.md` 対比）

| 計画テスト | 現状 |
|-----------|------|
| Parser trailing / PING→PONG / 登録→001 / nick 重複 433 | ✅ |
| PASS 461 / 464 / 462 | ❌ 未追加 |
| JOIN/PART/QUIT/PRIVMSG/operator | ❌ 未追加 |

---

## 5. 追加すべき機能 — 優先度付き

### P0 — MVP 到達（design §11）

| # | 項目 | B 層作業 | 依存 C 層 API |
|---|------|----------|--------------|
| 1 | 未登録ガード(451) | `dispatch` 入口 or 各 handler 先頭で `isRegistered()` 判定 | `Client::isRegistered()` 既存 |
| 2 | JOIN | +i/+k/+l 検証 → `addClientToChannel` → JOIN broadcast + 331/332 + 353/366 | `addClientToChannel`, `Channel::{getMembers,isInvited,getModes,getTopic}` 既存 |
| 3 | PRIVMSG | nick/ch 配送、404/401/403 | `getClientByNick`, `getChannel`, `Channel::{getMembers,hasMember}` 既存 |
| 4 | QUIT | `shouldDisconnect=true`, member へ QUIT 通知（Client 削除は A に委譲） | `Client::getChannels`, `Channel::getMembers` 既存 |

### P1 — MVP 直後

| # | 項目 | B 層作業 |
|---|------|----------|
| 5 | PART | `removeClientFromChannel` + PART broadcast |
| 6 | ReplyBuilder 拡充 | 451, 403, 404, 401, 471/473/475 + join/privmsg 非 numeric |
| 7 | PASS/NICK/USER テスト追加 | 461/462/464 ケース |

### P2 — 提出必須（design §11 後半）

| # | 項目 | B 層作業 |
|---|------|----------|
| 8 | KICK | operator 検証 → `removeClientFromChannel` + KICK broadcast |
| 9 | INVITE | operator → `inviteClientToChannel` + 通知 + 341 |
| 10 | TOPIC | 照会(331/332) / 変更(+t+op) + broadcast |
| 11 | MODE | 照会(324) / 変更(+i/+t/+k/+l/+o) + broadcast |

### P3 — 補完

| # | 項目 | 備考 |
|---|------|------|
| 12 | NOTICE | PRIVMSG handler 共用 |
| 13 | PONG 受信 | no-op |
| 14 | 353/366 on JOIN | irssi 互換 |
| 15 | Parser 強化 | 長さ上限等（任意） |
| 16 | ChannelService 分離 | optional（Dispatcher 肥大化時） |

### C 層側メモ

- 現状 **B 層の blocker なし**。channel 操作に必要な Facade API は C 側実装済（`addClientToChannel` は invite 消費・初回 operator 付与まで完了。`src/c/ServerState.cpp:19-36`, `src/c/Channel.cpp:19-22`）。

---

## 6. 次に手を付ける順（推奨）

1. `ReplyBuilder::notRegistered()` + dispatch 共通ガード(451)
2. `ReplyBuilder::join/privmsg` + `handleJoin` / `handlePrivmsg`
3. `handleQuit` + `shouldDisconnect`
4. `handlePart`
5. operator 系（KICK → INVITE → TOPIC → MODE）
6. `tests/blayer` を `decision_blayer_dispatcher_tests.md` に沿い拡張

> 主作業は **ReplyBuilder 拡充 + CommandDispatcher handler 追加**。C 層 Facade は概ね揃っている。
