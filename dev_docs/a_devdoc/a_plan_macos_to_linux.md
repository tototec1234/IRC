# A層 残作業計画 — ブランチ `fix/A-MacOS-to-linux`

> **⚠️ SUPERSEDED（2026-07-05）** — 内容は 2026-06-20 時点の草案で現状と大きく乖離。  
> **最新の自分用メモ:** [a_status_2026-07-05.md](a_status_2026-07-05.md)

> 作成日: 2026-06-20
> 対象: `src/a/Server.{hpp,cpp}` / `src/a/Connection.{hpp,cpp}`
> 前提: PR #42（B層共通化）・PR #43（接続ライフサイクル通知基盤）を `main` 経由で本ブランチに取り込み済み。
> スコープ: A層の残作業全部（横断クリティカルパス残 + graceful close + EAGAIN/errno）。
> 制約: **Linux(Ubuntu) でのビルド/実行検証は約9時間（12単位）後まで実行不可**（マシン都合）。
> 関連: [cross_layer_critical_path.md](./cross_layer_critical_path.md), [a_implementation_plan.md](./a_implementation_plan.md), [a_layer_io_flow.md](./a_layer_io_flow.md), [errno_and_nonblocking_42_policy.md](./errno_and_nonblocking_42_policy.md), [connection_lifecycle_integration.md](./connection_lifecycle_integration.md), [../interface.md](../interface.md)

---

## 1. 現状補正（既存docが古い点）

横断doc / 実装計画docは「Phase7 fcntl 未実施」「EAGAIN処理が宿題」とするが、実コードは進んでいる:

- **fcntl(O_NONBLOCK) は実装済み**。`Server::_setNonBlocking` を listen fd（ctor）と accept した cs の両方に適用済み。→ Phase7 fcntl は残作業ではなく**検証のみ**。
- **errno/EAGAIN は方針docどおり「現状維持が正解」**。poll駆動の「1イベント1 recv」設計では `recv/send` 後に errno を見ない・`n<0` を即切断扱いが 42準拠かつ正しい（[errno_and_nonblocking_42_policy.md](./errno_and_nonblocking_42_policy.md) §4）。EAGAIN分岐は**足さない**。→ コード変更不要。`Connection.cpp` の「Phase7でEAGANを分ける」誤誘導コメントの**削除のみ**が作業。

## 2. 新規発見 — QUIT が現状壊れている（最優先）

B層 `handleQuit` は `shouldDisconnect = true` を立てるだけで、チャンネルメンバーへの QUIT ブロードキャストをしない（`src/b/CommandDispatcher.cpp`）。A層は `shouldDisconnect` を見て `_disconnectClient(fd)` するのみで、**他メンバーへ QUIT 通知が一切出ない**。

PR #43 の `DisconnectNotifier::build()`（QUIT通知を `CommandResult` で生成）を **A が呼んで初めて QUIT が他者へ届く**。したがって「QUIT通知の配線」は堅牢性ではなく**必須の正当性**であり、A層の最優先残作業。

順序は [connection_lifecycle_integration.md](./connection_lifecycle_integration.md) §5 を厳守:

```
build通知（ServerStateから消す前）→ applyCommandResult → _disconnectClient → healthMonitor.removeClient
```

## 3. 残作業の全体像（優先度順）

| # | 作業 | 種別 | 検証環境 |
|---|------|------|---------|
| 1 | QUIT/非自発切断の `DisconnectNotifier` 通知配線 | **必須・正当性** | macOS可 |
| 2 | `_disconnectClient` を唯一の removeClient 経路に統一・run() の `--i` erase 安全性 | 必須 | macOS可 |
| 3 | Linux(Ubuntu) ビルド/実行整合 | **必須（提出）** | **Linux要（9h後）** |
| 4 | errno/EAGAIN: 変更せず誤誘導コメント削除 | 仕上げ | macOS可 |
| 5 | graceful close（flush後close、最小） | 品質 | macOS可 |
| 6 | PONG → `markPongReceived` 配線（4引数 dispatch） | 堅牢性（最小） | macOS可 |
| 7 | Phase8 仕上げ（usleep確認/デバッグ出力/重複include/`MAX_CLIENTS`重複） | 仕上げ | macOS可 |

