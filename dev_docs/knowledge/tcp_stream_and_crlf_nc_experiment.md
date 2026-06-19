# TCP ストリームと IRC 行末（`\r\n`）— nc 実験ナレッジ

> **作成者**: torinoue  
> **検証状況**: macOS・**Ubuntu 22.04（42 環境）** で実証済み。

---

## 目的

Phase 3（`Connection` 受信バッファ）に入る前に、次をターミナルだけで体感する。

1. **TCP はストリーム** — 1 回の `recv` が 1 行と一致しない
2. **IRC の行末は `\r\n`（CRLF）** — `\n` だけとはバイト列が違う
3. **見た目では区別できない** — `hexdump` 等でバイトを見る

---

## `\n` / `\r` / `\r\n` の違い

| 記号 | 名前 | 16進 | バイト数 | 備考 |
|------|------|------|----------|------|
| `\n` | LF (Line Feed) | `0x0A` | 1 | Unix/macOS の Enter が送ることが多い |
| `\r` | CR (Carriage Return) | `0x0D` | 1 | 単体では IRC 行末にならない |
| `\r\n` | CRLF | `0x0D 0x0A` | 2 | **IRC が要求する行末** |

`hello\r\n` の hexdump 例（macOS / Ubuntu 共通）:

```
00000000  68 65 6c 6c 6f 0d 0a                              |hello..|
00000007
```

- `68`〜`6f` = `hello`
- `0d` = `\r`
- `0a` = `\n`
- 合計 7 バイト

`hello\n` だけなら末尾は `0a` のみ（6 バイト、`0d` なし）。

---

## 実験 A: TCP はストリーム（1 recv ≠ 1 行）

**狙い:** TCP にはメッセージ境界がない、という概念を `nc` で触る。**ただし localhost では表示タイミングの差はほぼ出ない**（下記「観察」参照）。

### 手順

| 役割 | macOS | Ubuntu |
|------|-------|--------|
| ターミナル 1（待ち受け） | `nc -l 9999` | `nc -l 9999` |
| ターミナル 2（接続） | `nc localhost 9999` | `nc localhost 9999` |
| 送信 | ① `h` → Enter → … とゆっくり ② 長文（例: 200 文字）を一括 | 同左 |

### 観察

- 教科書的には、ターミナル 1 は **1 文字ごと**に出ることもあれば、**まとめて**出ることもある、と説明される
- **macOS / Ubuntu 22.04 / localhost 実測（torinoue）:** ① ゆっくり 1 文字送信でも ② 200 文字一括送信でも、ターミナル 1 には **どちらも即時に全文** が表示された（OS 差なし）
- Enter を押すたびに「1 行」として届く保証はない — ただし **`nc` + localhost では体感しにくい**

### なぜ localhost では分割が見えないか

- **loopback は遅延ほぼゼロ** — 送ったデータがすぐカーネルバッファに入り、`nc` がまとめて読んで表示する
- **`nc` はアプリの `recv` を隠す** — 受信バイトをそのまま stdout に出すだけなので、「何バイトずつ `recv` したか」は見えない
- 実験 A は **概念の導入** であり、**macOS / Ubuntu とも** localhost の `nc` だけでは「分割到着」は再現できない

### 結論

TCP にはメッセージ境界がない。アプリの `recv` が返すバイト数は送信回数・タイミングと一致しない。**IRC サーバ（Phase 3）では `\r\n` 区切りの 1 行が揃うまで `_recvBuffer` に溜め、揃ったら取り出す**必要がある。実験 B の `hel` → `lo\r\n` 分割到着の例は、この前提をコード側で扱う理由を示す。

---

## 実験 B: `\r\n` と `\n` のバイト差

**狙い:** 画面では同じに見える行末を、バイト列で区別する。

### 手順（共通の流れ）

1. ターミナル 1 で待ち受け + ダンプ
2. ターミナル 2 で `printf` 送信（**`-w 1` で接続を閉じる**）
3. ターミナル 1 の出力を比較
4. 次のテスト前にターミナル 1 の `nc` を **再起動**（`nc -l` は 1 接続で終わる）

### ターミナル 1: 受信バイトを表示

| 方法 | macOS（実測） | Ubuntu（実測） |
|------|---------------|----------------|
| おすすめ | `nc -l 9999 \| hexdump -C` | `nc -l 9999 \| hexdump -C` |
| 代替 | `nc -l 9999 \| cat -v`（`\r` は `^M`） | `nc -l 9999 \| cat -v`（`\r` は `^M`） |

### ターミナル 2: 送信

| テスト | コマンド | 期待する末尾（hex） |
|--------|----------|---------------------|
| IRC 正しい行末 | `printf 'hello\r\n' \| nc -w 1 localhost 9999` | `... 6f 0d 0a`（7 バイト） |
| LF のみ | `printf 'hello\n' \| nc -w 1 localhost 9999` | `... 6f 0a`（6 バイト） |

Ubuntu 実測（`hello\r\n`、ターミナル 1 = `nc -l 9999 | hexdump -C`）:

```
00000000  68 65 6c 6c 6f 0d 0a                              |hello..|
00000007
```

Ubuntu 実測（`hello\n` のみ）:

```
00000000  68 65 6c 6c 6f 0a                                 |hello.|
00000006
```

---

## macOS / Ubuntu コマンド差分

