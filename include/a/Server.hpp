#ifndef SERVER_HPP
#define SERVER_HPP

#include "../AnsiColor.hpp"
 #include <string.h>	// man 3 memset p41
#include <string>
// #include <sys/poll.h>
#include <vector>
#include <poll.h>

#define MAX_CLIENTS 42  //とりあえず最大同時接続42名　	ft_irc（提出）ではvector<pollfd> 動的　で上限は　実質 OS の fd 上限の予定？

class Server {
	public:
		Server(int port, const std::string& password);
		~Server();
		void run();

	private:
		int _listenFd;
		std::vector<struct pollfd> _pollfds;
		void _addFd(int fd, short events);
		void _acceptClient();
		void _setupListenSocket();

		Server();
		Server(const Server&);
		Server& operator=(const Server&);
};


#endif