> サーバ主導 PING + timeout 切断（`generatePing`/`collectTimedOutClients`）は**マンダトリ要件ではない**ため今回は見送り、別PRのトラッキング項目とする。

## 4. 作業分解（1単位 = 45分、計6単位）

### PR1: 切断/QUIT通知の正当性 + Linux整合（必須・低リスク）— 3単位

- **U1**: `_disconnectNotifier` を Server メンバーに追加。明示QUIT（`shouldDisconnect`）経路を「build通知 → applyCommandResult → _disconnectClient」に配線（順序厳守）。
- **U2**: 非自発切断（`recv==0` / `POLLHUP`/`POLLERR`）も同じ通知経路へ合流。`_disconnectClient` 一本化確認 + run() の `--i` erase 安全性レビュー。
- **U3**: Linux ビルド整合のコードレビュー（ヘッダ/型/関数の移植性）+ errno誤誘導コメント削除 + macOS `make` & `make test` & `nc` 手動検証（QUIT通知が他メンバーへ届く / 切断でクラッシュしない）。

### PR2: graceful close + PONG配線 + 仕上げ（品質）— 3単位

- **U4**: PONG配線のみ。`_healthMonitor` メンバー追加 + `_handleRead` で `updateActivity` + `dispatch(fd, msg, _state, _healthMonitor)` 4引数化。サーバ主導 PING/timeout は見送り。ビルド通し + PONG が `markPongReceived` に届く確認。
- **U5**: graceful close（最小: `_disconnectClient` 前に1回 flush 試行、残ってもclose）。QUIT通知が相手へ届くタイミングとの整合確認。
- **U6**: Phase8 仕上げ（`usleep` 無し確認・デバッグ出力 / 重複 `#include` / `strerror(errno)` デバッグ / `MAX_CLIENTS` 重複定義の整理）+ macOS 総合検証。

### 別枠（6単位外・machine-gated、約9h後）

- Linux(Ubuntu) で `make` 全体ビルド・`nc`/`irssi` 結合・連続接続/切断のクラッシュ確認。
- サーバ主導 PING + timeout 切断統合（`generatePing`/`collectTimedOutClients` → DisconnectNotifier 経路）は別PR候補。

> PR1/PR2 は **macOS検証 + コードレビューでマージ可**とし、Linux実行検証はトラッキング項目として残す（最終提出前に必須）。

## 5. PR分割の軸（採用）

**「正当性・提出必須（低リスク）」 vs 「品質・仕上げ」** で分割（上記 PR1 / PR2）。

- PR1 は QUIT通知という**正当性バグ修正** + Linux整合に絞れ、レビューが軽く先にマージでき、B の QUIT 完成のクリティカルパスを解放する。
- PR2 は graceful close / PONG配線 / 仕上げで `run()`/`_handleRead`/`_disconnectClient` を触るため独立レビューが妥当。PR1 の通知経路に後乗せする依存順で逐次実施。

## 6. 確定した設計判断（質問スキップによりデフォルト採用）

| 論点 | 採用 |
|------|------|
| PR分割軸 | 正当性+Linux / 品質+仕上げ（推奨どおり） |
| keepalive スコープ | PONG→`markPongReceived` 配線のみ。サーバ主導 PING/timeout は後続 |
| graceful close 深さ | 最小（切断前に1回 flush 試行、残ってもclose） |
| errno/EAGAIN | 変更なし（方針doc準拠）。誤誘導コメントのみ削除 |

## 変更履歴

| 日付 | 変更者 | 内容 |
|------|--------|------|
| 2026-06-20 | (AI Navigator 草案) | 初版。PR#42/#43 取り込み後の A層残作業計画 |
| 2026-07-05 | torinoue | superseded。後継 [a_status_2026-07-05.md](a_status_2026-07-05.md) を参照 |
