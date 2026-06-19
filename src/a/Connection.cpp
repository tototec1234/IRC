#include "a/Connection.hpp"
#include <string>
#include <sys/socket.h>   // recv
#include <cerrno>         // errno

Connection::Connection(int fd) : _fd(fd) {}
Connection::~Connection() {}
int Connection::getFd() const { return _fd; }

// イシュー #21 の#6: 先頭行(最初の '\n' まで／'\n' が無ければバッファ全体)が
//	512 バイトを超えていたら true。判定のみ。切断・出力は Server 側。
bool Connection::isLineTooLong() const {
	size_t n_pos = _recvBuffer.find('\n');
	size_t line_len = (n_pos == std::string::npos) ? _recvBuffer.size() : n_pos;
	return line_len > 510 + 1;			// IRCでの本文の長さは　510（本文+'\r\n' を含む生バイト　は512） '\n'だと１文字甘いが許容する
}
/*
python3 -c "import sys; sys.stdout.buffer.write(b'A'*512); print('\r\n')" | nc localhost 6667
↑これは当然OK

python3 -c "import sys; sys.stdout.buffer.write(b'A'*513); print('\r\n')" | nc localhost 6667
↑これは当然OUT

しかし
python3 -c "import sys; sys.stdout.buffer.write(b'A'*513); print('\n')" | nc localhost 6667
↑これも通してしまう　\r　の１文字分甘い

*/


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
		return false;
	if (n < 0)
		return false; /* エラー。bircd の r<0 も切断扱い
				  Phase7 で EAGAIN を「切断せずスキップ=true」に分ける
					 (client_read.c L94 のメモ) */
	_recvBuffer.append(buf, n);
	return true;
}

// ─────────────────────────────────────────────────────────────
// 修正版: \r\n または \n 単独のどちらでも行の完了とみなす
// ─────────────────────────────────────────────────────────────
bool Connection::hasCompleteLine() const {
	return _recvBuffer.find("\n") != std::string::npos ;
}

// ─────────────────────────────────────────────────────────────
// 修正版: \n を基準に切り出し、直前に \r があれば取り除く
// ─────────────────────────────────────────────────────────────
std::string Connection::popLine() {
	size_t n_pos = _recvBuffer.find('\n');
	
	size_t len = n_pos;
	if (n_pos > 0 && _recvBuffer[n_pos -1] == '\r'){
		len--;
	}

	std::string line = _recvBuffer.substr(0, len);
	_recvBuffer.erase(0, n_pos + 1);
	return line;
}

// ─────────────────────────────────────────────────────────────
// bircd(lesson3.5): client_write.c に対応。
//   if (buf_write_len == 0) return;              ← 送るもの無し
//   sent = send(cs, buf_write, buf_write_len, 0);
//   if (sent <= 0) { close; clean_fd; return; }  ← 失敗で切断
//   ft_memmove(buf_write, buf_write+sent, ...);  ← 送信済みを前詰め
// A層の違い:
//   - buf_write(固定長)+buf_write_len → std::string(_sendBuffer)（長さ内包）
//   - writeToSocket は send 専念で bool 返却。切断は呼び出し側(_handleWrite→Server)
//   - 1回の POLLOUT イベントにつき send 1回（残りは次の POLLOUT で）
// ─────────────────────────────────────────────────────────────
void Connection::bufferSend(const std::string& msg) {
	_sendBuffer += msg;
}

bool Connection::hasPendingOutput() const {
	return !_sendBuffer.empty();				// 中身があるとき true
}

bool Connection::writeToSocket() {
	if (_sendBuffer.empty())
		return true;							// bircd: buf_write_len==0 return
	ssize_t sent = send(_fd, _sendBuffer.c_str(), _sendBuffer.size(), 0);
	if (sent <= 0)
		return false;							// bircd: sent<=0 → 切断。Phase7でEAGAINは別扱い
	_sendBuffer.erase(0, sent);					// bircd: ft_memmove で前詰め
	return true;
}