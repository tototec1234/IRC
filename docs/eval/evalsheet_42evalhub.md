# 評価スケール（42evalhub）

> 出典: https://www.42evalhub.com/common/ftirc

---

## Mandatory Part

### Basic checks（致命的チェック）

以下のいずれかが不合格の場合、評価は終了し最終評価は **0点** となる。

- Makefileが存在し、プロジェクトが必要なオプションでコンパイルされ、C++で書かれており、実行ファイルが期待通りの名前であること。

  *原文: There is a Makefile, the project compiles correctly with the required options, is written in C++, and the executable is called as expected.*

- コード内に `poll()`（または同等のもの）がいくつあるか確認すること。**1つだけ** でなければならない。

  *原文: Ask and check how many poll() (or equivalent) are present in the code. There must be only one.*

- `poll()`（または同等のもの）が、各 `accept`、`read/recv`、`write/send` の **前に毎回** 呼ばれることを確認すること。これらの呼び出しの後、errnoを使用して特定のアクション（例：`errno == EAGAIN` の後に再度読み込む）をトリガーしてはならない。

  *原文: Verify that the poll() (or equivalent) is called every time before each accept, read/recv, write/send. After these calls, errno should not be used to trigger specific action (e.g. like reading again after errno == EAGAIN).*

- 各 `fcntl()` 呼び出しが以下の形式で行われていることを確認すること：`fcntl(fd, F_SETFL, O_NONBLOCK);` その他の `fcntl()` の使用は禁止。

  *原文: Verify that each call to fcntl() is done as follows: fcntl(fd, F_SETFL, O_NONBLOCK); Any other use of fcntl() is forbidden.*

---

### Networking

以下の要件を確認すること：

- サーバーが起動し、コマンドラインから指定されたポートで全ネットワークインターフェースでリッスンする。

  *原文: The server starts, and listens on all network interfaces on the port given from the command line.*

- `nc` ツールを使用してサーバーに接続し、コマンドを送信でき、サーバーが応答を返す。

  *原文: Using the 'nc' tool, you can connect to the server, send commands, and the server answers you back.*

- チームのリファレンスIRCクライアントを確認すること。

  *原文: Ask the team what is their reference IRC client.*

- このIRCクライアントを使用してサーバーに接続できる。

  *原文: Using this IRC client, you can connect to the server.*

- サーバーが複数の接続を同時に処理できる。サーバーはブロックしてはならない。すべての要求に応答できなければならない。IRCクライアントとncを同時に使ってテストを行うこと。

  *原文: The server can handle multiple connections at the same time. The server should not block. It should be able to answer all demands. Do some test with the IRC client and nc at the same time.*

- 適切なコマンドでチャンネルに参加する。そのチャンネル上の1つのクライアントからのすべてのメッセージが、チャンネルに参加している他のすべてのクライアントに送信されることを確認する。

  *原文: Join a channel thanks to the appropriate command. Ensure that all messages from one client on that channel are sent to all other clients that joined the channel.*

---

### Networking specials

ネットワーク通信は多くの異常な状況で乱される可能性がある。

- 課題と同様に、`nc` を使用して部分的なコマンドを送信してみること。サーバーが正しく応答することを確認する。部分的なコマンドが送信された状態で、他の接続がまだ正常に動作することを確認する。

  *原文: Just like in the subject, using nc, try to send partial commands. Check that the server answers correctly. With a partial command sent, ensure that other connections still run fine.*

- クライアントを予期せず強制終了する。その後、サーバーが他の接続および新しい着信クライアントに対してまだ動作していることを確認する。

  *原文: Unexpectedly kill a client. Then check that the server is still operational for the other connections and for any new incoming client.*

- コマンドの半分だけを送信した状態で `nc` を予期せず強制終了する。サーバーが異常な状態やブロック状態になっていないことを再度確認する。

  *原文: Unexpectedly kill a nc with just half of a command sent. Check again that the server is not in an odd state or blocked.*

- チャンネルに接続したクライアントを停止する（`^-Z`）。その後、別のクライアントを使ってチャンネルをフラッドする。サーバーはハングしてはならない。クライアントが再開したとき、保存されたすべてのコマンドが正常に処理されること。また、この操作中のメモリリークも確認すること。

  *原文: Stop a client (^-Z) connected on a channel. Then flood the channel using another client. The server should not hang. When the client is live again, all stored commands should be processed normally. Also, check for memory leaks during this operation.*

---

### Client Commands basic

- `nc` とリファレンスIRCクライアントの両方で、認証、ニックネーム設定、ユーザー名設定、チャンネル参加ができることを確認する。これは問題なく動作するはず（既に前のステップで確認済み）。

  *原文: With both nc and the reference IRC client, check that you can authenticate, set a nickname, a username, join a channel. This should be fine (you should have already done this previously).*

