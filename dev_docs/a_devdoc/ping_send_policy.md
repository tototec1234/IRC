# Issue: ConnectionHealthMonitor の PING 能動送信方式の決定（A案 vs B案）

> branch: `fix/A-MacOS-to-linux`
> commit: `741a673` `feat(a): ConnectionHealthMonitor統合。ただし PING / QUIT / graceful close は未実装`
> 関連ファイル:
> - `src/a/Server.cpp` / `include/a/Server.hpp`
> - `src/lifecycle/ConnectionHealthMonitor.cpp` / `include/lifecycle/ConnectionHealthMonitor.hpp`
> - `dev_docs/a_devdoc/connection_lifecycle_integration.md`

---

## 1. 背景

commit `741a673` で PING/PONG 生存確認の **配線** は完了した。

- `_handleRead` で `updateActivity(fd)`
- `dispatch(fd, msg, _state, _healthMonitor)` で B 層へ monitor を渡す
- timeout 切断経路（`collectTimedOutClients` → `DisconnectNotifier::build` → `applyCommandResult` → `_disconnectClient` → `removeClient`）を接続

ただし **PING の能動送信（`generatePing`）は未実装**。`waitingForPong` を立てる経路が
`generatePing()` の 1 箇所だけなので、これを呼ばないとタイムアウト切断ブロックは
**一度も発火しない（現状デッドコード、配線のみ）**。

したがって「いつ・どの fd に PING を送るか」という送信方式を決める必要がある。

## 2. 決めたいこと

PING 能動送信の方式を **A案 / B案** のどちらにするか。本 Issue では:

1. まず **B案** を実装する（実害を校舎の実機で体感する目的）。
2. B/lifecycle 担当と **校舎の実機テスト** を行う。
3. **合意の上で A案へ移行** する。

## 3. 送信方式の案

- **A案（アイドル時のみ）**: ライフサイクル側で「最後の受信からの経過時間」を見て、
  **沈黙している接続だけ** に PING を送る。
- **B案（一斉送信）**: A 層がサーバーとして **全クライアントへ一定間隔で一斉送信** する。
  born2beroot の `cron` + `wall` に「返事がないと殺す」を足したイメージ。

## 4. RFC 1459 §4.6.2 からの根拠（一次資料）

出典: RFC 1459, Section 4.6.2 "Ping message"（p.37）
<https://www.rfc-editor.org/rfc/rfc1459.txt>

該当文（原文ママ、`if no other activity detected` は原文の表記）:

> The PING message is used to test the presence of an active client at
> the other end of the connection.  A PING message is sent at regular
> intervals if no other activity detected coming from a connection.  If
> a connection fails to respond to a PING command within a set amount
> of time, that connection is closed.

同節の補足（サーバ側の応答方針）:

> Any client which receives a PING message must respond to <server1>
> (server which sent the PING message out) as quickly as possible with
> an appropriate PONG message to indicate it is still there and alive.
> Servers should not respond to PING commands but rely on PINGs from
> the other end of the connection to indicate the connection is alive.

### 解釈（A案/B案への当てはめ）

この一文は 2 つの要素に分解できる。

1. **「殺す」意味論**（後半）: 「一定時間内に PONG が返らなければ close」。
   → **A案・B案ともに準拠**。差は出ない。
2. **「送る」トリガ**（前半）: 「他に活動が検出されない場合に、一定間隔で送る」。
   → 記述としてはアイドル時送信。**A案がそのまま一致**。
   B案は活動が検出されていても送るため、この文が描く挙動より広い。

ただし重要な留保:

- この文は **MUST / MUST NOT の強制句ではなく descriptive（記述的）**。
- RFC 1459 自体が **Experimental Protocol**（冒頭 Status of This Memo）。
- 「活動中は送ってはならない」とは **明記していない**。

したがって公平な評価は次のとおり:

| 観点 | 評価 |
|---|---|
| kill 意味論（無応答で close） | A案・B案ともに準拠 |
| 送信トリガの素直な読み | アイドルベース = **A案** |
| B案は §4.6.2 違反か | **違反とは言えない**（強制句がないため） |

結論: **RFC は A案を強制しない**。B案も kill 意味論では準拠する。ただし
**§4.6.2 の送信条件の素直な読みは A案** である。

## 5. B案の実害

### 実害1: アクティブ接続への無駄 PING

`updateActivity` で生存が分かっている接続にも PING を投げる。チャットが活発なほど
無駄な PING/PONG 往復が増える（born2beroot の wall を全 TTY に出すのと同じ無駄）。

### 実害2: 無条件 re-ping でタイムアウトが永久に発火しない（見落としやすい）

`generatePing()` は呼ぶたびに `lastPingSentAt = now` を上書きする
（`src/lifecycle/ConnectionHealthMonitor.cpp`）。一方タイムアウト判定は
`now - lastPingSentAt > timeoutSeconds`。タイムアウト走査は poll ループ毎回（約1秒）回る。

