# ServerState::updateNick と ClientRegistry::updateNick の関係

## 背景

nick 変更では、`Client` 本体の nick cache と nick lookup 用の辞書を同時に更新する必要がある。

直接 `Client::_unsafe_setNick()` を呼ぶと、`Client` 本体だけが更新され、nick 辞書との整合性が壊れる。

そのため、B層は必ず `ServerState::updateNick()` を呼ぶ。

## 決定事項

`ServerState::updateNick()` と `ClientRegistry::updateNick()` は、同じ操作を別レイヤーで表す。

- `ServerState::updateNick()` は B層向けの Facade API
- `ClientRegistry::updateNick()` は `ServerState` から委譲される内部実装
- B層は `ClientRegistry` を直接触らない
- `Client::_unsafe_setNick()` は最終的な cache 更新だけを行う低レベル API

## Facade としての ServerState

B層にとって `updateNick()` は、`ServerState` が提供する C層の公開 API である。

`ClientRegistry` は `ServerState` の内部実装詳細であり、B層はその存在を知る必要がない。

呼び出し側は `ServerState` を C層の窓口として見ればよく、nick map の実管理が `ClientRegistry` に分離されていることを意識しなくてよい。

## なぜ同名のままにするか

`ClientRegistry::registerNick()` のように内部実装側だけ別名にすると、`ServerState::updateNick()` と `ClientRegistry::registerNick()` が別の操作なのか、同じ操作なのかが分かりづらくなる。

Facade と委譲先で同じ操作名を保つことで、以下の構造が読み取りやすくなる。

```cpp
bool ServerState::updateNick(Client& client, const std::string& newNick) {
  return _client.updateNick(client, newNick);
}
```

これは「C層の公開窓口が、内部 registry に同じ操作を委譲している」ことを示す。

## まとめ

- B層は `ServerState::updateNick()` だけを呼ぶ
- `ClientRegistry::updateNick()` は `ServerState` 内部の委譲先
- `Client::_unsafe_setNick()` は直接呼ばない
- 同名委譲は意図的な Facade / delegation の表現である