| 項目 | macOS | Ubuntu（実測: 22.04） |
|------|-------|------------------------|
| `nc` パッケージ | BSD netcat（OS 同梱） | `netcat-openbsd` 1.218-4ubuntu1 |
| `nc -l ポート` | 使える | 使える |
| `nc -w 1`（アイドル後に切断） | 使える | 使える |
| `hexdump -C` | 使える | 使える |
| `od -c -w1` | **使えない**（BSD `od`） | **使える**（GNU coreutils） |
| `nc -l \| od` + クライアント `-w 1` | BSD `od` はブロックしやすい → `hexdump` 推奨 | GNU `od -An -tx1 -c -w1` で表示できる（`-w 1` 必須） |
| ターミナル Enter | 多くは `\n` のみ送信 | 多くは `\n` のみ送信 |

**`od` 単体テスト（Ubuntu）:** `printf 'hello\r\n' | od -An -tx1 -c -w1` で 1 バイトずつ hex + 文字表示ができる。実験 B のリスナーには `hexdump -C` を推奨（Mac と手順を揃えやすい）。

---

## FAQ

### Q: nc 同士では通信できないのか？

**できる。** 待ち受け側が起動している状態で、別ターミナルから `nc localhost 9999` すれば接続できる。

「つながらない」ように見える典型原因:

1. **リスナーがまだ起動していない**（先に `nc -l 9999`）
2. **`nc -l` は 1 クライアントで終了** — 2 回目のテスト前に再起動が必要
3. **パイプ + `od` の組み合わせで両方が待ち** — 下記参照

### Q: `nc -l 9999 | od -c` で何も出ず固まる

**通信失敗ではなく、読み側がブロックしている。**

- `printf 'hello\r\n' | nc localhost 9999` だけだと、送信後も TCP 接続が開いたまま残ることがある
- BSD `od`（macOS）はデフォルトで 16 バイト単位読み。7 バイトだけではブロックして表示しない
- クライアント側に **`-w 1`** を付けてアイドル後に切断する

```bash
# クライアント側
printf 'hello\r\n' | nc -w 1 localhost 9999

# リスナー側（Mac では hexdump が楽）
nc -l 9999 | hexdump -C

# リスナー側（Ubuntu では GNU od も可。クライアントは必ず -w 1）
nc -l 9999 | od -An -tx1 -c -w1
```

### Q: Mac の Enter と `printf '\r\n'` は同じか？

**違う。** Mac / Ubuntu のターミナルで Enter だけ押すと、多くの場合 `\n`（`0x0A`）のみ。IRC クライアントは `\r\n` を送る。Phase 3 の動作確認では `printf 'hello\r\n'` を使う。

### Q: なぜサーバは `\n` だけでは行を切~~らないのか？~~ 切る！

IRC 仕様上、メッセージ境界は **CRLF**（[RFC 2812](https://www.rfc-editor.org/rfc/rfc2812)）。`\n` だけを境界にすると、本物の IRC クライアントと挙動がずれる。実装では `find("\r\n")` を使う。


---

## Phase 3 実装への接続

受信データの流れ（概念）:

```mermaid
flowchart LR
    subgraph socket [ソケット]
        TCP[TCP ストリーム<br/>分割・結合あり]
    end
    subgraph conn [Connection]
        RB["_recvBuffer<br/>append し続ける"]
        HCL{hasCompleteLine<br/>"\r\n" あり?}
        PL[popLine<br/>最初の CRLF まで取り出し]
    end
    subgraph server [Server Phase 3]
        OUT["stdout: [recv] ..."]
    end
    TCP -->|recv| RB
    RB --> HCL
    HCL -->|yes| PL
    PL --> OUT
    HCL -->|no| RB
```

| 受信内容 | `hasCompleteLine()` | Phase 3 での挙動 |
|----------|---------------------|------------------|
| `hello\r\n` | true | `[recv] hello` が出る |
| `hello\n` | false | バッファに溜まったまま（行として出ない） |
| `hel` → `lo\r\n`（分割到着） | 2 回目の recv 後に true | バッファが `\r\n` まで揃ってから 1 行 |

関連: [A 層実装計画（Phase 3）](../a_devdoc/a_implementation_plan.md) §2 Phase 3 / §4.2 `Connection`

---

## 実験チェックリスト

読者が自分で試したらチェックする。

### 実験 A（TCP ストリーム）

- [ ] ターミナル 1 で `nc -l 9999` を起動した
- [ ] ターミナル 2 から `nc localhost 9999` で接続できた
- [ ] ① ゆっくり送信と ② 一括送信を試した（localhost ではどちらも即時全文表示になりがち。分割到着の体感は期待しない）

### 実験 B（行末のバイト差）

- [ ] リスナー側で `hexdump -C` を使った
- [ ] `printf 'hello\r\n' | nc -w 1 localhost 9999` の末尾に `0d 0a` があることを確認した
- [ ] `printf 'hello\n' | nc -w 1 localhost 9999` では `0d` がなく `0a` のみであることを確認した
- [ ] 2 回目のテスト前にリスナーを再起動した

### OS 別（torinoue 実証済み）

- [x] macOS / Ubuntu で実験 A を実施した（localhost では 1 文字送信・一括送信とも即時全文表示。OS 差なし）
- [x] Ubuntu で実験 B を再現した（`hexdump -C` で CRLF / LF 差分確認）
- [x] 本ドキュメントの Ubuntu 列を実測値に更新した

---

## 改定履歴

| 日付 | 内容 |
|------|------|
| 2026-06-11 | 初版（macOS 実測。Ubuntu は想定値） |
| 2026-06-11 | Ubuntu 22.04（42 環境）で実験 B・コマンド差分を実測更新。実験 B 表から `od` 行を OS 差分節へ移動 |
| 2026-06-11 | 実験 A: localhost `nc` では分割到着が見えない旨を追記（macOS / Ubuntu とも 1 文字／200 文字即時全文表示） |
