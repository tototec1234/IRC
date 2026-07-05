# graceful close の今回コミット見送り判断（設計メモ）

> 種別: 設計判断の記録（Decision Record）
> 対象: `src/a/Server.cpp` の切断経路（`_disconnectClient`）
> 関連: [connection_lifecycle_integration.md](connection_lifecycle_integration.md) §5.1（`connection_lifecycle_integration.md:147-160`）
> ステータス: 今回コミットでは **見送り（defer）**。本文末尾の「Issue 化用サマリ」を後日 GitHub Issue に転記する。

## 1. 背景

[connection_lifecycle_integration.md:153-157](connection_lifecycle_integration.md) に、将来 graceful close を入れる場合の足場として、A 層の切断関数を 2 段階に分ける案がある。

```cpp
void Server::_closeConnectionOnly(int fd);  // close + _removeFd + delete Connection
void Server::_disconnectClient(int fd);     // _closeConnectionOnly + _state.removeClient
```

これを今回のコミットで取り込むか検討した。結論として、**関数分割・graceful close 本体ともに今回は見送る**。`_disconnectClient(fd)` に集約したままとする（`connection_lifecycle_integration.md:160` の「集約してよい」に準拠）。

現状の切断関数（変更しない）:

```323:333:src/a/Server.cpp
void Server::_disconnectClient(int fd) {
	close(fd);
	_removeFd(fd);
	std::map<int, Connection*>::iterator it = _connections.find(fd);
	if (it != _connections.end()) {
		delete it->second;
		_connections.erase(it);
		_state.removeClient(fd);			// C層からも除去
	}
	std::cout << "client #" << fd << " gone away" << std::endl;  // bircd の gone away 相当
}
```

## 2. 判断

- 今回コミット: graceful close を**実装しない**。`_closeConnectionOnly` への**分割もしない**。
- 切断は従来通り `_disconnectClient(fd)` の一括処理（論理除去＋即 `close`）のまま。
- graceful close は別 Issue として切り出し、時間に余裕のあるタイミングで着手する。

## 3. 根拠

### 3.1 分割そのものは挙動を変えない＝今やる利得が小さい

doc 153-157 の分割は graceful close の実装ではなく、その**下準備（足場）**にすぎない。分割だけでは挙動は現状と同一になる。

加えて、分割時に1か所だけ挙動が変わる注意点がある。現状 `_state.removeClient(fd)` は `if (it != _connections.end())` の**内側**にあり「`_connections` に存在した時だけ C 層から消す」という条件付きである。分割すると map 管理が `_closeConnectionOnly` に移り、`_disconnectClient` 側の `_state.removeClient(fd)` は**無条件呼び出し**になる。

> 検証済み（2026-06-21）: `ServerState::removeClient(fd)`（`src/c/ServerState.cpp:76-95`）は冒頭で `getClientByFd(fd)` が NULL の場合に早期 return する。未登録 fd を渡しても no-op で安全であり、後続の channel 処理・`_client.removeClient(fd)` には到達しない。よって無条件呼び出し化しても挙動差は生じず、事前チェックの追加も不要。

→ 上記のとおり分割自体は安全に行えるが、利得（足場のみ）に対しレビューコストが見合わない。graceful 本体に着手するときに同時に入れる方が、関連変更をまとめてレビューできて安全。

### 3.2 graceful close 本体には、分割以外に状態機械が必要

graceful close の本質は「論理切断（即時）」と「物理切断（遅延）」の分離である。

- 論理切断（即時）: `_state.removeClient(fd)` を先に呼び、以後そのクライアントへ新規メッセージをルーティングしない。
- 物理切断（遅延）: 送信バッファ（QUIT 通知・`ERROR :Closing Link` 等）を**送り切ってから** `close` する。

これを実現するには分割に加えて、最低限以下が必要になる。

- closing 中の fd を保持する集合（例: `std::set<int> _closing`）。
- `_handleWrite` で送信バッファが空になった時点で実 `close` する経路（drain 検出）。
- closing 中は `POLLIN` を外し `POLLOUT` を残す監視切替（必要なら `shutdown(fd, SHUT_RD)`）。
- flush が完了しない（相手が受信しない）ケース用の closing タイムアウト。さもないと fd が滞留する。

