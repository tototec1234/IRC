#include "a/Server.hpp"
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
// #include <cstring>
// #include <string>
#include <unistd.h> // close()

Server::Server(int port, const std::string& pw) : _listenFd(-1) {

	struct sockaddr_in	servAddr;	//ローカルアドレス

	/* accept 実装時に復活
	struct sockaddr_in	clntAddr; //クライアントアドレス
	*/

	/* 	soket	p41 着信接続用のソケットを作成*/
	_listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenFd < 0)
		throw std::runtime_error("socket() failed");

	/*　ローカルのアドレス構造体を作成　*/	
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.sin_port = htons(port);

	/* 	bind	p42 ローカルアドレスへバインド*/
	if (bind(_listenFd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
	{
		close(_listenFd);
		throw std::runtime_error("bind() 失敗");
	}

	  /* 	listen	p42 「接続要求をリスん中」というマークをソケットにつける」*/
	if (listen(_listenFd, MAX_CLIENTS) < 0)
	{
		close(_listenFd);
		throw std::runtime_error("listen() 失敗");
	}

	/*　bircd の　`init_fd.c`　相当の処理*/
	_addFd(_listenFd, POLLIN);
	std::cout << "Server listening on port " << port << std::endl;

	(void)pw;


}
//　 reinterpret_cast<struct sockaddr*>(&clntAddr) にしようか迷ったが、CPP06 ex01 で危険だったので保留
/* ************************************************************************** 
// reinterpret_cast has almost no restrictions == very free == very dangerous! Handle with care!
// Don't use reinterpret_cast until you have no choice but to use it.
// Examples of use cases:
// Storing pointer address values in integer variables
// Converting pointers of different types (e.g., double* to int*)
uintptr_t	Serializer::serialize(Data *ptr)
{
	// Test: static_cast cannot convert between pointer and integer types
	// return(static_cast<uintptr_t>(ptr)); // Compile error: static_cast cannot convert between pointer and integer types
	return (reinterpret_cast<uintptr_t>(ptr));
}

Data	*Serializer::deserialization(uintptr_t raw)
{
	return (reinterpret_cast<Data *>(raw));
}

*/

Server::~Server() {
	if (_listenFd >= 0)
    close(_listenFd);
	std::cout << RED_COLOR << "Server デストラクタ　コールド" << RESET_COLOR << std::endl;
}

// API
void Server::run() {
	std::cout << "RUN RUN RUN" << std::endl;
	while (true)
	{
		int ret = poll(&_pollfds[0], _pollfds.size(), -1);
		std::cout << "ret=" << ret
		<< " fd=" << _pollfds[0].fd
		<< " revents=" << _pollfds[0].revents
		<< std::endl;
		if (ret < 0)
			break; 	// errno 処理は後で
		for (size_t i = 0; i < _pollfds.size(); ++i)
		{
			usleep(100000);
			if(_pollfds[i].revents & POLLIN)
			{
				if(_pollfds[i].fd == _listenFd)
					std::cout << "_acceptClient(); " << std::endl;
					// _acceptClient(); // ここで accept
				}
		}
		 // TODO: revents 走査 → listen なら _acceptClient()
		}
}

// privete methods

void Server::_addFd(int fd, short events){
	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = events;
	pfd.revents = 0;
	_pollfds.push_back(pfd);
}
