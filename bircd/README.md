# bircd 学習ガイド
> **作成者**: torinoue  

ft_irc 向けの参考実装 **bircd**（C）を読み、改造し、最終的に C++98 の A 層（`src/a/Server.cpp`）へ橋渡しするための教材です。

---

## 前提（必読）

| 項目 | 内容 |
|------|------|
| **Phase 1** | **必修。** select 版 bircd の読解を終えてから Phase 2 以降に進む |
| **スキル** | C の基礎、`socket` / `bind` / `listen` / `accept`、`select()` と `fd_set` が読めること |
| **Git** | ブランチ checkout 程度は経験あり。**タグは初見可**（下記「Git タグ入門」で学ぶ） |
| **言語** | 本ディレクトリのドキュメントは日本語 |

Phase 1 を飛ばすと Phase 2 の「なぜこう変えるか」が理解できません。

---

## ドキュメント一覧

| ファイル | 用途 |
|----------|------|
| [README.md](README.md) | 本ファイル。全体像・ブランチ・**タグ**の使い方 |
| [bircd_learning_curriculum.md](bircd_learning_curriculum.md) | Phase 1〜5 の全カリキュラム |
| [bircd_learning_curriculum_ans.md](bircd_learning_curriculum_ans.md) | 解答・メモ（Phase 2 以降の答え合わせはここへリンク） |
| [phase2_guide.md](phase2_guide.md) | **Phase 2 ハンズオン**（select → poll） |
| [bircd_analysis.md](bircd_analysis.md) | bircd 構造の分析メモ |
| [poll_check.c](poll_check.c) | `POLLIN` 等のビットフラグ確認用（Phase 2 前の実験） |

---

## Git タグ入門（メタ学習）

bircd カリキュラムでは **Phase ごとに Git タグ** を使う。  
タグ自体も学習の一部 — ft_irc 開発でもリリース版の目印などに使われる。

### タグとは

**特定のコミットに付ける「名前付きしおり」**。

```
learn/bircd-curriculum ブランチ（先端は右へ伸びる）
    │
    ●───●───●───●
    │       │
 phase-1  phase-2   ← タグはここに固定。Phase 3 を足しても動かない
```

| 概念 | 例 | 動く？ | 用途 |
|------|-----|--------|------|
| **コミット SHA** | `bbc74b9` | 動かない | コミットそのもの。覚えにくい |
| **タグ** | `phase-1` | 動かない | SHA への **人間が読める別名** |
| **ブランチ** | `learn/bircd-curriculum` | **先端が進む** | 作業・追加開発 |

**覚え方:** ブランチ = 伸びる枝。タグ = 幹に付けた **位牌**（その地点を記録）。

### 本リポジトリのタグ一覧

| タグ | 指す Phase | bircd の状態 |
|------|-----------|-------------|
| `phase-1` | Phase 1 完了 | select 版 + 教材（コード変更なし） |
| `phase-2` | Phase 2 完了 | poll 版 |
| `phase-3` 以降 | （未作成） | ブランチに追加されたらタグも増える |

タグが指すコミットを確認:

```bash
git show phase-1 --oneline --no-patch
git show phase-2 --oneline --no-patch
```

### よく使うコマンド

```bash
# タグ一覧
git tag

# タグの説明付きで見る（annotated tag）
git show phase-1

# Phase 1 のコード全体に移動（読むだけ）
git checkout phase-1

# Phase 1 から自分用ブランチを切って Phase 2 を自力実装
git checkout -b my-phase2-work phase-1

# Phase 1 → Phase 2 の差分だけ見る（最重要）
git diff phase-1 phase-2 -- bircd/

# 特定ファイルだけ Phase 1 版を表示
git show phase-1:bircd/init_fd.c

# ブランチ先端（最新 Phase）に戻る
git checkout learn/bircd-curriculum
```

### リモートから取得

ブランチだけ fetch しても **タグは自動では来ない** ことが多い。

```bash
git fetch origin learn/bircd-curriculum
git fetch origin tag phase-1 tag phase-2
# または全タグ
git fetch origin --tags
```

push 側（教材管理者）:

```bash
git push origin learn/bircd-curriculum
git push origin phase-1 phase-2
```

### detached HEAD とは（注意）

`git checkout phase-1` すると **detached HEAD** 状態になる。

- **できること:** 読む、ビルドする、`git diff` する
- **危ないこと:** そのまま編集してコミット → 枝のないコミットが迷子になる

**自分で Phase 2 を書くとき** は必ず:

```bash
git checkout -b my-phase2-work phase-1
```

### タグの自己チェック（メタ学習）

口に出して答えられるか:

1. タグとブランチの違いは？（→ タグは固定、ブランチは先端が進む）
2. `git checkout phase-1` と `git checkout learn/bircd-curriculum` の bircd/ の違いは？
3. `git diff phase-1 phase-2 -- bircd/` は何を見せる？
4. なぜ `git push` だけではタグが共有されないことがある？
5. Phase 2 を自力実装するとき、なぜ `git checkout -b my-work phase-1` が安全？

