#include "a/Connection.hpp"
#include <string>
#include <sys/socket.h>   // recv
#include <cerrno>         // errno

Connection::Connection(int fd) : _fd(fd) {}
Connection::~Connection() {}
int Connection::getFd() const { return _fd; }

// ─────────────────────────────────────────────────────────────
// bircd(lesson3.5): client_read.c の read_and_store() に対応。
//   r = recv(cs, tmp, BUF_SIZE - buf_read_len, 0);
//   if (r <= 0) return r;            ← 0/負はそのまま返す
//   ft_memcpy(buf_read + buf_read_len, tmp, r);  ← 累積
// A層での違い:
//   - 累積先を可変長 std::string(_recvBuffer) にした
//     → bircd の buf_read_len 管理は不要（std::string が長さを持つ）
//     → bircd の「BUF_SIZE 満杯で return -1（行長すぎ切断）」も自然には不要
//       ※ ただし無制限増加は DoS。上限を設けるかは後の設計判断(TODO)
//   - recv は poll 駆動で1イベント1回（client_read.c L30-32 のメモ通り）
//   - bircd は client_read 内で close+clean_fd するが、A層は bool 返却で
//     呼び出し側(Server, Phase6)に委ねる（recv と切断処理の責務分離）
// ─────────────────────────────────────────────────────────────
bool Connection::readFromSocket() {
    char buf[BUF_SIZE + 1]; // マクロはcppライクでない、、、
    ssize_t n = recv(_fd, buf, sizeof(buf), 0);
    if (n == 0)
        return false; /*FIN。bircd の r==0 → gone away 相当） */
    if (n < 0)
        return false; /* エラー。bircd の r<0 も切断扱い
                  Phase7 で EAGAIN を「切断せずスキップ=true」に分ける
                     (client_read.c L94 のメモ) */
    _recvBuffer.append(buf, n);
    return true;
}

// ─────────────────────────────────────────────────────────────
// bircd(lesson3.5): client_read.c の get_crlf_pos() に対応。
//   bircd は len 走査で str[i]=='\r' && str[i+1]=='\n' を探し、
//   見つかれば i+2（消費長）、無ければ 0 を返す。
//   len 走査なのは '\0' 混入耐性のため。
// A層: std::string::find は \0 を含んでもバイナリ安全なので find で代替可。
//   RFC2812 2.3: https://datatracker.ietf.org/doc/html/rfc2812#section-2.3
// ─────────────────────────────────────────────────────────────
bool Connection::hasCompleteLine() const {
    return _recvBuffer.find("\r\n") != std::string::npos ;
}

// ─────────────────────────────────────────────────────────────
// bircd(lesson3.5): client_read.c の extract_and_consume() に対応。
//   buf_read[crlf_end-2] = '\0';            ← CRLF 手前で行を確定
//   broadcast_message(e, cs, buf_read);     ← bircd はここで送信まで実行
//   ft_memmove(buf_read, buf_read+crlf_end, ...);  ← 消費分を前詰め
// A層での違い:
//   - popLine は「1行を取り出して返すだけ」。送信(broadcast/dispatch)は
//     呼び出し側(_handleRead→B層)の責務。bircd は extract 内で broadcast まで
//     やっており責務が混ざっている。A層は分離する。
//   - 前詰め memmove は std::string::erase で代替。
// ─────────────────────────────────────────────────────────────
std::string Connection::popLine() {
	size_t pos = _recvBuffer.find("\r\n");          // bircd: get_crlf_pos
	std::string line = _recvBuffer.substr(0, pos)  ;      // bircd: buf_read[crlf_end-2]='\0'
	_recvBuffer.erase(0, pos + 2)  ;          // bircd: ft_memmove で前詰め
	return line;
    // ※ 呼び出し規約: hasCompleteLine()==true のときだけ呼ぶ
}