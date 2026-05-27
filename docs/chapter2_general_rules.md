# Chapter II: General rules

> 原文: ft_irc.pdf (Version 10.0) - Page 3

---

## クラッシュ禁止

プログラムはいかなる状況でもクラッシュしてはならない（メモリ不足の場合でも）。また、予期せず終了してはならない。

*原文: Your program should not crash in any circumstances (even when it runs out of memory), and should not quit unexpectedly.*

これが発生した場合、プロジェクトは機能していないとみなされ、評価は0点となる。

*原文: If it happens, your project will be considered non-functional and your grade will be 0.*

---

## Makefile 要件

ソースファイルをコンパイルするMakefileを提出すること。不必要な再リンクを行ってはならない。

*原文: You have to turn in a Makefile which will compile your source files. It must not perform unnecessary relinking.*

Makefileには少なくとも以下のルールを含めること：

*原文: Your Makefile must at least contain the rules:*

- `$(NAME)`
- `all`
- `clean`
- `fclean`
- `re`

---

## コンパイルフラグ

`c++` を使用し、以下のフラグでコンパイルすること：

*原文: Compile your code with c++ using the flags -Wall -Wextra -Werror.*

```
-Wall -Wextra -Werror
```

---

## C++98 標準

コードは **C++ 98 標準** に準拠しなければならない。`-std=c++98` フラグを追加してもコンパイルできる必要がある。

*原文: Your code must comply with the C++ 98 standard. Then, it should still compile if you add the flag -std=c++98.*

---

## C++ 機能の優先使用

可能な限りC++の機能を使用してコーディングすること（例：`<string.h>` より `<cstring>` を選択）。C関数の使用は許可されているが、可能な限りC++版を優先すること。

*原文: Try to always code using C++ features when available (for example, choose <cstring> over <string.h>). You are allowed to use C functions, but always prefer their C++ versions if possible.*

---

## 外部ライブラリ禁止

外部ライブラリおよびBoostライブラリの使用は禁止。

*原文: Any external library and Boost libraries are forbidden.*

---

## 技術用語

| 英語 | 日本語 | 説明 |
|------|--------|------|
| crash | クラッシュ | プログラムの異常終了 |
| relinking | 再リンク | 変更のないオブジェクトファイルの再リンク |
| C++ 98 standard | C++98標準 | 1998年に標準化されたC++の仕様 |
| Boost libraries | Boostライブラリ | C++の準標準ライブラリ集（本課題では使用禁止） |

---

## 重要な制約まとめ

| 制約 | 内容 |
|------|------|
| クラッシュ | 絶対禁止（0点） |
| C++標準 | **C++98のみ** |
| 外部ライブラリ | 禁止（Boost含む） |
| Makefile | `$(NAME)`, `all`, `clean`, `fclean`, `re` 必須 |
| コンパイルフラグ | `-Wall -Wextra -Werror` 必須 |

---

## C++98 で使えない機能（注意）

| 機能 | C++11以降 | C++98での代替 |
|------|----------|---------------|
| `auto` | ○ | 明示的な型宣言 |
| `nullptr` | ○ | `NULL` または `0` |
| Range-based for | ○ | イテレータ使用 |
| `std::unordered_map` | ○ | `std::map` |
| `std::thread` | ○ | 使用不可（fork禁止のため関係なし） |
| ラムダ式 | ○ | 関数オブジェクト |
| `std::to_string()` | ○ | `std::stringstream` |
