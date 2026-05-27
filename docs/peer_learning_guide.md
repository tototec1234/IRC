# ft_irc ペアラーニングガイド

> 原文参照: the_art_of_peer_evaluation.en.pdf + ft_irc.pdf Chapter III (AI Instructions)
> 対象: torinoue, sohyamaz

---

## 1. 42のピアラーニング哲学

### コア原則

42は**コミュニティ**である。あなたは全体の一部であり、以下に貢献することが自然：

*原文: 42 is a community. You're part of a whole.*

- 学校の良い雰囲気
- 全員の進歩
- 利用可能なリソース
- 学生の評価

### 評価の目的

評価（ディフェンス）は単なる採点ではなく、**学習の最終ステップ**：

*原文: Once work is done, one could rush towards the next project, this final and most important step is sometime botched, sometime even ignored by the less involved students. What a fool they are! This final step is extremely important, even vital to your schooling, WHATEVER the quality of your work!*

- 作業完了後の振り返り
- 戦略の検証
- 成功・失敗の分析
- **作業の質に関係なく**重要

---

## 2. torinoue / sohyamaz ペア学習ルール

### 稼働時間の分担

| メンバー | 週間稼働 | 役割配分 |
|----------|----------|----------|
| torinoue | 30h/week | メイン実装、ドキュメント |
| sohyamaz | 10h/week | コードレビュー、テスト、質問 |

### 相互学習セッション（推奨）

**週1回、60分のペア学習セッション**を設ける：

| 時間 | 内容 |
|------|------|
| 0-15分 | 進捗共有（torinoue → sohyamaz） |
| 15-35分 | コードウォークスルー（説明する側が学ぶ） |
| 35-50分 | 相互質問・ディスカッション |
| 50-60分 | 次週の計画確認 |

### 説明責任の原則

> *原文 (AI Instructions): Explaining your reasoning and debating with peers often reveals gaps in your understanding. Make peer learning a priority.*

**実装者（主にtorinoue）の義務**:
- 書いたコードを sohyamaz に説明できること
- 「なぜこの設計か」を言語化できること
- AIが生成したコードも完全に理解していること

**レビュー者（主にsohyamaz）の義務**:
- 理解できない部分は必ず質問する
- 「動いているから良い」で終わらせない
- 代替案があれば提案する

---

## 3. ディフェンス（評価）の心構え

### 評価の流れ

*原文: This is our view of what a defense should be:*

1. **対面で会う**（リモート禁止）

   *原文: Never accept a defense if the other student is not physically present next to you. Please read that sentence again.*

2. **時間をかける**（15-20分以上）

   *原文: Whatever the work done (or not done), a defense must last the required time (15 to 20min, a bit more for bigger projects).*

3. **結果を一緒に確認**

   *原文: Notice, together, what works, and what does not. Everybody should agree on the results of the tests, as well as the respect of the scale and the grade.*

4. **アイデアを交換**

   *原文: An exchange. Of ideas, of hypothesis, of solutions. Of the relevance and the quality of the product, its factors of success or failure.*

5. **建設的に終わる**

   *原文: Constructive. Everybody should leave a defense with the feeling of learned something new, either on the technical, relational or organisational side.*

### 評価でのNG行動

| NG | 理由 |
|----|------|
| リモート評価 | 禁止（対面必須） |
| 15分未満で終わる | 時間をかけることが必須 |
| 予定した評価を断る | 絶対禁止（サンクション対象） |
| フィードバックを出さない | 必須（経験値が入らない） |

---

## 4. AI協働ルール（ft_irc用）

### AI使用の原則

> *原文 (AI Instructions): Only use AI-generated content that you fully understand and can take responsibility for.*

| 許可 | 禁止 |
|------|------|
| 設計の相談 | コード丸写し |
| デバッグのヒント取得 | 理解せずに使用 |
| ドキュメント作成補助 | ピアレビューなしでの採用 |
| 概念の説明を受ける | 評価時に説明できないコード |

### 良い例・悪い例

**✓ 良い例**:
> AIにパーサーの設計を相談 → ロジックをペアで確認 → バグを2つ発見して一緒に修正 → 完全に理解した状態で完成

*原文: I use AI to help design a parser. Then I walk through the logic with a peer. We catch two bugs and rewrite it together — better, cleaner, and fully understood.*

**✗ 悪い例**:
> AIにコードを生成させる → コンパイルできたのでそのまま使う → 評価で仕組みを説明できない → **失格**

*原文: I let Copilot generate my code for a key part of my project. It compiles, but I can't explain how it handles pipes. During the evaluation, I fail to justify and I fail my project.*

---

## 5. ft_irc 特有の学習ポイント

### 二人で必ず理解すべき概念

| 概念 | なぜ重要か |
|------|-----------|
| poll() / select() / epoll() | サーバーの心臓部。評価で確実に聞かれる |
| ノンブロッキング I/O | 課題の核心要件 |
| IRC プロトコル（RFC 1459/2812） | クライアントとの通信の基盤 |
| fd 管理 | リソースリーク防止 |
| 部分データの集約 | nc テストで確実にテストされる |

### ペアで実施すべき確認

1. **コードウォークスルー**: 各コマンド（NICK, JOIN, PRIVMSG等）の処理フローを口頭で説明
2. **エッジケーステスト**: 部分データ、接続切断、不正入力などを二人でテスト
3. **評価シミュレーション**: 実際の評価を想定した質疑応答

---

## 6. 相互学習クイズ（実施推奨）

### 使い方

1. 一方が質問を読み上げる
2. もう一方が口頭で回答
3. 回答できなければ、一緒に調べる
4. 両者が回答できるようになるまで繰り返す

### サンプル質問（ソケット基礎）

| # | 質問 | 確認ポイント |
|---|------|-------------|
| 1 | `socket()` の3つの引数は何か？ | AF_INET, SOCK_STREAM, 0 |
| 2 | `bind()` と `connect()` の違いは？ | サーバー vs クライアント |
| 3 | `listen()` の第2引数は何を意味するか？ | backlog |
| 4 | `accept()` は何を返すか？ | 新しいソケット fd |
| 5 | ノンブロッキングにする方法は？ | `fcntl(fd, F_SETFL, O_NONBLOCK)` |

### サンプル質問（IRC プロトコル）

| # | 質問 | 確認ポイント |
|---|------|-------------|
| 1 | IRC メッセージの終端は？ | `\r\n` (CRLF) |
| 2 | NICK コマンドの形式は？ | `NICK <nickname>` |
| 3 | チャンネル名の接頭辞は？ | `#` |
| 4 | オペレーターと一般ユーザーの違いは？ | KICK/INVITE/TOPIC/MODE の権限 |
| 5 | MODE +k は何をするか？ | チャンネルパスワードを設定 |

---

## 7. 週次チェックリスト

### セッション開始時

- [ ] 前回からの進捗を共有した
- [ ] 実装したコードを説明できる状態にした
- [ ] 質問リストを準備した

### セッション終了時

- [ ] 相互に理解できなかった点を特定した
- [ ] 次週の目標を設定した
- [ ] session_log を更新した

---

## 8. 参考リソース

### IRC プロトコル

- [RFC 1459 - Internet Relay Chat Protocol](https://datatracker.ietf.org/doc/html/rfc1459)
- [RFC 2812 - Internet Relay Chat: Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812)

### ソケットプログラミング

- [TCP/IPソケットプログラミング C言語編](https://www.ohmsha.co.jp/book/9784274065194/) - 購入済み
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) - 無料オンライン

### 42 ペア学習

- the_art_of_peer_evaluation.en.pdf（本リポジトリ内）
