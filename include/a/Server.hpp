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

#include "b/Parser.hpp"
#include "b/CommandDispatcher.hpp"
#include "b/CommandResult.hpp"
#include "c/ServerState.hpp"

#include <fcntl.h>	// fcntl, F_SETFL, O_NONBLOCK
#include <csignal>	// signal, SIGPIPE, SIG_IGN 

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
		std::map<int, Connection*>  _connections;
		bool _handleRead(int fd);

		bool _handleWrite(int fd);		// bircd: check_fd.c の POLLOUT 分岐
		void _enablePollout(int fd);	// 送信データを積んだとき POLLOUT 監視ON
		void _disablePollout(int fd);	// 送り切ったとき OFF（空POLLOUTのbusy回避）
		
		void _disconnectClient(int fd);
		void _removeFd(int fd);	

		void applyCommandResult(const CommandResult& result); 
		
		ServerState			_state;		// password を保持。ctor で初期化必須
		CommandDispatcher	_dispatcher;// ステートレス
		Server();
		Server(const Server&);
		Server& operator=(const Server&);
};


#endif

