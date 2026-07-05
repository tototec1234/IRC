# README 下書き — torinoue 担当分（日本語）参加検討者向け

> **用途:** tvaroux がルート `README.md`（英語）に統合するための素材
> **作成:** 2026-07-05 / torinoue（本文は AI 生成。方針・事実・レビューは人間）
> **状態:** 改訂版（§1–7）。英語版は AI 翻訳 → tvaroux がルート `README.md` に統合
> **参照:** [chapter5_readme_requirements.md](../docs/chapter5_readme_requirements.md)

---

## 1. 最初の行（イタリック体）

*This project has been created as part of the 42 curriculum by torinoue, tvaroux, tyamaoka.*

（統合時はそのまま英語1行目に転記）

---

## 2. Description

### 2.1 プロジェクト概要

ft_irc は、C++98 で実装した IRC サーバーである。課題のマンダトリ要件に従い、`poll()` による I/O 多重化、非ブロッキングソケット、IRC プロトコル（RFC 1459 系）の主要コマンドをサポートする。

### 2.2 アーキテクチャ（3層構成）


| 層                         | 担当       | 責務                                                                          |
| ------------------------- | -------- | --------------------------------------------------------------------------- |
| **A層（Network/IO）**        | torinoue | `Server` / `Connection` — `socket`・`poll()`・`recv`/`send`・送受信バッファ・接続ライフサイクル |
| **B層（Protocol/Command）**  | tvaroux  | `Parser` / `CommandDispatcher` / `ReplyBuilder` — メッセージ解析・コマンド実行・応答生成       |
| **C層（Application State）** | tyamaoka | `ServerState` / `Client` / `Channel` / `ChannelModes` — クライアント・チャンネル状態管理    |


**プロジェクト設計:** torinoue が **PM / 設計リード**（3層分割・層間契約・オンボーディング設計）を担当し、**A層を実装**。層間の契約とクラス関係は §2.5 の設計ドキュメントが SSOT（担当表は [interface.md](interface.md) §チーム構成と一致）。

データの流れ:

```
recv(A) → Parser::parse(B) → CommandDispatcher::dispatch(B)
  → ServerState 参照/更新(C) → CommandResult → applyCommandResult → POLLOUT 送信(A)
```

切断時は `DisconnectNotifier` がチャンネルメンバーへ QUIT 通知を生成し、A層が `build → enqueue → _disconnectClient` の順で処理する（[connection_lifecycle_integration.md](a_devdoc/connection_lifecycle_integration.md) §5 準拠）。

### 2.3 実装済みコマンド（マンダトリ）


| コマンド                  | 状態  | 備考                                       |
| --------------------- | --- | ---------------------------------------- |
| PASS / NICK / USER    | ✅   | 登録フロー（001 welcome）                       |
| JOIN / PART           | ✅   | チャンネル参加・退出                               |
| PRIVMSG / NOTICE      | ✅   | クライアント・チャンネル宛メッセージ                       |
| QUIT                  | ✅   | `shouldDisconnect` + チャンネルへの QUIT 通知     |
| KICK / INVITE / TOPIC | ✅   | B層 `ChannelCommandHandler` 経由            |
| MODE                  | ✅   | `i` `t` `k` `o` `l`（ChannelModes 連携）     |
| PING / PONG           | ✅   | B層応答 + `ConnectionHealthMonitor` 連携（受信側） |




### 2.4 技術的選択（A層）

- **I/O 多重化:** `poll()`（macOS / Linux 両対応）
- **非ブロッキング:** `fcntl(O_NONBLOCK)` を listen fd と client fd に適用
- **部分読み書き:** `Connection` の `_recvBuffer` / `_sendBuffer` で `\r\n` 行単位に再構成
- **errno / EAGAIN:** poll 駆動のため `recv`/`send` 後に errno を参照しない方針（[errno_and_nonblocking_42_policy.md](a_devdoc/errno_and_nonblocking_42_policy.md)）



