# 実装ロードマップ（スケルトン）

> **ステータス**: スケルトン（2026-05-29）  
> **セッション**: #0006  
> **SSOT**: 実装状況の詳細は本ファイルで管理（今後更新）。設計は [design.md](./design.md)、API 契約は [interface.md](./interface.md)。

---

## 凡例

| 記号 | 意味 |
|------|------|
| ⬜ | 未着手 |
| 🔄 | 作業中 |
| ✅ | 完了 |
| ⏸ | 後回し（MVP 外） |

---

## Phase 3: 詳細設計

| ID | 項目 | 状態 | 備考 |
|----|------|------|------|
| 3-1〜3-3 | クラス / IF / データフロー | ✅ | 0004 |
| 3-4 | 依存関係・実装順序 | ✅ | 0006。`development_dependency_diagram.md`, `timeline_diagram.md` |
| 3-5 | ドキュメント整備 | ✅ | 0004, 0005 |
| 3-6 | 設計レビュー（ペア） | ⏸ | メンバー3人目未定のため保留 |
| — | 設計決定: 自作 template 不使用 | ✅ | `decision_no_custom_templates.md` |
| — | 設計決定: エラー・所有権 | ✅ | `decision_error_handling.md` |
| — | 設計決定: invite / removeClient | ✅ | `decision_invite_and_removal.md` |

---

## Phase 4: サーバー基盤（A 層）

| クラス / タスク | 状態 | 担当 | 備考 |
|----------------|------|------|------|
| Server | ⬜ | A | `interface.md` §2 準拠。poll 単一ループ、`applyCommandResult` / `sendTo`（`design.md` §10.2） |
| Connection | ⬜ | A | fd 自己保持、recv/send buffer、complete line、POLLOUT 連動（`ref_interface.md` §5.2） |
| Poller | ⏸ | A | optional |
| ConnectionManager | ⏸ | A | optional |

---

## Phase 5: コマンド（B 層 + C1/C2）

| クラス / コマンド | 状態 | 担当 | 備考 |
|------------------|------|------|------|
| Parser / Message | ⬜ | B | スタブ方針: `proposal_b_layer_stubs.md` |
| CommandDispatcher | ⬜ | B | |
| ReplyBuilder | ⬜ | B | |
| Client / ServerState | ⬜ | C1 | |
| Channel / ChannelModes | ⬜ | C2 | |
| PASS / NICK / USER | ⬜ | B | MVP |
| PING / PONG | ⬜ | B | MVP |
| JOIN / PRIVMSG | ⬜ | B | MVP |
| KICK / INVITE / TOPIC / MODE | ⏸ | B | MVP 後 |

---

## 統合マイルストーン

| マイルストーン | 状態 | 目標 |
|---------------|------|------|
| A + B スタブ結合 | ⬜ | echo / PING |
| MVP（登録 + JOIN + PRIVMSG） | ⬜ | `design.md` Section 11 |
| 全コマンド + 評価準備 | ⬜ | Phase 8 |

---

## 変更履歴

| 日付 | 内容 |
|------|------|
| 2026-05-29 | スケルトン作成（#0006） |
| 2026-05-29 | 外部リポジトリ言及削除、SSOT 参照に統一 |
