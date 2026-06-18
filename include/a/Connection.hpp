#ifndef CONNECTION_HPP
#define CONNECTION_HPP

# define FD_FREE	0
# define FD_SERV	1
# define FD_CLIENT	2

#define MAX_CLIENTS 42  //とりあえず最大同時接続42名　	ft_irc（提出）ではvector<pollfd> 動的　で上限は　実質 OS の fd 上限の予定？

# define BUF_SIZE	4096

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

 private:
	int			_fd;
	std::string	_recvBuffer;
	std::string	_sendBuffer;

	Connection();
	Connection(const Connection&);
	Connection& operator=(const Connection&);
};

#endif