### 2.5 設計ドキュメント（Design Documentation）

torinoue がプロジェクト初期に策定した **3層アーキテクチャと層間契約** の参照先。実装の「なぜそうなったか」を追うときの入口。


| ドキュメント                                                                   | 内容                                                         |
| ------------------------------------------------------------------------ | ---------------------------------------------------------- |
| [interface.md](interface.md)                                             | **層間契約の憲章（SSOT）** — `CommandResult`・境界ルール・設計決定の理由          |
| [diagrams/class_overview_diagram.md](diagrams/class_overview_diagram.md) | **クラス関係図** — A/B/C のクラスと主要 public API（`interface.md` の図解版） |
| [diagrams/data_flow_diagram.md](diagrams/data_flow_diagram.md)           | **データフロー図** — 受信→解析→状態更新→送信の時系列（mermaid シーケンス）             |


設計の流れ: 層を分けて **B が A/C の内部を触らない**（`CommandResult` 経由で送信のみ依頼）。A は complete line を渡し、切断は `shouldDisconnect` + `DisconnectNotifier` で通知と物理切断を分離する。

#### 設計判断ナレッジ（`dev_docs/knowledge/`）

実装中に確定した方針の要約。詳細は各 MD を参照。


| ナレッジ                                                                                   | 要約                                                                                                                                                   |
| -------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------- |
| [tcp_stream_and_crlf_nc_experiment.md](knowledge/tcp_stream_and_crlf_nc_experiment.md) | **A層の recv バッファ設計の根拠。** TCP はストリームであり 1 回の `recv` ≠ 1 行。IRC 行末は `\r\n`（CRLF）。`nc` と `hexdump` で macOS/Ubuntu 双方で実証し、`Connection::popLine` の必要性を確認した。 |
| [invite_ticket_policy.md](knowledge/invite_ticket_policy.md)                           | **INVITE の二面性を分離。** クライアントへ届く INVITE 通知と、invite-only チャンネル入場の内部招待券は別物。招待券は `Channel` の invite list が保持し、B/C の責務分界を明確化した。                             |
| [facade_delegation_update_nick.md](knowledge/facade_delegation_update_nick.md)         | **C層 Facade パターン。** B層は `ServerState::updateNick()` のみ呼び、`ClientRegistry` を直接触らない。nick 辞書と `Client` 本体の整合性を層内に閉じ込める設計。                               |
| [channel_limit_policy.md](knowledge/channel_limit_policy.md)                           | **MODE +l の責務分担。** limit の妥当性検証は B層、状態保持は `ChannelModes`（C層）。`0` や負の無効値は B で弾き、C は `>= 1` と `-1`（解除）のみ受け付ける。                                         |




### 2.6 未実装・制限事項（README に正直に書く候補）


| 項目                               | 状態                                                                       |
| -------------------------------- | ------------------------------------------------------------------------ |
| サーバ主導 PING（`generatePing()`）     | `run()` 未配線（Issue #48 決定待ち）                                              |
| Linux 実行検証                       | **未実施**（macOS で開発・検証中）                                                   |
| graceful close（送信 flush 後 close） | 最小実装は defer（[issue_graceful_close.md](a_devdoc/issue_graceful_close.md)） |


---



## 3. Instructions



### 3.1 動作確認済み環境


| 環境               | 状態                                                  |
| ---------------- | --------------------------------------------------- |
| macOS（開発機）       | ✅ `make release` / `make test` / nc 手動結合テスト済み       |
| Linux（校舎 Ubuntu） | ⚠️ **未検証** — 提出前に `make release && make test` を実施予定 |




### 3.2 Compilation

```bash
# サーバーからのログ出力なし（提出・評価想定）
make release

# サーバーからのログ出力あり（開発・デバッグ用）
make debug

# 単体テスト（C層 + B層）
make test
```



### 3.3 Execution

```bash
./ircserv <port> <password>
```

例:

```bash
./ircserv 6667 mypassword
```



### 3.4 クライアント接続

