#include "a/Server.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h> // close()
#include <arpa/inet.h> // inet_ntoa 用

#include <cstring>
#include <cerrno>	// accept() 失敗ログ用 (errno / strerror)

#include <fcntl.h>	// fcntl, F_SETFL, O_NONBLOCK
#include <csignal>	// signal, SIGPIPE, SIG_IGN 

/*
これはrevents挙動確認用関数です 
*/
static void _printRevents(short revents); 

static void _setNonBlocking(int fd) {
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("fcntl(O_NONBLOCK) failed");
}

Server::Server(int port, const std::string& pw) : _listenFd(-1), _state(pw), _healthMonitor(30) {

	struct sockaddr_in	servAddr;	//ローカルアドレス

	/* accept 実装時に復活
	struct sockaddr_in	clntAddr; //クライアントアドレス
	*/

	signal(SIGPIPE, SIG_IGN);	// send 先が切断済みでも 無視。 SIGPIPE で落ちない sendは-1を返す

	/* 	soket	p41 着信接続用のソケットを作成*/
	_listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenFd < 0)
		throw std::runtime_error("socket() failed");
	_setNonBlocking(_listenFd);		// ★追加: listen fd を non-blocking に

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
	while (true){	
		/*
		https://man7.org/linux/man-pages/man2/poll.2.html
		*/

		// int ret = poll(&_pollfds[0], _pollfds.size(), -1);
		// -1の無限まちから 1秒(1000ms)待ちでタイムアウト検知を回す。
		int ret = poll(&_pollfds[0], _pollfds.size(), 1000);

		// ここからデバッグ出力
		if (tmp != ret){
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
				
				// hasTImeOut デバッグ出力（A層は本来叩かない）
				if (_pollfds[i].fd != _listenFd){
					std::cout <<  " [client]";
					if (_healthMonitor.hasTimedOut(_pollfds[i].fd)){
					std::cout << RED_COLOR << " [Timeout!!!]" << RESET_COLOR;
					} else if(_healthMonitor.isWaitingForPong(_pollfds[i].fd)){
					std::cout << YELLOW_COLOR << " [Waiting PONG...]" << RESET_COLOR;
					}
				} else {
					std::cout <<" [listen]";
				}		
				std::cout << std::endl;
			}
		}
		// ここまではデバッグ出力
		
		if (ret < 0)
			break; 	// errno 処理は後で

		/* revents 走査 → listen なら _acceptClient()　*/
		for (size_t i = 0; i < _pollfds.size(); ++i){
			short 	rev = _pollfds[i].revents;
			int		fd = _pollfds[i].fd;
			if (rev == 0)
				continue;
			// (1) listen fd を最初に処理。client 系処理には流さない
			// ここは通常1回しか通らない　　ハズ？
			if (fd == _listenFd){
				if (rev & POLLIN)
				{
					std::cout << "_acceptClient(); " << std::endl;
					_acceptClient();		// ここで accept
				}
				continue;
			} 
			// (2) 以下は client fd のみ
			// 
			if (rev & POLLIN) {
						//　①「クライアントの意思でclose() や shutdown()を行った場合（TCPのFINパケット（EOF）が届くが、その場合でもPOLLIN（読み込み可能）フラグは立ったままである。この状態で_handleReadがrecv()を呼ぶと、ブロックせずに即座に0を返してくる
						//　②PONGタイムアウトfdはループ末尾で処理
				if (!_handleRead(fd)) {//  EOF/recvエラー/行長すぎ　は非自発的失敗として切断
					if (_connections.find(fd) != _connections.end())
						_notifyAndDisconnect(fd, "Connection reset");	// connection_lifecycle_integration.md　 5.2 recv==0 / POLLHUP も DisconnectEvent に寄せる場合　準拠
					--i;
					continue;
				}
			}
			// (3) 書ける状態なら送る  bircd: check_fd.c の if(revents & POLLOUT) fct_write
			if (rev & POLLOUT) {
				if (!_handleWrite(fd)) {	// send 失敗で false
					_notifyAndDisconnect(fd, "Connection reset");
					--i;
					continue;
				}
			}
			// (4) 読み切った後で HUP/ERR を判定して切断
			/*
			POLLHUP は相手側が切断（hang up）した ことを示すイベントだが、受信バッファに未読データが残っていることがある。
			POLLHUP が立っていても、POLLIN もたっているケースがあるので
			(2)で先に読み切る！！
			*/
			if (rev & (POLLERR | POLLHUP | POLLNVAL)) {
				_notifyAndDisconnect(fd, "Connection reset");	// connection_lifecycle_integration.md　 5.2 recv==0 / POLLHUP も DisconnectEvent に寄せる場合　準拠
				--i;						// _pollfds が縮むので添字を戻す（走査中erase対策）
				continue;
			}

		}

		/*　PONG　未応答　でタイムアウトしたクライアントを切断　*/
		std::vector<int> timeOut = _healthMonitor.collectTimedOutClients();
		for (std::vector<int>::iterator it = timeOut.begin(); it != timeOut.end(); ++it)
			_notifyAndDisconnect(*it, "Ping timeout");
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
		std::cerr
		<< RED_COLOR
		<< "accept() 失敗"
		<< strerror(errno)
		<< RESET_COLOR << std::endl;
		return; // （→ accept は 1 回 1 fd なので return で十分）
	}
	// accept 成功後（cs が有効）
	_setNonBlocking(cs);		// ★追加: クライアント fd を non-blocking に

	Connection* conn = new Connection(cs);
	_connections[cs] = conn;
	_addFd(cs, POLLIN);
	std::string host = inet_ntoa(csin.sin_addr);
	
	//	dispatch は NULL なら早期 return するが、未登録 fd を作らないため accept 時に addClient しておく
	_state.addClient(cs, host);

	std::cout << GREEN_COLOR
	<< "New client #" << cs // csはクラアントのfd　#0 stdin  #1 stdout  #2 stderr #3 listenソケット よって最初のクライアントは #4 から 
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
//   Parser に送り　dispatch→applyCommandResult 受け取る
bool Server::_handleRead(int fd) {
	Connection* conn = _connections[fd];
	if (!conn->readFromSocket()) {
		return false;
	}

	/* 既知の問題点
	isLineTooLong() のチェックが readFromSocket() 直後に 1 回だけなので、
	1 回の recv で「短い1行 + 改行なしの巨大データ」を受け取った場合に、
	最初の短い行だけ pop して巨大な未完了行が _recvBuffer に残り続けます（次の recv が来ないと切断されない）
	*/
	if (conn->isLineTooLong()) {									// pop する前に弾く
		std::cerr << RED_COLOR << "line too long (#" << fd
					<< ") 長すぎ切断" << RESET_COLOR << std::endl;	// 切断理由を決めるのはA層の責任　とりま　デバッグ出力
		return false;												// run() が _notifyAndDisconnect を呼ぶ（QUIT 済みなら _connections ガードでスキップ）
	}

	_healthMonitor.updateActivity(fd);

	while (conn->hasCompleteLine()) {
		std::string line = conn->popLine();
		std::cout << "[recv #" << fd << "] " << line << std::endl;  // 動作確認
		Message		msg = Parser::parse(line);
		CommandResult result = _dispatcher.dispatch(fd, msg, _state,
																	_healthMonitor);
		applyCommandResult(result, fd);		// 送信先ごとに _enqueueReplies 経由で送信バッファへ積む
		if (result.shouldDisconnect)
			return false;
	}
	return true;
}

