#include "a/Server.hpp"
#include <iostream>
// #include <cstring>
// #include <string>

Server::Server(int port, const std::string& pw) : _listenFd(-1) {
    (void)port;
    (void)pw;
}

Server::~Server() {}

// API
void Server::run() {
	std::cout << "RUN RUN RUN" << std::endl;
}

// privete methods

void Server::_addFd(int fd, short events){
	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = events;
	pfd.revents = 0;
	_pollfds.push_back(pfd);
}
