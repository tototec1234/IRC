# レビュー指摘と対応記録

外部プレビュー・チーム peer review・Issue 化前の設計メモを、**指摘単位**で整理するディレクトリ。

## 目的

- 指摘内容・再現手順・対応方針・PR/Issue リンクを一箇所に残す
- チーム合意前の説明資料（Slack / 口頭レビュー / GitHub Issue 下書き）として使う
- 本番提出用 README 等へ転載する運用マニュアル（ポート占有時の対処など）の下書き置き場

## ディレクトリ命名

```
review_responses/
  YYYY-MM-DD_<レビュー源>_preview/   # 例: 2026-07-05_samatsum_preview
    README.md                        # 当該セッションの指摘一覧（索引）
    <topic>.md                       # 指摘1件 = 1ファイル（例: fix_bind_port_reuse.md）
```

## 1ファイルの想定構成

各 `<topic>.md` は次を含める（[issue_graceful_close.md](../a_devdoc/issue_graceful_close.md) と同系統）。

1. 背景・指摘原文（要約）
2. 現象・再現手順
3. 原因
4. 対応内容（変更ファイルと diff の要点）
5. 検証手順・期待結果
6. 運用マニュアル（README 転載用があれば）
7. Issue 化用サマリ / PR 説明用サマリ

## 外部レビューアの扱い

42 課題の制約上、チームメンバー以外は GitHub リポジトリに **read-only** で招待する。  
co-author や write 権限は付与しない。本ディレクトリでは **プレビュー依頼先として名前を記録**するのみとする。

## 索引

| 日付 | セッション | 指摘数 | 索引 |
|------|-----------|--------|------|
| 2026-07-05 | samatsum 氏プレビュー（同席なし） | 1（他は追記予定） | [2026-07-05_samatsum_preview/README.md](2026-07-05_samatsum_preview/README.md) |