これを踏まえた挙動を 4 象限で整理する。

| 方式 | `PING_INTERVAL < timeoutSeconds` | `PING_INTERVAL > timeoutSeconds` |
|---|---|---|
| **無条件 re-ping** | `lastPingSentAt` が interval ごとに更新 → `now - lastPingSentAt ≤ interval < timeout` → **常に偽 → 誰も死なない（実害2）** | 次の掃引(t0+interval)より先に t0+timeout で走査が発火 → **正しく死ぬ** |
| **ガード有り（待機中スキップ）** | t0 で 1 回だけ ping → `lastPingSentAt=t0` 固定 → 後続掃引はスキップ → t0+timeout で **正しく死ぬ** | t0 で ping → 次掃引より先に t0+timeout で死んで `removeClient` 済み → **ガードは発動しない（無意味）** |

要点:

- 現状 `_healthMonitor(30)`（テストのため　timeout 30 秒に設定している）。例えば 10 秒間隔で無条件一斉 ping すると
  `lastPingSentAt` が 10 秒ごとに更新され、`now - lastPingSentAt` が 30 を超えられず、
  **切断機能が死ぬ**。
- 回避策は `isWaitingForPong(fd)` で **待機中の fd をスキップしてから** `generatePing` する
  （= 「ガード有り」）。`lastPingSentAt` を固定するため timeout が正しく発火する。
- **ガードが意味をなすのは `PING_INTERVAL < timeoutSeconds` のとき**（表の左列）。
  `>` の場合は次掃引前に走査が殺すため、ガードの有無は結果に影響しない。

## 6. A案の API 提案（移行時の参考スケッチ）

A案では「沈黙している接続だけ」を選別する API を `ConnectionHealthMonitor` に追加する。

```cpp
// 既存 collectTimedOutClients と同じ二段構え（now 注入でテスト可能に）
std::vector<int> collectClientsNeedingPing(std::time_t idleSeconds) const;
std::vector<int> collectClientsNeedingPing(std::time_t idleSeconds,
                                           std::time_t now) const;
```

中身の骨子:

```cpp
std::vector<int> ConnectionHealthMonitor::collectClientsNeedingPing(
    std::time_t idleSeconds, std::time_t now) const {
  std::vector<int> needing;
  for (HealthMap::const_iterator it = _clients.begin();
       it != _clients.end(); ++it) {
    const HealthState& s = it->second;
    if (s.waitingForPong)                                   // ① 待機中スキップ（実害2回避）
      continue;
    if (std::difftime(now, s.lastActivity) >= idleSeconds)  // ② 沈黙してる接続だけ（実害1回避）
      needing.push_back(it->first);
  }
  return needing;
}
```

A 層側はこれを回すだけ:

```cpp
std::vector<int> needing = _healthMonitor.collectClientsNeedingPing(idleSeconds);
for (/* needing の各 fd */)
    applyCommandResult(_healthMonitor.generatePing(fd));
```

- ① の `waitingForPong` スキップが実害2のガード。
- ② の `lastActivity` アイドル判定が実害1の回避。
- この API 1 本で B案の実害1・実害2 を構造的に両方消せる。これが A案が
  §4.6.2 の「if no other activity detected」を素直に実装できる理由。

## 7. 進め方

1. **B案を実装**（ループ末尾で掃引時刻 `_lastPingSweep` を持ち、一定間隔で全 client fd へ `generatePing`）。
2. B案の re-ping 方針を決める（無条件 / `isWaitingForPong` ガード）。
3. B/lifecycle 担当と **校舎の実機の実機テスト**。
4. A案 vs B案の **合意を記録**（本 Issue にコメント）。
5. 合意後 **A案（§6 の API）へ移行**。

## 8. 作業内容

- [ ] B案実装: 掃引時刻 `_lastPingSweep` を保持し、一定間隔で全 client fd へ `generatePing` → `applyCommandResult`
- [ ] B案の re-ping 方針（無条件 / `isWaitingForPong` ガード）を決定
- [ ] B/lifecycle 担当と校舎の実機テスト
- [ ] A案 vs B案の合意を本 Issue に記録
- [ ] A案（`collectClientsNeedingPing`）へ移行

## 9. Test plan（校舎の実機テストの受け入れ条件）

- [ ] クライアントが PING に対して PONG を返す（`isWaitingForPong` が解除される）
- [ ] PONG 未応答でタイムアウト切断が発火する（実害2を踏むか踏まないかを確認）
- [ ] アクティブ接続にも無駄 PING が出る挙動を観測する（実害1）
- [ ] irssi 等の実 IRC クライアントが自動 PONG を返すことを確認

## 10. 関連

- commit: `741a673` `feat(a): ConnectionHealthMonitor統合`
- `dev_docs/a_devdoc/connection_lifecycle_integration.md`（A 層統合ガイド）
- RFC 1459 §4.6.2 / §4.6.3: <https://www.rfc-editor.org/rfc/rfc1459.txt>