**irssi（評価本番想定）:**

```bash
irssi -c 127.0.0.1 -p 6667 -w mypassword
```

サーバー内コマンド例: `/nick alice`, `/join #general`, `/quit`

**nc（プロトコル手打ち・部分データテスト）:**

```bash
nc -C 127.0.0.1 6667
```

登録例:

```
PASS mypassword
NICK alice
USER alice 0 * :Alice
JOIN #general
PRIVMSG #general :Hello!
QUIT
```

部分送信テスト（課題書 IV.3 推奨）:

```bash
# ctrl+D でコマンドを分割送信
nc -C 127.0.0.1 6667
# → PASS my^Dpassword 等
```



### 3.5 結合テスト例（A層担当分の smoke）

```bash
# サーバー起動
./ircserv 6667 mypassword &

# PING → PONG（B層経由、prefix 付き）
(printf 'PING :foo\r\n'; sleep 1) | nc -C 127.0.0.1 6667

# 登録フロー
(printf 'PASS mypassword\r\nNICK alice\r\nUSER a a a :Alice\r\n'; sleep 1) | nc -C 127.0.0.1 6667
```

---



## 4. Resources



### 4.1 IRC プロトコル

- [RFC 1459 — Internet Relay Chat Protocol](https://datatracker.ietf.org/doc/html/rfc1459)
- [RFC 2812 — Internet Relay Chat: Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812)



### 4.2 ソケットプログラミング

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)



### 4.3 プロジェクト設計・ナレッジ（リポジトリ内）

§2.5 の設計ドキュメント・`knowledge/` ナレッジに加え、実装フェーズ用の資料:

- [a_implementation_plan.md](a_devdoc/a_implementation_plan.md) — A層 Phase 0–8 計画
- [connection_lifecycle_integration.md](a_devdoc/connection_lifecycle_integration.md) — 切断・QUIT 通知の層間契約
- [errno_and_nonblocking_42_policy.md](a_devdoc/errno_and_nonblocking_42_policy.md) — 非ブロッキング I/O 方針
- **bircd** — 課題添付サンプル（[bircd/README.md](../bircd/README.md)）



### 4.4 AI の使用方法（torinoue 分）

課題書 Chapter III に従い、AI の使用は開示する。以下は **torinoue が担当・関与した範囲** のみ。他メンバー分は各自が追記すること。

#### ドキュメント（Markdown）に関する開示

**torinoue が作成・関与した Markdown（**`dev_docs/`**・**`onboarding_docs/`**・本下書き等）は、いずれも AI による生成が主体である。** 白紙から人間のみが一筆書きした MD は存在しない。

人間（torinoue）の役割は次に限定する:

- 何を書くかの指示・設計判断・事実の投入（実験結果・チーム合意・コードの実態）
- AI 生成文のレビュー・修正・採否・コミット判断
- 評価時に内容を説明できることの確認

対象の例: `interface.md`、`diagrams/*.md`、`a_devdoc/*.md`、`knowledge/*.md`、`onboarding_docs/irssi_handson_common.md`、本ファイル `[README_jp_draft_torinoue_sections.md](README_jp_draft_torinoue_sections.md)`

#### プロジェクト進行・クリティカルパス / ボトルネック分析

**PM としての進捗管理・優先順位付け・層間依存の洗い出しについては、全面的に AI による分析を採用した。** コードベースと `dev_docs/` を AI に読ませ、残作業・待ち関係・ボトルネック層（例: B層コマンド実装）を横断的に整理し、その成果を MD として蓄積した。

人間（torinoue）の役割: 分析の**問いの設定**（「今どこが詰まっているか」「A は B を待っているか」）、実コード・PR 事実の**訂正**、チームへの**共有判断**。


