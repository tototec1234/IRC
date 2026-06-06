#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>

class Connection {
 public:
	explicit Connection(int fd);
	~Connection();

	int			getFd() const;
	bool		readFromSocket();
	bool		writeToSocket();
	bool		hasCompleteLine() const;
	std::string	popLine();
	void		bufferSend(const std::string& msg);
	bool		hasPendingOutput() const;

 private:
	int			_fd;
	std::string	_recvBuffer;
	std::string	_sendBuffer;

	Connection();
	Connection(const Connection&);
	Connection& operator=(const Connection&);
};

#endif