// bircd: client_read.c の close(cs); clean_fd(&e->fds[cs]); に対応。
//   bircd は clean_fd で FD_FREE マーク → 次ループ init_fd が pollfds 再構築でスキップ。
//   A層は _pollfds 持続なので close + _pollfds erase + Connection delete を自分でやる。
void Server::_disconnectClient(int fd) {
	close(fd);								// ①物理切断
	_removeFd(fd);							// ②pollfdsから除去　監視しない
	std::map<int, Connection*>::iterator it = _connections.find(fd);
	if (it != _connections.end()) {
		delete it->second;
		_connections.erase(it);
		_state.removeClient(fd);			// ③C層からも除去　論理除去
	}
	_healthMonitor.removeClient(fd);	// ④ライフサイクル層も除去（fd再利用バグ・リーク対策）
	// _clients のエントリは updateActivity(fd) で作られ、_connections の有無とは独立。
	// 従って_connections に無くても _clients には残り得るので、if の外（無条件）に置く
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
bircd: init_fd.c の「strlen(buf_write)>0 で events|=POLLOUT」に対応。
bircd は毎ループ pollfds を作り直すが、A は持続配列なので明示トグル。
*/

void Server::_enablePollout(int fd) {
	for (size_t i = 0; i < _pollfds.size(); ++i)
	/*　データを積んだとき → _enablePollout（init_fd.c L59-60 の「strlen(buf_write)>0 で立てる」を、
		積んだ瞬間に手動でやる）　*/
		if (_pollfds[i].fd == fd) { _pollfds[i].events |= POLLOUT; return; }	// enable　|= FLAG で立てる
}

void Server::_disablePollout(int fd) {
	for (size_t i = 0; i < _pollfds.size(); ++i)
	/*　送り切ったとき → _disablePollout（bircd なら次ループで自動的に下りる部分を、手動で下ろす）*/
	if (_pollfds[i].fd == fd) { _pollfds[i].events &= ~POLLOUT; return; }	// disable　&= ~FLAG で下ろす
}

// send 部分: bircd client_write.c に対応（send + 送信済み消費）。
// _disablePollout 部分: bircd は init_fd.c が毎ループ POLLOUT を計算し直すので
//   buf_write が空になれば自動で下りる。A は _pollfds 持続のため明示的に下ろす。
bool Server::_handleWrite(int fd) {
	Connection* conn = _connections[fd];
	if (!conn->writeToSocket())
		return false;						// send 失敗 → 呼び出し側で切断
	if (!conn->hasPendingOutput())
		_disablePollout(fd);				// 空POLLOUTで poll が回り続けるのを防ぐ
	return true;
}

// B層が作った CommandResult を A層の送信経路へ流す。
// 送信先 fd は source fd とは限らない（JOIN 等は他メンバーへブロードキャスト）。
void Server::applyCommandResult(const CommandResult& result, int sourceFd) {
	_enqueueReplies(result);	// "Client Quit" の根拠は— connection_lifecycle_integration.md §9 / irssi_handson_common.md}
	if (result.shouldDisconnect)
		_notifyAndDisconnect(sourceFd, "Client Quit");
}

void Server::_enqueueReplies(const CommandResult& result){
	for (size_t i = 0; i < result.replies.size(); ++i) {
		int target = result.replies[i].fd;
		std::map<int, Connection*>::iterator it = _connections.find(target);
		if (it == _connections.end())
			continue;					// クラッシュ防止のため、B層が既に切断済み等になっているfdを返してきたばあいはスルーする。
		it->second->bufferSend(result.replies[i].message);
		_enablePollout(target);
	}
}

void Server::_notifyAndDisconnect(int fd, const std::string& reason){
	// ① 切断イベントを作成　DisconnectEvent を作る（fd, reason）
	DisconnectEvent event(fd, reason);
	// ② B層に他の参加者（チャンネル）への QUIT 通知メッセージを作ってもらう
	CommandResult notify = _disconnectNotifier.build(event, _state);
	// ③ _enqueueReplies(notify)
	_enqueueReplies(notify);
	// ④ _disconnectClient(fd)
	_disconnectClient(fd);
}

/*
バッファ動作確認方法

./ircserv 6667 pass
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