---

## 学習ブランチの使い方

学習用の変更は **`learn/bircd-curriculum` ブランチ1本** に Phase ごとのコミット + **タグ** で管理する。

### 初回セットアップ

```bash
cd IRC_torinoue
git fetch origin learn/bircd-curriculum
git fetch origin --tags
git checkout learn/bircd-curriculum
```

### 典型的な学習フロー

**A) Phase 1 を読むだけ**

```bash
git checkout phase-1
cd bircd && make re && ./bircd 6667
# 読み終わったら
git checkout learn/bircd-curriculum
```

**B) Phase 2 を自力で書く（推奨）**

```bash
git checkout -b my-phase2-work phase-1
# phase2_guide.md に従って実装 …
git diff phase-1 -- bircd/          # 自分の変更確認
git diff my-phase2-work phase-2 -- bircd/   # 模範解答との差分
```

**C) Phase 2 完成版をそのまま動かす**

```bash
git checkout phase-2
cd bircd && make re && ./bircd 6667
git checkout learn/bircd-curriculum
```

### ビルド・動作確認（共通）

```bash
cd bircd
make re
./bircd 6667
```

別ターミナル:

```bash
nc localhost 6667
```

---

## Phase ロードマップ

```
Phase 1  bircd 完全理解（select 版）     … 必修          [tag: phase-1]
Phase 2  select → poll 変換              … [phase2_guide.md](phase2_guide.md)  [tag: phase-2]
Phase 3  受信・送信バッファリング         … 未着手
Phase 4  ノンブロッキング I/O             … 未着手
Phase 5  C++98 化（A 層 Server）          … ft_irc 本体（bircd 外）
```

| Phase | bircd の状態 | 学ぶこと |
|-------|-------------|----------|
| 1 | `select()` + `fd_set` | socket 系 API、メインループ、コールバック設計 |
| 2 | `poll()` + `pollfd[]` | I/O 多重化の書き換え、`events` / `revents` |
| 3 | + バッファ | TCP ストリーム、`\r\n` 切り出し、POLLOUT 制御 |
| 4 | + 非ブロッキング | `fcntl`、`EAGAIN`、切断検知 |
| 5 | C++98 `Server` | `std::vector<pollfd>`、RAII、責務分割 |

---

## Phase 2 へ進む前のチェックリスト

Phase 1 完了の目安（[bircd_learning_curriculum.md](bircd_learning_curriculum.md) 参照）:

- [ ] `main_loop` → `init_fd` → `do_select` → `check_fd` の流れを説明できる
- [ ] `FD_ZERO` / `FD_SET` / `FD_ISSET` の役割を説明できる
- [ ] `fds[fd番号]` で fd をインデックスしている理由がわかる
- [ ] 上記「タグの自己チェック」5問に答えられる

問題なければ → **[phase2_guide.md](phase2_guide.md)** を開く。

---

## ft_irc（A 層）との対応

bircd で学んだ概念は ft_irc にそのまま載ります。

| bircd（Phase 2） | ft_irc A 層 |
|------------------|-------------|
| `struct pollfd pollfds[MAX_CLIENTS + 1]` | `std::vector<struct pollfd> _pollfds` |
| `nfds`（使用中 fd の個数） | `_pollfds.size()` |
| `init_fd()` で毎ループ配列再構築 | `_addFd()` / `_removeFd()` |
| `events = POLLIN` / `\|= POLLOUT` | `_enablePollout()` / `_disablePollout()` |
| `check_fd()` → `fct_read` | `Server::run()` 内の `POLLIN` 処理 |

Phase 2 完了後は `src/a/Server.cpp`（または `a_tmp/Server.cpp`）を読み、「固定配列が vector になっただけ」と捉えると理解が早いです。

---

## 定数（Phase 2 以降の正）

```c
#define MAX_CLIENTS 42
struct pollfd pollfds[MAX_CLIENTS + 1];  // listen fd 1 + client 最大 42
```

- `+1` は **listen fd 分**
- ft_irc 提出物では上限を固定せず `vector` で OS の fd 上限まで伸ばす想定

---

## 困ったとき

1. [bircd_learning_curriculum_ans.md](bircd_learning_curriculum_ans.md) の Phase 2 セクション
2. `man 2 poll`
3. `git diff phase-1 phase-2 -- bircd/` で模範 diff を確認
4. チーム内で質問

---

## 教材管理者向け: タグの付け方

Phase コミットを追加したあと、**そのコミットにタグを付けて push** する。

```bash
# Phase 1 完了コミット（select 版 + 教材）にタグ
git tag -a phase-1 -m "Phase 1: select 版 bircd + 教材"

# Phase 2 完了コミット（poll 版）にタグ
git tag -a phase-2 -m "Phase 2: select → poll 変換完了"

# 付け直す場合（コミットを reset したあとなど）
git tag -d phase-2
git tag -a phase-2 -m "Phase 2: select → poll 変換完了"

# リモートへ
git push origin learn/bircd-curriculum
git push origin phase-1 phase-2
```