→ 今回のコミット規模で安全に入れ切れる量ではない。`connection_lifecycle_integration.md:197` も現状の最小統合では「送信保証は限定的」と明記しており、ここを詰めるのは独立したスコープが妥当。

### 3.3 既存の切断呼び出し元4か所のうち、graceful が効くのは1系統だけ

`_disconnectClient` の呼び出し元と、graceful 化の意味の有無は以下。

| 行 | 状況 | graceful の意味 |
|---|---|---|
| `src/a/Server.cpp:178` | `_handleRead` が false（EOF / recv エラー / 行長すぎ / B層切断要求） | recv==0・エラーは相手が既に不在 → hard close でよい |
| `src/a/Server.cpp:186` | `_handleWrite` 失敗（send 失敗） | 送信経路が壊れている → flush 不可 → hard close |
| `src/a/Server.cpp:198` | `POLLERR / POLLHUP / POLLNVAL` | 相手切断済み → hard close |
| `src/a/Server.cpp:218` | timeout ループ（Ping timeout） | サーバ主導の整列切断 → 最後の1行を届けたいなら graceful 候補 |

graceful close が実益を持つのは「サーバ主導で、まだ生きている相手に最後のメッセージを確実に届けたい」ケースのみ。4 か所中 3 か所は相手かソケットが既に壊れており hard close が正しい。

加えて、各呼び出し元は `--i; continue;`（`src/a/Server.cpp:179` 等）で `_pollfds` 縮小に合わせ添字を戻している。graceful 化で「呼んでも即 erase しない（closing に入れて後で消す）」と、この `--i` の前提が崩れ、closing 中は `_removeFd` しない＝ `--i` 不要、という分岐整理も必要になる。

→ 効果が限定的（1系統）な一方で、ループの添字管理にまで影響が及ぶ。費用対効果が低い。

## 4. 今回やること / やらないこと

- やる: 現状の `_disconnectClient(fd)` 一括処理を維持。`connection_lifecycle_integration.md` の最小統合方針（§5）に沿う。
- やらない: `_closeConnectionOnly` への分割、closing 状態機械、drain 検出、`shutdown` 併用、closing タイムアウト。

## 5. 将来 graceful close に着手するときの TODO

- [ ] `_disconnectClient` を `_closeConnectionOnly` + `_state.removeClient` に分割（§3.1 の無条件呼び出し化の差分を確認）。
- [ ] `_closing` 集合と drain 検出（`_handleWrite` でバッファ空 → 実 close）を実装。
- [ ] closing 中の `POLLIN` 外し / `POLLOUT` 維持（必要なら `shutdown(fd, SHUT_RD)`）。
- [ ] closing タイムアウト（flush 不能な相手で fd を滞留させない）。
- [ ] timeout 系統（`src/a/Server.cpp:218`）のみ graceful 経路へ。hard close 系統（`178/186/198`）は据え置き。
- [ ] graceful 化に伴う `--i` / `_removeFd` の添字管理を見直し。
- [ ] 実 close 時点で `_healthMonitor.removeClient(fd)` を呼ぶ整合（`connection_lifecycle_integration.md:145` の fd 再利用安全性）。

## 6. Issue 化用サマリ（後日 GitHub Issue へ転記）

**タイトル案**: A層 切断経路に graceful close を導入する（送信 flush 後 close）

**概要**: 現状 `_disconnectClient(fd)` は論理除去と即 `close` を一括で行うため、送信バッファに積んだ最後のメッセージ（QUIT 通知・`ERROR :Closing Link` 等）が flush 前に切断され得る（`connection_lifecycle_integration.md:197` 参照）。

**今回見送る理由（要点）**:
1. doc 153-157 の関数分割は足場のみで挙動を変えず、単独で入れる利得が小さい（§3.1）。
2. graceful 本体は closing 状態機械・drain 検出・closing タイムアウトを要し、今回コミットの規模に収まらない（§3.2）。
3. graceful が実益を持つのは timeout 等のサーバ主導切断1系統のみ。残り3系統は hard close が正しく、`--i` 添字管理にも波及する（§3.3）。

**スコープ**: 上記「§5 TODO」。timeout 系統を graceful 経路に乗せ、hard close 系統は据え置く。
