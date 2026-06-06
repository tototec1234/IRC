#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <sys/poll.h>
#include <vector>
#include <poll.h>

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
		Server();
		Server(const Server&);
		Server& operator=(const Server&);
};


#endif
