# client_read フローチャート

> 作成日: 2026-06-12
> 対象: [`bircd/client_read.c`](./client_read.c)
> 関連資料: [`bircd_data_flow_diagram.md`](./bircd_data_flow_diagram.md)

```mermaid
flowchart TD
    A["client_read(t_env *e, int cs)"] --> B["recv(cs, e->fds[cs].buf_read, BUF_SIZE, 0)"]
    B --> C{"r <= 0?"}

    C -->|Yes| D["close(cs)"]
    D --> E["clean_fd(&e->fds[cs])"]
    E --> F["printf('client #... gone away')"]
    F --> Z["Return"]

    C -->|No| G["Set i = 0"]
    G --> H{"i < e->maxfd?"}

    H -->|No| Z
    H -->|Yes| I{"fds[i].type == FD_CLIENT and i != cs?"}

    I -->|Yes| J["send(i, e->fds[cs].buf_read, r, 0)"]
    I -->|No| K["Skip send"]

    J --> L["i++"]
    K --> L
    L --> H
```

## 補足

- `r <= 0` は「切断（r == 0）」と「エラー（r < 0）」の両方を同じ経路で処理する
- broadcast ループは自分自身（`i == cs`）と非クライアント FD をスキップする
- `recv` → 即 `send` の直結。`buf_write` / `client_write` 経路は未使用
