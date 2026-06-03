# REMINDER: onboarding §番号の更新

> **状態**: 完了（2026-06-01）  
> **決定日**: 2026-06-01 MTG  
> **更新者**: torinoue

---

## 背景

onboarding / reading_guide が旧 myIRCd `interface.md` の §番号を参照したまま。
2026-06-01 MTG でドキュメント体制を確定したが、§番号の一括修正は後回しにしていた。

## 前提（完了済み）

- [x] `ref_interface.md` → `b_implementation_reader.md` リネーム
- [x] ドキュメント体制確定 — **SSOT 2**: `class_overview_diagram.md`（公開 API）+ `interface.md`（契約憲章）。**非 SSOT**: `b_implementation_reader.md`（B 層主読者の実装読み物）
- [x] 各ファイルのヘッダ・相互参照を更新
- [x] `b_implementation_reader.md` 公開 API 表をミニ表化（メソッド・利用者・内容。脚注 [^fn-api-ref]）
- [x] onboarding / reading_guide の §番号・参照先を現行ドキュメント体制に合わせて更新

## 更新内容（2026-06-01）

| ファイル | 変更 |
|---------|------|
| `onboarding_A.md` | 公開 API → `class_overview` + `b_implementation_reader` §5 + `interface` §2 |
| `onboarding_B.md` | → `class_overview` + `b_implementation_reader` §6 + `interface` §3 |
| `onboarding_C1.md` | → `class_overview` + `interface` §4 + `b_implementation_reader` §7 |
| `onboarding_C2.md` | → `class_overview` + `interface` §5 + `b_implementation_reader` §8 |
| `reading_guide_A/B/C1/C2.md` | 同上方針で §番号・リンク更新 |
| `reading_guide_common.md` | 更新日・更新者追記 |

## 現行 `interface.md` の §構成（参考）

| § | 内容 |
|---|------|
| §1 | 境界オブジェクト（A↔B） |
| §2 | A層概要 |
| §3 | B層概要 |
| §4 | C1層インターフェース |
| §5 | C2層インターフェース |
| §6 | 全層共通ルール |
| §7 | 設計決定 |
| §8 | 関連ドキュメント |

## リネーム（2026-06-01 決定）

`b_implementation_guide.md` → **`b_implementation_reader.md`**（B 主読者の実装読み物。SSOT ではない）