- プライベートメッセージ（PRIVMSG）が異なるパラメータで完全に機能することを確認する。

  *原文: Verify that private messages (PRIVMSG) are fully functional with different parameters.*

---

### Client Commands channel operator（0-5点評価）

- `nc` とリファレンスIRCクライアントの両方で、一般ユーザーがチャンネルオペレーターのアクションを実行する権限を持っていないことを確認する。その後、オペレーターでテストする。すべてのチャンネル操作コマンドをテストすること（動作しない機能ごとに1点減点）。

  *原文: With both nc and the reference IRC client, check that a regular user does not have privileges to do channel operator actions. Then test with an operator. All the channel operation commands should be tested (remove one point for each feature that is not working).*

評価: 0 1 2 3 4 5

---

## Bonus part

ボーナスパートの評価は、必須パートが完全かつ完璧に完了し、エラー管理が予期しないまたは不正な使用を処理する場合にのみ行う。防御中に必須ポイントがすべて合格しなかった場合、ボーナスポイントは完全に無視される。

*原文: Evaluate the bonus part if, and only if, the mandatory part has been entirely and perfectly done, and the error management handles unexpected or bad usage. In case all the mandatory points were not passed during the defense, bonus points must be totally ignored.*

### File transfer

リファレンスIRCクライアントでファイル転送が動作する。

*原文: File transfer works with the reference IRC client.*

### A small bot

IRCボットが存在する。

*原文: There's an IRC bot.*

---

## チェックリスト（まとめ）

### 致命的チェック（1つでも不合格→0点）

- [ ] Makefile存在、C++、実行ファイル名が正しい
- [ ] `poll()`（または同等）が **1つだけ**
- [ ] `poll()` が accept/read/recv/write/send の **前に毎回** 呼ばれる
- [ ] `errno == EAGAIN` 後の再read禁止
- [ ] `fcntl()` は `fcntl(fd, F_SETFL, O_NONBLOCK)` 形式のみ

### Networking

- [ ] サーバー起動、指定ポートでリッスン
- [ ] `nc` で接続・コマンド送信・応答確認
- [ ] リファレンスIRCクライアントで接続
- [ ] 複数同時接続、ブロックしない
- [ ] チャンネル参加、メッセージ配信

### Networking specials

- [ ] `nc` で partial command → サーバー正常応答
- [ ] クライアント突然kill → サーバー稼働継続
- [ ] `nc` 半分のコマンドで突然kill → サーバー正常
- [ ] クライアント `^-Z` 停止 → channel flood → ハングしない
- [ ] 停止クライアント再開 → 保存コマンド正常処理
- [ ] メモリリーク確認

### Client Commands basic

- [ ] 認証（PASS）
- [ ] ニックネーム設定（NICK）
- [ ] ユーザー名設定（USER）
- [ ] チャンネル参加（JOIN）
- [ ] PRIVMSG 完全動作

### Client Commands channel operator（各項目1点、max 5点）

- [ ] 一般ユーザーに権限なし確認
- [ ] KICK
- [ ] INVITE
- [ ] TOPIC
- [ ] MODE（+i, +t, +k, +o, +l）

### Bonus

- [ ] File transfer
- [ ] IRC bot

---

## Introduction（評価ルール）

評価プロセス中は、以下のルールに従うこと：

- 評価プロセス全体を通して、礼儀正しく、丁寧で、敬意を持ち、建設的であること。コミュニティの健全性はそれにかかっている。

  *原文: Remain polite, courteous, respectful and constructive throughout the evaluation process. The well-being of the community depends on it.*

- 評価対象の学生またはグループと共に、プロジェクトの機能不全の可能性を特定すること。特定された問題について議論し、話し合う時間を取ること。

  *原文: Identify with the student or group whose work is evaluated the possible dysfunctions in their project. Take the time to discuss and debate the problems that may have been identified.*

- 同僚がプロジェクトの指示や機能の範囲をどのように理解したかについて、違いがあるかもしれないことを考慮すること。常にオープンマインドを保ち、できるだけ正直に評価すること。教育は、ピア評価が真剣に行われた場合にのみ有用である。

  *原文: You must consider that there might be some differences in how your peers might have understood the project's instructions and the scope of its functionalities. Always keep an open mind and grade them as honestly as possible. The pedagogy is useful only and only if the peer-evaluation is done seriously.*

---

## Guidelines（評価ガイドライン）

- 評価対象の学生またはグループのGitリポジトリに提出された作業のみを評価すること。

  *原文: Only grade the work that was turned in the Git repository of the evaluated student or group.*

