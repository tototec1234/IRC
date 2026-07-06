*This project has been created as part of the 42 curriculum by torinoue, tvaroux, tyamaoka.*

# ft_irc

## Description

ft_irc is an IRC server written in C++98 for the 42 curriculum. The goal of the project is to implement the core behavior of an IRC server: accepting multiple clients, registering users, joining channels, exchanging messages, and managing channel operators and modes.

The server uses non-blocking sockets and `poll()` for I/O multiplexing, so it can handle several client connections in one process without creating one thread per client. Incoming TCP data is buffered until complete IRC lines ending in `\r\n` are available, then those lines are parsed and dispatched as IRC commands.

The implementation is organized into three main layers:

| Layer | Main classes | Responsibility |
| --- | --- | --- |
| Network / I/O | `Server`, `Connection` | sockets, `poll()`, `recv` / `send`, buffers, connection lifecycle |
| Protocol / Command | `Parser`, `CommandDispatcher`, `ReplyBuilder` | IRC message parsing, command handling, numeric replies |
| Application State | `ServerState`, `Client`, `Channel`, `ChannelModes` | clients, channels, memberships, operators, channel modes |

Data generally flows as follows:

```text
recv -> Parser::parse -> CommandDispatcher::dispatch
  -> ServerState read/update -> CommandResult -> send buffer
```

The mandatory IRC features implemented by the server include:

| Command | Status | Notes |
| --- | --- | --- |
| `PASS`, `NICK`, `USER` | Implemented | registration flow and welcome reply |
| `JOIN`, `PART` | Implemented | channel membership |
| `PRIVMSG`, `NOTICE` | Implemented | client and channel messages |
| `QUIT` | Implemented | disconnect handling and channel notification |
| `KICK`, `INVITE`, `TOPIC` | Implemented | channel operator commands |
| `MODE` | Implemented | channel modes `i`, `t`, `k`, `o`, `l` |
| `PING`, `PONG` | Implemented | keepalive-related protocol handling |

## Instructions

### Requirements

- A C++ compiler compatible with C++98
- `make`
- An IRC client such as `irssi`, or `nc` for manual protocol tests

The project has been tested on macOS and Linux.

### Compilation

Build the submission-style server without debug output:

```bash
make release
```

Build the development version with debug logging enabled:

```bash
make debug
```

Run the unit tests for the protocol and application-state layers:

```bash
make test
```

Clean generated files:

```bash
make clean
make fclean
```

### Execution

Start the IRC server with a TCP port and connection password:

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 mypassword
```

### Connecting With irssi

```bash
irssi -c 127.0.0.1 -p 6667 -w mypassword
```

Example commands inside `irssi`:

```text
/nick alice
/join #general
/msg #general Hello!
/quit
```

### Connecting With nc

Use `nc` when you want to type raw IRC protocol lines or test partial TCP sends:

```bash
nc -C 127.0.0.1 6667
```

Example session:

```text
PASS mypassword
NICK alice
USER alice 0 * :Alice
JOIN #general
PRIVMSG #general :Hello!
QUIT
```

You can also pipe commands directly:

```bash
printf 'PASS mypassword\r\nNICK alice\r\nUSER a a a :Alice\r\nJOIN #general\r\n' | nc -C 127.0.0.1 6667
```

### Bonus Bot

The bonus bot is built separately and runs as a normal IRC client. This keeps bot behavior isolated from the server process.

```bash
make -C bonus
./bonus/ircbot <host> <port> <password> <channel> [nickname]
```

Example:

```bash
./bonus/ircbot 127.0.0.1 6667 mypassword '#general' bonusbot
```

### File Transfer

File transfer can be tested with an IRC client that supports DCC:

```text
/dcc send <nick_to_send> <file>
/dcc get <nick_from>
```

## Technical Choices

- `poll()` is used for multiplexing listening and client file descriptors.
- Listening and client sockets are configured as non-blocking with `fcntl(O_NONBLOCK)`.
- `Connection` owns receive and send buffers, so TCP stream chunks can be reassembled into IRC lines.
- IRC protocol handling is kept out of the low-level socket code as much as possible. Command handlers return `CommandResult` objects that describe replies, broadcasts, and disconnect requests.
- A small lifecycle boundary handles disconnect and timeout-related notifications, including `QUIT` messages for channel members.

## Design Documentation

The main design notes are available in `dev_docs/`:

- [dev_docs/interface.md](dev_docs/interface.md) - layer contracts, `CommandResult`, and boundary rules
- [dev_docs/diagrams/class_overview_diagram.md](dev_docs/diagrams/class_overview_diagram.md) - class overview
- [dev_docs/diagrams/data_flow_diagram.md](dev_docs/diagrams/data_flow_diagram.md) - receive / parse / update / send flow
- [dev_docs/a_devdoc/connection_lifecycle_integration.md](dev_docs/a_devdoc/connection_lifecycle_integration.md) - disconnect and `QUIT` notification flow
- [dev_docs/knowledge/tcp_stream_and_crlf_nc_experiment.md](dev_docs/knowledge/tcp_stream_and_crlf_nc_experiment.md) - TCP stream and CRLF buffering notes
- [dev_docs/knowledge/invite_ticket_policy.md](dev_docs/knowledge/invite_ticket_policy.md) - `INVITE` notification and invite-list responsibility
- [dev_docs/knowledge/facade_delegation_update_nick.md](dev_docs/knowledge/facade_delegation_update_nick.md) - nickname update facade design
- [dev_docs/knowledge/channel_limit_policy.md](dev_docs/knowledge/channel_limit_policy.md) - `MODE +l` validation and state policy

## Resources

### IRC Protocol

- [RFC 1459 - Internet Relay Chat Protocol](https://datatracker.ietf.org/doc/html/rfc1459)
- [RFC 2812 - Internet Relay Chat: Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812)
- [Modern IRC Client Protocol documentation](https://modern.ircdocs.horse/)

### Socket Programming

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- `poll(2)`, `socket(2)`, `bind(2)`, `listen(2)`, `accept(2)`, `recv(2)`, `send(2)`, and `fcntl(2)` manual pages

### AI Usage

AI tools were used as development support, with human review and final responsibility kept by the project members. In particular, AI was used for:

- drafting and translating documentation, including parts of this README and internal design notes;
- organizing the three-layer architecture explanation and onboarding material;
- discussing edge cases in IRC command behavior, connection lifecycle, buffering, and non-blocking I/O;
- generating review checklists, test ideas, and summaries of implementation decisions;
- helping compare implementation choices against RFC behavior and 42 project constraints.

AI was not treated as an authority for the final implementation. Protocol behavior, design decisions, code changes, and documentation were reviewed and adjusted by the team.