| 成果物                                                                                      | 内容                                                           |
| ---------------------------------------------------------------------------------------- | ------------------------------------------------------------ |
| [a_devdoc/cross_layer_critical_path.md](a_devdoc/cross_layer_critical_path.md)           | **横断クリティカルパス SSOT** — A/B/C 依存マトリクス・コマンド別の待ち関係・ボトルネック（B層）の結論 |
| [diagrams/development_dependency_diagram.md](diagrams/development_dependency_diagram.md) | 担当間・実装依存と開発クリティカルパス短縮の論点                                     |
| [diagrams/timeline_diagram.md](diagrams/timeline_diagram.md)                             | フェーズタイムラインとクリティカルパス（PDM 図）・ボトルネックの可視化                        |
| [a_devdoc/a_plan_macos_to_linux.md](a_devdoc/a_plan_macos_to_linux.md)                   | 横断分析に基づく A層残作業の優先度分解（PR 分割案）                                 |
| [b_devdoc/pr42_impact_review.md](b_devdoc/pr42_impact_review.md)                         | PR マージ判断用のインパクト評価（スコープ波及・A↔B 前提）                             |
| [a_devdoc/a_layer_io_flow.md](a_devdoc/a_layer_io_flow.md)                               | クリティカルパス分析の補助（A層実行時 I/O の関数単位フロー）                            |
| [project_management/phase_plan.md](project_management/phase_plan.md)                     | 全体フェーズ計画（EV/BAC 管理の親ドキュメント）                                  |




#### チーム全体の AI 利用方針（経緯）

本プロジェクトの AI 利用は、次の3段階で段階的に広げた。

1. **初期（設計〜実装）** — Cursor 上の AI を **Navigator** として使用。設計レビュー・進捗整理・ドキュメント草案・実装のヒント出し。本番コードは人間が手書きし、AI 生成コードをそのままコミットしない方針を維持した。
2. **PR レビュー習熟期** — チーム全員が GitHub フロー（ブランチ・PR・人間による peer review）に慣れるまで、**レビューは人間のみ**で実施。設計判断と層間契約の合意を優先した。
3. **Copilot Review 導入（PR レビュー体験後）** — 上記を一通り経験したのち、**GitHub Copilot による PR Review** を補助として取り入れた。人間レビュアーの前処理・見落とし候補の洗い出しに使い、**マージ判断は人間レビューが最終**とする。


| 段階     | ツール                   | 用途             | 最終責任        |
| ------ | --------------------- | -------------- | ----------- |
| 設計・実装  | Cursor AI（Navigator）  | 解説・草案・レビュー     | 人間（実装・コミット） |
| PR     | 人間レビュアー               | 層間契約・設計・動作の確認  | 人間          |
| PR（補助） | GitHub Copilot Review | 静的観点の指摘候補・文言提案 | 人間（採否・マージ）  |




#### プロジェクト設計（PM / 設計リード）


| タスク                             | AI の役割                    | 人間（torinoue）の役割        |
| ------------------------------- | ------------------------- | ---------------------- |
| 3層分割・`interface.md`・クラス/データフロー図 | **MD 本文の生成**（構成・文案）       | 設計決定・事実確認・レビュー修正・チーム合意 |
| クリティカルパス / ボトルネック分析             | **全面的に AI 分析**（上表の成果物 MD） | 問いの設定・事実訂正・優先度の最終判断    |
| オンボーディング（irssi ハンズオン）           | **MD 本文の生成**・整形           | シナリオ意図の指示・実施・内容の検証     |
| PR レビュー（補助）                     | Copilot Review で指摘候補の整理   | 人間レビューで採否・設計整合の最終判断    |


| 2 +-



#### A層（Network/IO）の実装


| タスク                                               | AI の役割              | 人間（torinoue）の役割                  |
| ------------------------------------------------- | ------------------- | -------------------------------- |
| 切断配線（`_notifyAndDisconnect` / `shouldDisconnect`） | 設計レビュー・進捗整理・コード案の提示 | **AI 生成コードは破棄**し、設計を理解したうえで手書き実装 |
| git 再入場・残タスク整理                                    | 診断コマンド案・計画書の読み合わせ   | コマンド実行・コミットは本人が実施                |
| `dev_docs/a_devdoc/` 等の技術 MD                      | **本文の生成**（草案・更新案）   | 実コードとの照合・採否・修正指示                 |