- Gitリポジトリが学生に属していることを再確認すること。プロジェクトが期待されるものであることを確認する。また、空のフォルダで `git clone` が使用されていることを確認する。

  *原文: Double-check that the Git repository belongs to the student(s). Ensure that the project is the one expected. Also, check that 'git clone' is used in an empty folder.*

- 悪意のあるエイリアスが使用されていないか注意深く確認すること。公式リポジトリの内容ではないものを評価させられないようにする。

  *原文: Check carefully that no malicious aliases was used to fool you and make you evaluate something that is not the content of the official repository.*

- 驚きを避けるため、該当する場合は、評価を容易にするために使用されるスクリプト（テストまたは自動化用のスクリプト）を一緒に確認すること。

  *原文: To avoid any surprises and if applicable, review together any scripts used to facilitate the grading (scripts for testing or automation).*

- 評価するアサインメントを完了していない場合、評価プロセスを開始する前に課題全体を読むこと。

  *原文: If you have not completed the assignment you are going to evaluate, you have to read the entire subject prior to starting the evaluation process.*

- 空のリポジトリ、動作しないプログラム、Normエラー、不正行為などを報告するために利用可能なフラグを使用すること。これらの場合、評価プロセスは終了し、最終評価は0点、または不正行為の場合は-42点となる。ただし、不正行為を除き、学生は将来繰り返すべきでない間違いを特定するために、提出された作業を一緒にレビューすることを強く推奨する。

  *原文: Use the available flags to report an empty repository, a non-functioning program, a Norm error, cheating, and so forth. In these cases, the evaluation process ends and the final grade is 0, or -42 in case of cheating. However, except for cheating, student are strongly encouraged to review together the work that was turned in, in order to identify any mistakes that shouldn't be repeated in the future.*

- 防御の期間中、セグメンテーション違反、その他の予期しない、早すぎる、制御されていない、または予期しないプログラムの終了があってはならない。そうでなければ最終評価は0点となる。適切なフラグを使用すること。設定ファイルが存在する場合を除き、ファイルを編集する必要があってはならない。ファイルを編集したい場合は、評価対象の学生にその理由を説明し、双方が同意していることを確認すること。

  *原文: Remember that for the duration of the defence, no segfault, no other unexpected, premature, uncontrolled or unexpected termination of the program, else the final grade is 0. Use the appropriate flag. You should never have to edit any file except the configuration file if it exists. If you want to edit a file, take the time to explicit the reasons with the evaluated student and make sure both of you are okay with this.*

- メモリリークがないことも確認すること。ヒープに割り当てられたすべてのメモリは、実行終了前に適切に解放されなければならない。leaks、valgrind、e_fenceなど、コンピュータで利用可能な様々なツールを使用することが許可されている。メモリリークがある場合は、適切なフラグにチェックを入れること。

  *原文: You must also verify the absence of memory leaks. Any memory allocated on the heap must be properly freed before the end of execution. You are allowed to use any of the different tools available on the computer, such as leaks, valgrind, or e_fence. In case of memory leaks, tick the appropriate flag.*

---

## 技術用語

| 英語 | 日本語 | 説明 |
|------|--------|------|
| poll() | poll() | 複数のファイルディスクリプタを監視するシステムコール |
| fcntl() | fcntl() | ファイルディスクリプタの属性を制御するシステムコール |
| O_NONBLOCK | O_NONBLOCK | ノンブロッキングモードを示すフラグ |
| errno | errno | 直前のシステムコールのエラーコードを保持するグローバル変数 |
| EAGAIN | EAGAIN | 「今はデータがない、後で再試行せよ」を示すerrno値 |
| partial command | 部分コマンド | 複数パケットに分割されて届くコマンド |
| reference client | リファレンスクライアント | 動作確認の基準とするIRCクライアント |
| channel operator | チャンネルオペレーター | チャンネル管理権限を持つユーザー |
| segfault | セグメンテーション違反 | 不正なメモリアクセスによるプログラムクラッシュ |
| memory leak | メモリリーク | 解放されないメモリが蓄積する問題 |

---

## 学習ポイント

1. **致命的チェックの重要性**: Basic checksが1つでも不合格なら即0点。poll()の数、fcntl()の形式は最優先で確認すべき。

2. **Networking specialsの難易度**: partial command、突然のkill、`^-Z`停止+floodは実装難易度が高い。これらを想定したバッファ管理が必要。

3. **評価者視点の理解**: このドキュメントは「評価者が何をチェックするか」のリスト。実装時にこれを意識することで、評価で落とされるリスクを減らせる。

4. **メモリリーク検出ツール**: macOSでは`leaks`、Linuxでは`valgrind`を使う。評価前に必ず実行すること。

5. **ボーナスの条件**: 必須パートが「完全かつ完璧」でないとボーナスは採点されない。まず必須を固めること。
