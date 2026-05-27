# Chapter VII: Submission and peer-evaluation

> 原文: ft_irc.pdf (Version 10.0) - Pages 12-13

---

## 提出

通常通り、課題を Git リポジトリに提出すること。ディフェンス中に評価されるのはリポジトリ内の作業のみ。

*原文: Submit your assignment to your Git repository as usual. Only the work inside your repository will be evaluated during the defense.*

ファイル名が正しいことを再確認することをためらわないこと。

*原文: Do not hesitate to double-check the names of your files to ensure they are correct.*

---

## テストプログラム

提出・評価対象ではないが、プロジェクト用のテストプログラムを作成することを推奨する。

*原文: You are encouraged to create test programs for your project even though they will not be submitted or graded.*

これらのテストは、ディフェンス中に自分のサーバーをテストするのに特に役立つ。また、将来他の ft_irc を評価する際にも使用できる。

*原文: Those tests could be especially useful to test your server during defense, but also your peer's if you have to evaluate another ft_irc one day.*

評価プロセス中に必要なテストを自由に使用できる。

*原文: Indeed, you are free to use whatever tests you need during the evaluation process.*

リファレンスクライアントは評価プロセスで使用される。

*原文: Your reference client will be used during the evaluation process.*

---

## ディフェンス中の修正要求

評価中に、プロジェクトの簡単な修正が時折要求される場合がある。これには、軽微な動作変更、数行のコードの記述または書き換え、追加しやすい機能などが含まれる。

*原文: During the evaluation, a brief modification of the project may occasionally be requested. This could involve a minor behavior change, a few lines of code to write or rewrite, or an easy-to-add feature.*

このステップがすべてのプロジェクトに適用されるわけではないが、評価ガイドラインに記載されている場合は準備しておくこと。

*原文: While this step may not be applicable to every project, you must be prepared for it if it is mentioned in the evaluation guidelines.*

このステップは、プロジェクトの特定の部分についての実際の理解度を確認するためのものである。

*原文: This step is meant to verify your actual understanding of a specific part of the project.*

修正は選択した開発環境（通常のセットアップなど）で行うことができ、特定の時間枠が評価の一部として定義されていない限り、数分以内に実行可能であるべき。

*原文: The modification can be performed in any development environment you choose (e.g., your usual setup), and it should be feasible within a few minutes — unless a specific timeframe is defined as part of the evaluation.*

例として、関数やスクリプトの小さな更新、表示の修正、新しい情報を格納するためのデータ構造の調整などを求められる可能性がある。

*原文: You can, for example, be asked to make a small update to a function or script, modify a display, or adjust a data structure to store new information, etc.*

詳細（範囲、対象など）は評価ガイドラインで指定され、同じプロジェクトでも評価ごとに異なる場合がある。

*原文: The details (scope, target, etc.) will be specified in the evaluation guidelines and may vary from one evaluation to another for the same project.*

---

## 評価時に想定される修正例

| 種類 | 例 |
|------|-----|
| 軽微な動作変更 | 特定のエラーメッセージの変更 |
| 数行のコード | 新しい MODE オプションの追加 |
| 追加しやすい機能 | 簡単なサーバーコマンドの追加 |
| 表示の修正 | ログ出力のフォーマット変更 |
| データ構造の調整 | クライアント情報に新しいフィールドを追加 |

---

## 評価準備のポイント

1. **コードの完全な理解**: すべてのコードを説明できること
2. **即座の修正対応**: 開発環境をすぐに使える状態にしておく
3. **テストの準備**: 自作テストとリファレンスクライアントの両方
4. **時間管理**: 数分以内に修正を完了できるようにする

---

## ピア評価について

課題書とは別に、`the_art_of_peer_evaluation.en.pdf` に評価の心構えが記載されている。主なポイント：

- 対面で行う（リモート禁止）
- 時間をかける（15-20分以上）
- 相互に学び合う姿勢
- フィードバックは必須