#### README 作成（本ファイル）


| ステップ                 | AI                 | 人間（torinoue）                      |
| -------------------- | ------------------ | --------------------------------- |
| 素材の選定                | —                  | 既存 `dev_docs/`・`docs/` から何を載せるか判断 |
| 日本語下書きの構成・本文         | **生成・整形**（質問応答を含む） | 方針指示・回答・レビュー修正                    |
| 英語版 `README.md` への翻訳 | **翻訳**（予定）         | 内容確認（tvaroux が統合・最終レビュー）          |


> **評価時:** **コード**は人間が理解し説明できるもののみコミット（AI 生成コードの無批判なコミットは行っていない）。**Markdown** は AI 生成が主体だが、人間がレビューし説明できる内容に限定している。Copilot Review の指摘も、採用前に人間が内容を確認している。

---



## 6. 開発プロセス・オンボーディング（Development Process）



### 6.1 チームビルディングの流れ（体験 → 理解 → 参加）

本プロジェクトは、**参加前に IRC を体験する**ことからチーム編成を始めた。

1. **募集・スカウト** — ft_irc 参加検討者向けに「IRC クライアント体験ハンズオン」（約 30 分）を案内
2. **体験** — 公開 IRC サーバー（Libera Chat 等）に **irssi** で接続し、`/join`・会話・オペレーター機能を実際に触る
3. **理解** — ハンズオン後半で「今触った操作が ircserv のどの層か」（A/B/C）を対応づけ、**作るもののイメージ**を共有
4. **アサイン** — 興味・適性に応じて A / B / C 層を分担（設計ドキュメント §2.5 が実装の共通言語になる）
5. **実装** — 各層が `interface.md` の契約に沿って並行開発。成果物 `ircserv` はハンズオンで体験した IRC 機能をサーバー側で再現する

この流れにより、メンバーは **ユーザー視点（irssi）と実装視点（3層）の両方** を持った状態で開発に入れる。

### 6.2 ハンズオン資料

詳細シナリオ・コマンド手順・層対応表は以下を参照:

- [onboarding_docs/irssi_handson_common.md](onboarding_docs/irssi_handson_common.md)

要約: 公開サーバーで irssi を使い IRC を体験 → nc で生プロトコルを覗き見 → ircserv の 3層構造と `/nick` `/join` 等の対応を学ぶ。参加検討者向け（2名以上での実施推奨）。

---



## 7. tvaroux への統合メモ


| 項目              | 内容                                                                                           |
| --------------- | -------------------------------------------------------------------------------------------- |
| 英語化             | 本ファイルをベースに AI 翻訳 → tvaroux がルート `README.md` に統合                                              |
| 要追記（torinoue 外） | B層詳細・C層詳細・tvaroux / tyamaoka の AI Usage                                                      |
| §6 の英語化         | "Development Process" として短く翻訳（[chapter5](../docs/chapter5_readme_requirements.md) 必須4セクション外） |
| Linux 検証後       | §3.1 の Linux 行を ✅ に更新                                                                        |
| SIGINT 実装後      | §2.6 から該当制限を削除 or §3 に終了手順を追記                                                                |
| 1行目 login       | `torinoue, tvaroux, tyamaoka` で確定（要チーム確認）                                                    |


---



## 変更履歴


| 日付         | 内容                                               |
| ---------- | ------------------------------------------------ |
| 2026-07-05 | 初版。セクション1–4 を質問しながら作成（torinoue 担当分）              |
| 2026-07-05 | 改訂。ビルド表記・§2.5 設計・knowledge 要約・§6 チームビルディング・担当表更新 |
| 2026-07-05 | §4.4: クリティカルパス/ボトルネック分析は AI 全面採用 + 成果物 MD リンク一覧  |


