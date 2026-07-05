#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <sys/socket.h> //send

class Connection {
 public:
 	// Connection(int fd);
 	explicit Connection(int fd);
	~Connection();

	int			getFd() const;
	bool		readFromSocket();
	bool		writeToSocket();
	bool		hasCompleteLine() const;
	std::string	popLine();
	void		bufferSend(const std::string& msg);
	bool		hasPendingOutput() const;
	bool		isLineTooLong() const;   // ★追加: 先頭行が 512B 超か（イシュー #21　の#6 対策で判定のみ）


 private:
	static const size_t buf_size = 4096;

	int			_fd;
	mutable size_t _nlPos;		// mutable は、「const メンバ関数（状態を変えない関数）の中からでも、この変数だけは例外的に書き換えてもいいよ」とコンパイラに許可を与えるキーワード
	std::string	_recvBuffer;
	std::string	_sendBuffer;

	Connection();
	Connection(const Connection&);
	Connection& operator=(const Connection&);
};

#endif
