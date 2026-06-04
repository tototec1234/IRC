# ChannelModes +l limit の扱い

## 背景

`MODE #channel +l <limit>` は channel の人数制限を設定する。

C層の `ChannelModes` は channel mode の状態を保持するが、IRC コマンド文字列の解析や numeric reply の生成は B層の責務である。

そのため、`+l` の入力値検証は B層と C層で責務を分ける。

## 決定事項

`ChannelModes` は以下の値だけを有効な limit として保持する。

- `limit >= 1`: 人数制限あり
- `limit == -1`: 制限解除

`0` は「0人しか入れない channel」を意味し、実用上存在しないため無効とする。

`-2` 以下の負数も無効とする。

## 最大値

C層はサーバの能力や運用上の上限を知らない。

そのため、最大値は C層では決めない。B層または呼び出し側が、パース結果やサーバ方針に基づいて上限を管理する。

## B層の責務

B層の `CommandDispatcher` は、`MODE +l` の引数を C層に渡す前に検証する。

例:

- `/MODE #channel +l 0`
- `/MODE #channel +l abc`
- `/MODE #channel +l -2`

これらが無効な入力であるかどうかは、文字列を数値に変換する段階で B層が判定する。

B層は IRC プロトコル上の失敗理由を判断し、適切な numeric reply を返す。

## C層の責務

C層の `ChannelModes::setLimit(int limit)` は、呼び出し側が検証済みの値を渡すことを前提にする。

ただし、B層の実装ミスや予期しない呼び出しで不正な値が渡された場合に状態を壊さないため、最終防衛ラインとして以下の guard を置く。

```cpp
void ChannelModes::setLimit(int limit) {
  if (limit >= 1 || limit == -1) {
    _limit = limit;
  }
}
```

不正値は no-op とし、既存の limit を維持する。

これは C層が IRC プロトコル判定を肩代わりするためではなく、C層の状態を壊さないための保険である。

## まとめ

- 入力文字列の解析とエラー判定は B層
- `ChannelModes` は `int` として渡された limit 状態を保持する
- C層は `>= 1` または `-1` のみ受け入れる
- 最大値は C層では決めない
- 不正値が C層に届いた場合は no-op とする
