#include "a/Connection.hpp"

Connection::Connection(int fd) : _fd(fd) {}
Connection::~Connection() {}
int Connection::getFd() const { return _fd; }