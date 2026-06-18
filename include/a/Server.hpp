#ifndef SERVER_HPP
#define SERVER_HPP

#include "../AnsiColor.hpp"
 #include <string.h>	// man 3 memset p41
#include <string>
// #include <sys/poll.h>
#include <vector>
#include <poll.h>
#include "a/Connection.hpp"
#include <map>

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
		std::map<int, Connection*>  _connections;	// ← Phase 3 追加。fd → Connection*
		bool _handleRead(int fd);					// ← 戻り値を bool に変更（false=切断要求）

		/*
		切断後 POLLHUP で busy-loop　の対策

		Phase6 を先行し _disconnectClient(close+_pollfds除去+delete)で対応
		run() は for (size_t i; i<_pollfds.size(); ++i) で添字走査している。
		ループ途中で 	_pollfds から要素を erase すると後ろが前へ詰まり、++i で 1 個飛ばす。
		とりま対策は
		「erase したら --i で添字を戻す」。

		*/
		void _disconnectClient(int fd);		// ← 追加
		void _removeFd(int fd);				// ← 追加

		Server();
		Server(const Server&);
		Server& operator=(const Server&);
};


#endif

