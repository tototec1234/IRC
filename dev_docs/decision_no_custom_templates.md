# 設計決定: 自作テンプレートを使わない

> **ステータス**: 決定（2026-05-29）  
> **根拠**: `design.md` / `interface.md` に基づくテンプレート導入可否の分析  
> **関連**: [design.md](./design.md), [interface.md](./interface.md), [chapter2_general_rules.md](../docs/chapter2_general_rules.md)

---

## 1. 決定内容

本プロジェクト（ircserv）では、**自作テンプレート（`template` キーワードで定義するクラス・関数）を導入しない**。

- `.tpp` / `.ipp` によるテンプレート実装の分離も行わない
- データ構造は **具象クラス + C++98 標準ライブラリ（STL）** で実装する
- 層間 API（`interface.md`）は変更しない

---

## 2. 用語の区別

| 区分 | 例 | 本決定での扱い |
|------|-----|----------------|
| **STL 利用** | `std::vector`, `std::map`, `std::string` | **使用する**（課題要件・現設計どおり） |
| **自作テンプレート** | `template<typename K, typename V> class Registry` | **使用しない** |

`std::vector<std::string>` は内部的にテンプレートだが、利用者が `template` を書く必要はない。  
本決定が禁じるのは後者（自作の汎用化）のみである[^010]。

---

## 3. 論拠

### 3.1 現設計で自作テンプレートが不要

`design.md` / `interface.md` で定義されたクラスは、すべて具象型の API である。

| 層 | 主要クラス | データ構造（設計上） |
|----|-----------|---------------------|
| A | `Server`, `Connection` | `std::string`, `std::vector<struct pollfd>`, optional で `std::map<int, Connection*>` |
| B | `Message`, `Parser`, `CommandDispatcher`, `ReplyBuilder` | `std::vector`, `std::string`, static メソッド |
| C1 | `Client`, `ServerState` | `std::map` による fd / nick / channel 辞書 |
| C2 | `Channel`, `ChannelModes` | `std::vector<Client*>`、`Client*` 集合 |

公開 API にテンプレートパラメータは存在しない。  
MVP[^020] から最終提出まで、課題要件（PASS / NICK / USER / JOIN / PRIVMSG / KICK / INVITE / TOPIC / MODE 等）を満たすのに自作テンプレートは必須ではない。

### 3.2 導入候補を検討したが ROI[^030] が低い

分析で挙がった自作テンプレート候補と、採用しなかった理由:

| 候補 | 想定担当 | 期待効果 | 不採用理由 |
|------|----------|----------|------------|
| `Registry<K,V>` | C1 / 共有 | 辞書操作の DRY[^040] | `std::map` の薄いラッパーに留まりがち。バグ削減効果は `updateNick` / `removeClient` の実装規律で代替可能 |
| `ClientPtrSet` | C2 | member / operator / invited 集合の共通化 | `std::set<Client*>` または小さな具象クラスで足りる。テンプレートの得が小さい |
| `NumericReply` template | B | numeric 返信の共通化 | `ReplyBuilder` の static メソッド設計で十分。可読性を損なう |
| `CommandTable` template | B | ルーティング整理[^050] | `std::map<std::string, 関数ポインタ>` または switch / if チェーンで足りる |

いずれも「コード行数削減」以上の効果（型安全性の大幅向上、複数型への再利用）が ft_irc の規模では見込めない。

### 3.3 チーム構成との整合

- 現設計者は具象クラス + STL を前提に API を定義している
- template 経験者は 1 名参加予定だが担当未定
- 自作テンプレートを導入すると、共有ヘッダの設計・レビューがその 1 人に集中し、ボトルネックになる

層境界（A↔B は `CommandResult`、B↔C1/C2 は公開 API のみ）を維持するには、**内部実装を具象のまま各担当が独立して進める**方が並行開発に適する。

### 3.4 C++98 制約下でのコスト

課題は C++98 のみ（`-std=c++98`）[^060]。

自作テンプレートを C++98 で書く場合:

- イテレータ型の明示が冗長になる（`std::map<...>::iterator`）
- コンパイルエラーメッセージが長く、テンプレート未経験者には読みにくい
- `auto`、range-for、`std::unordered_map` が使えない

STL を「使う側」であれば iterator 明示は必要だが、**テンプレート定義・実装分離（`.tpp`）の知識は不要**である。  
自作テンプレートを足すと、全員がその分の学習コストを負う。

### 3.5 代替案（C 風・手書きコンテナ）も不採用

`std::map` / `std::vector` を避け、配列や自前リストで辞書を実装する案[^070]も検討対象外とした。

- 一般ルールは C++ 機能の利用を推奨している
- メモリ管理・イテレーションのバグリスクが増える
- ft_irc の規模に対して工数が見合わない

→ **STL 利用 + 具象クラス** が妥当な中間点である。

