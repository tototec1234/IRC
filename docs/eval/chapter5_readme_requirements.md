# Chapter V: Readme Requirements

> 原文: ft_irc.pdf (Version 10.0) - Page 10

---

## README.md の配置

`README.md` ファイルを Git リポジトリのルートに配置すること。

*原文: A README.md file must be provided at the root of your Git repository.*

---

## 目的

プロジェクトに不慣れな人（ピア、スタッフ、リクルーターなど）が、プロジェクトの概要、実行方法、トピックに関する詳細情報の入手先をすぐに理解できるようにすること。

*原文: Its purpose is to allow anyone unfamiliar with the project (peers, staff, recruiters, etc.) to quickly understand what the project is about, how to run it, and where to find more information on the topic.*

---

## 必須セクション

README.md には少なくとも以下を含めること：

*原文: The README.md must include at least:*

### 1. 最初の行（イタリック体）

最初の行は**イタリック体**で、以下のように記載すること：

*原文: The very first line must be italicized and read:*

```
*This project has been created as part of the 42 curriculum by <login1>[, <login2>[, <login3>[...]]].*
```

例: `*This project has been created as part of the 42 curriculum by torinoue, sohyamaz.*`

### 2. "Description" セクション

プロジェクトを明確に紹介し、目標と概要を含めること。

*原文: A "Description" section that clearly presents the project, including its goal and a brief overview.*

### 3. "Instructions" セクション

コンパイル、インストール、実行に関する関連情報を含めること。

*原文: An "Instructions" section containing any relevant information about compilation, installation, and/or execution.*

### 4. "Resources" セクション

トピックに関連する定番のリファレンス（ドキュメント、記事、チュートリアルなど）のリスト。また、**AIの使用方法**（どのタスクに対して、プロジェクトのどの部分に使用したか）の説明を含めること。

*原文: A "Resources" section listing classic references related to the topic (documentation, articles, tutorials, etc.), as well as a description of how AI was used — specifying for which tasks and which parts of the project.*

---

## 追加セクション

プロジェクトによっては追加のセクションが必要になる場合がある（使用例、機能リスト、技術的選択など）。必要な追加事項は以下に明記される。

*原文: Additional sections may be required depending on the project (e.g., usage examples, feature list, technical choices, etc.). Any required additions will be explicitly listed below.*

---

## 言語

READMEは**英語**で記述すること。

*原文: Your README must be written in English.*

---

## README テンプレート

```markdown
*This project has been created as part of the 42 curriculum by torinoue, sohyamaz.*

## Description

ft_irc is an IRC server implementation in C++98. [目標と概要を記載]

## Instructions

### Compilation

```bash
make
```

### Execution

```bash
./ircserv <port> <password>
```

### Example

```bash
./ircserv 6667 mypassword
```

## Resources

### IRC Protocol

- [RFC 1459 - Internet Relay Chat Protocol](https://datatracker.ietf.org/doc/html/rfc1459)
- [RFC 2812 - Internet Relay Chat: Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812)

### Socket Programming

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

### AI Usage

AI was used for the following tasks:
- [AIを使用したタスクと部分を記載]
```

---

## AI使用の記載について

課題書 Chapter III（AI Instructions）では、AIの使用は許可されているが、以下が求められている：

1. 生成されたコンテンツを完全に理解し、責任を持てること
2. ピアレビューを求めること（AIの検証だけに頼らない）
3. 評価時に説明できること

README の "Resources" セクションでは、具体的に以下を記載することを推奨：

- AIを使用した**タスク**（例：設計、デバッグ、ドキュメント作成）
- AIを使用した**プロジェクトの部分**（例：パーサー設計、エラーハンドリング）
