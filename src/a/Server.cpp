#include "a/Server.hpp"
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
// #include <string>
#include <unistd.h> // close()
#include <arpa/inet.h> // inet_ntoa 用

#include <cstring>
#include <cerrno>

/*
これはrevents挙動確認用関数です 
*/
static void _printRevents(short revents); 


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
	int tmp = 4242;
	while (true)
	{	
		/*
		https://man7.org/linux/man-pages/man2/poll.2.html
		*/

		int ret = poll(&_pollfds[0], _pollfds.size(), -1);

		// ここから
		if (tmp != ret)
		{
			tmp = ret;
			std::cout
			<< BLUE_COLOR
			<< "ret=" << ret		// ret = ready な fd の総数（listen 限定ではない）
			<< " fd=" << _pollfds[0].fd
			<< " nfds=" << _pollfds.size()
			<< " [listen] fd=" << _pollfds[0].fd
			<< " revents="			// revents=0 → 今回の poll でイベントなし（監視は継続）
			<< RESET_COLOR;
			_printRevents(_pollfds[0].revents);
			std::cout << std::endl;

			for (size_t i = 0; i < _pollfds.size(); ++i)
			{
				std::cout << "  i=" << i << " fd=" << _pollfds[i].fd
				<< " revents=";
				_printRevents(_pollfds[i].revents);
				std::cout
				<< (_pollfds[i].fd == _listenFd ? " [listen]" : " [client]")
				<< std::endl;
			}
	
		}
		// ここまではデバッグ出力
		
		if (ret < 0)
			break; 	// errno 処理は後で

		/* revents 走査 → listen なら _acceptClient()　*/
		for (size_t i = 0; i < _pollfds.size(); ++i)
		{
			short 	rev = _pollfds[i].revents;
			int		fd = _pollfds[i].fd;
			if (rev == 0)
				continue;
			// (1) listen fd を最初に処理。client 系処理には流さない
			if (fd == _listenFd){
				if (rev & POLLIN)
				{
					std::cout << "_acceptClient(); " << std::endl;
					_acceptClient(); // ここで accept
				}
				continue; // Phase3以降ではアクセプトしたら次のfdに進む必要あり
			} 
			// (2) 以下は client fd のみ
			
			
			// (3) まず読む。recv==0(EOF) なら _handleRead が false → 切断
			if (rev & POLLIN) {
				if (!_handleRead(fd)) {     // recv<=0 等で false
					_disconnectClient(fd);
					--i;
					continue;
				}
			}
			// (4) 読み切った後で HUP/ERR を判定して切断
			/*
			POLLHUP は「もう書き込めない」を意味するが、受信バッファに未読データが残っていることがある。
			POLLHUP が立っていても、POLLIN もたっているケースがあるので
			(3)で先に読み切る！！
			*/
			if (rev & (POLLERR | POLLHUP | POLLNVAL)) {
				_disconnectClient(fd);
				--i;            // _pollfds が縮むので添字を戻す（走査中erase対策）
				continue;
			}
		}
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

void Server::_acceptClient()
{
	int			cs;
	struct sockaddr_in	csin;
	socklen_t	csin_len;

	csin_len = sizeof(csin);
	cs = accept(_listenFd, (struct sockaddr *)&csin, &csin_len);
	if (cs < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;	    // EAGAIN / EWOULDBLOCK は無視（Phase 7 で正式に扱う）
		std::cerr
		<< RED_COLOR
		<< "accept() 失敗"
		<< strerror(errno)
		<< RESET_COLOR << std::endl;
		return; // （→ accept は 1 回 1 fd なので return で十分）
	}
	// accept 成功後（cs が有効）
	Connection* conn = new Connection(cs);
	_connections[cs] = conn;
	_addFd(cs, POLLIN);
	// host 文字列（Phase4 の addClient 用）
	std::string host = inet_ntoa(csin.sin_addr);
	
	std::cout << GREEN_COLOR
	<< "New client #" << cs // csはクラアントのfd　#1 stdin  #2 stdout  #3 stderr #4 listenソケット なので必ず#4から 
	<< " from " << host
	<< ":" << ntohs(csin.sin_port)
	<< RESET_COLOR << std::endl;
}

/* #include <netinet/in.h> でsockaddr_inの中身確認するとこうなってる
 * Socket address, internet style.

 struct sockaddr_in {
	__uint8_t       sin_len;
	sa_family_t     sin_family;
	in_port_t       sin_port;
	struct  in_addr sin_addr;
	char            sin_zero[8];
};
*/

// bircd: client_read() 本体に対応。
//   read_and_store → while(get_crlf_pos){ extract_and_consume } の構造を
//   A層では readFromSocket → while(hasCompleteLine){ popLine } に置換。
//   bircd は extract 内で broadcast するが、A層は popLine で取り出した行を
//   Phase4 で B層(Parser→dispatch)へ渡す（今はログ確認のみ）。
bool Server::_handleRead(int fd) {
	Connection* conn = _connections[fd];
	if (!conn->readFromSocket()) {
		return false;
	}
	while (conn->hasCompleteLine()) {
		std::string line = conn->popLine();
		std::cout << "[recv #" << fd << "] " << line << std::endl;  // 動作確認
	}
	return true;
}

// bircd: client_read.c の close(cs); clean_fd(&e->fds[cs]); に対応。
//   bircd は clean_fd で FD_FREE マーク → 次ループ init_fd が pollfds 再構築でスキップ。
//   A層は _pollfds 持続なので close + _pollfds erase + Connection delete を自分でやる。
void Server::_disconnectClient(int fd) {
	close(fd);
	_removeFd(fd);
	std::map<int, Connection*>::iterator it = _connections.find(fd);
	if (it != _connections.end()) {
		delete it->second;
		_connections.erase(it);
	}
    // TODO(Phase4): _state.removeClient(fd);	// C層からも除去（_state 追加後に有効化）
	std::cout << "client #" << fd << " gone away" << std::endl;  // bircd の gone away 相当
}

void Server::_removeFd(int fd) {
	for (size_t i = 0; i < _pollfds.size(); ++i) {
		if (_pollfds[i].fd == fd) {
			_pollfds.erase(_pollfds.begin() + i);
			return;
		}
	}
}

/*
これは挙動確認用関数です
*/
static void _printRevents(short revents)
{
	std::cout << revents;
	if (revents == 0) {
		std::cout << "(none)";
		return;
	}
	std::cout << "[";
	if (revents & POLLIN)  std::cout << "IN ";
	if (revents & POLLOUT) std::cout << "OUT ";
	if (revents & POLLERR) std::cout << "ERR ";
	if (revents & POLLHUP) std::cout << "HUP ";
	std::cout << "]";
}


/*
バッファ動作確認方法

make && ./bircd 6667
# 別ターミナル2枚:
	# 受信側
	nc localhost 6667

	# 2メッセージ一括 → while の確認
	printf 'PING :a\r\nPING :b\r\n' | nc localhost 6667

	# 分割 → 累積の確認
	(printf 'PI'; sleep 1; printf 'NG :x\r\n') | nc localhost 6667  

	# \r\n無し200B → 切断の確認
	python3 -c "import sys; sys.stdout.buffer.write(b'A'*200)" | nc localhost 6667
*/