---

## コラム: 「bircd」という名前の由来

> 公式な命名理由は Epitech 教材内に明示されておらず確証はない。以下、まず確実な部分（`d`）、続いて頭文字 `b` の有力説、同名別プロジェクトの順に併記する。

### 確実な部分: 末尾の `d` = daemon

Unix / Linux の慣習で、**バックグラウンドで常駐するサーバプログラム**には末尾に小文字 `d` を付ける命名規則がある。

| プログラム | 正式名 |
|------------|--------|
| `httpd` | HTTP daemon（Apache 等） |
| `sshd` | SSH daemon |
| `ftpd` | FTP daemon |
| `inetd` | Internet super-server daemon |
| `crond` | cron daemon |
| `syslogd` | system logger daemon |
| `mysqld` / `mariadbd` | MySQL / MariaDB daemon |
| **`ircd`** | **IRC daemon** |

**daemon の語源:**
- 元はギリシャ語 δαίμων (daimōn = 守護霊・精霊)
- MIT Project MAC (1963) のスタッフが「裏で黙々と仕事をするプロセス」を Maxwell の悪魔（Maxwell's demon）になぞらえた命名
- ユーザーと直接対話せず、システムの裏で動き続けるプロセスを指す

つまり `ircd` = **IRC daemon** = IRC プロトコルを話すサーバ常駐プログラム。

> 補足: ft_irc で作る `ircserv` も実質 ircd の一種だが、**fork 禁止**（課題要件）なので厳密にはフォアグラウンドで動く IRC サーバ。

#### 42 の他課題でも同じ規則に遭遇する

例えば後期課題 **Inception** で Docker コンテナに MariaDB を立てるとき、起動エントリポイントが `mariadbd` であることに気づく。これも当然「daemon」の `d`。歴史的には `mysqld`（MySQL daemon）が標準だったが、MariaDB 10.4 以降は `mariadbd` がデフォルト名として独立し、同じ命名規則を踏襲している。

つまり 42 のカリキュラムは **「サーバ常駐プログラム = 末尾 `d`」という Unix 文化に複数の課題で繰り返し触れさせる**構造になっており、`bircd` の `d` もこの流儀の延長線上にある。`ircserv` という非 `d` 命名はむしろ **fork 禁止 ＝ 厳密には daemon ではない**ことを示唆していると読むこともできる。

### 頭文字 `b` の有力説

1. ⭐ **basic IRCd** — 「最小限の IRC サーバ雛形」を素直に表す。Epitech Tek2（2 年生）ネットワークプログラミング教材の文脈にぴたり合う。実際 `client_read.c` を見ても **IRC プロトコルを一切持たない TCP echo broadcaster** で、文字通り "basic"。
2. **bare-bones IRCd** — 直訳「骨と皮だけ」。コード量・機能とも極小（<300 行）で骨格しかない。
3. **boilerplate IRCd** — 出発点として配布される雛形。学生はここに肉付けする。Epitech 教材的位置づけに合致。
4. **beginner('s) IRCd** — 初学者向け。Epitech Tek2 向け教材という文脈と整合。

### 同名・別プロジェクト（こちらは無関係）

5. **beware ircd** — 偶然同じ略称をもつ別実在の IRC サーバ（[bircd.org](https://www.bircd.org/) / [ircd.bircd.org](https://ircd.bircd.org/history.html)）。Stskeeps 作の Windows 系本格 IRCd。**Epitech 教材の bircd とは無関係**だが、語感は同じ。

### 結論

`d` = daemon は確実。頭文字 `b` は辞書なし正典なしだが、教材文脈と素直なネーミング規則から **basic IRCd** が最も妥当。公式 Epitech シラバスやリファレンスに明記が見つかれば、本節を更新すること。

**参考リンク:**
- [GitHub - anders/bircd](https://github.com/anders/bircd/blob/master/ircd.conf.example) — beware ircd 系の派生
- [GitHub topics: tek2](https://github.com/topics/tek2) — Epitech Tek2 のネットワーク系プロジェクト群
- [Wikipedia: Daemon (computing)](https://en.wikipedia.org/wiki/Daemon_(computing)) — daemon の由来と MIT の命名経緯
- [MariaDB Knowledge Base: mariadbd Options](https://mariadb.com/kb/en/mariadbd-options/) — `mariadbd` の公式リファレンス（42 Inception 課題でおなじみ）

---

## 関連リンク

- カリキュラム全体: [bircd_learning_curriculum.md](bircd_learning_curriculum.md)
- A 層設計: [../dev_docs/design.md](../dev_docs/design.md)
- Git タグ公式: [git-scm.com/docs/git-tag](https://git-scm.com/docs/git-tag)