### 3.6 課題要件との関係

課題書は C++98 準拠と `.tpp` / `.ipp` の提出を許可しているが、**テンプレート使用を義務付けていない**（`chapter4_mandatory_part.md`）。

自作テンプレートなしでも提出要件を満たせる。

---

## 4. 採用する実装方針

### 4.1 データ構造

| 用途 | 採用 |
|------|------|
| 可変長配列 | `std::vector` |
| 辞書（fd / nick / channel） | `std::map` |
| 文字列 | `std::string` |
| Client* 集合（重複なし） | `std::set<Client*>` または `std::vector<Client*>` + 線形探索（規模が小さいため） |

`std::unordered_map` は C++11 のため使用しない[^060]。

### 4.2 C++98 でのコーディング

`A_fd_responsibility_design.md` Section 5 に従う:

- `auto` の代わりにイテレータ型を明示する
- range-for の代わりに `for (it = begin(); it != end(); ++it)` を使う
- `nullptr` の代わりに `NULL` を使う

これは「テンプレートを書く」必要はなく、「STL を C++98 文法で使う」範囲である。

### 4.3 重複コードへの対処

辞書操作のコピペ（DRY 違反気味な箇所）は、テンプレートではなく以下で対処する:

- **private メソッド**に共通処理を寄せる（例: `ServerState` 内の nick 更新ロジック）
- optional 分離時は **具象クラス**（`ClientRegistry`）で責務を分ける
- 必要なら **非テンプレートの free 関数**（特定型専用）を `utils/` に置く

---

## 5. 見直し条件

以下のいずれかが起きた場合のみ、チームで自作テンプレート導入を再議する。

1. 同一パターンの `std::map` 操作が **5 箇所以上**に増え、private メソッド / 具象クラスでは保守が困難になった
2. optional 分離（`ConnectionManager`, `ClientRegistry`）後も辞書同期バグが **テンプレート以外では防げない**と判断された
3. 全員が共有 utils の API に合意し、template 経験者が **owner として維持できる**

再議しても導入候補は **共有 `Registry` 1 種類に限定**する。B 層の ReplyBuilder 等への波及は行わない。

---

## 6. 担当者への影響

| 担当 | 影響 |
|------|------|
| A | `std::vector<struct pollfd>`, optional で `std::map<int, Connection*>`。変更なし |
| B | `std::vector`, `std::string`。変更なし |
| C1 | `std::map` 3 種。`updateNick` / `removeClient` の整合性に注力 |
| C2 | `std::vector<Client*>` 等。変更なし |

**template 経験者**は必須スキルではない。参加する場合はレビュー・ペアプログラミング・C++98 iterator 周りの助言が主な役割となる。

---

## 7. 関連ドキュメント

| ドキュメント | 内容 |
|-------------|------|
| [design.md](./design.md) | 層構成・クラス責務 |
| [interface.md](./interface.md) | 層間 API 契約 |
| [A_fd_responsibility_design.md](./A_fd_responsibility_design.md) | C++98 コーディング注意 |
| [chapter2_general_rules.md](../docs/chapter2_general_rules.md) | C++98 制約・禁止機能 |

---

## 脚注

[^010]: **テンプレート** — 型や値を `<>` で差し替え可能にする C++ の仕組み。`std::vector` はテンプレートクラスだが、利用者が `template` キーワードを書く必要はない。本決定の「自作テンプレート」は、`template<typename T>` のように自分で定義するものを指す。

[^020]: **MVP（Minimum Viable Product）** — 最小実用製品。本プロジェクトでは `design.md` Section 11 に定義された、まず動かす最小機能セット（登録・JOIN・PRIVMSG 等）を指す。

[^030]: **ROI（Return On Investment）** — 投資対効果。導入コスト（学習・複雑さ・時間）に対して得られる benefit（バグ削減・保守性）が見合うかの判断基準。

[^040]: **DRY（Don't Repeat Yourself）** — 同じロジックをコピペで散らさない設計原則。辞書操作の重複を避ける動機にはなるが、ft_irc の必須要件ではない。

[^050]: **ルーティング整理** — 入力（IRC コマンド名等）を正しい処理関数へ振り分ける構造の整理。`CommandDispatcher` が NICK / JOIN 等をどの handler に渡すかを決める部分。

[^060]: **C++98** — 1998 年標準化の C++。`-std=c++98` でコンパイル可能であることが課題要件。`auto`, `nullptr`, range-for, `std::unordered_map` 等は使用不可。

[^070]: **C 風・手書きコンテナ** — `std::vector` / `std::map` を使わず、C のように動的配列や自前ハッシュ表を実装する方式。本プロジェクトでは不採用。

---

## 変更履歴

| 日付 | 内容 |
|------|------|
| 2026-05-29 | 初版（テンプレート導入可否分析に基づく設計決定） |
