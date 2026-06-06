#include "a/Server.hpp"

Server::Server(int port, const std::string& pw) : _listenFd(-1) {
    (void)port;
    (void)pw;
}

Server::~Server() {}

void Server::run() {}