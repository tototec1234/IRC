#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>

class Server {
	public:
		Server(int port, const std::string& password);
		~Server();
		void run();

	private:
		int _listenFd;

		Server();
		Server(const Server&);
		Server& operator=(const Server&);
};


#endif
