#include <cstdlib>
#include <iostream>
#include <string>

#include "a/Server.hpp"

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return 1;
	}

	char* end;
	long  port = std::strtol(argv[1], &end, 10);
	if (*end != '\0' || port <= 0 || port > 65535) {
		std::cerr << "Error: invalid port '" << argv[1] << "'" << std::endl;
		return 1;
	}

	Server server(static_cast<int>(port), std::string(argv[2]));
	server.run();
	return 0;
}
