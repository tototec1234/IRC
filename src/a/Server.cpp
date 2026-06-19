#include "a/Server.hpp"
#include <iostream>
#include <locale>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
// #include <string>
#include <unistd.h> // close()
#include <arpa/inet.h> // inet_ntoa 用

#include <cstring>
#include <cerrno>	//　デバッグ出力用　後で消す
#include <fcntl.h>
#include <csignal>

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

Server::Server(int port, const std::string& pw) : _listenFd(-1), _state(pw) {

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
					_acceptClient();		// ここで accept
				}
				continue; // Phase3以降ではアクセプトしたら次のfdに進む必要あり
			} 
			// (2) 以下は client fd のみ
			
			
			// (3) まず読む。recv==0(EOF) なら _handleRead が false → 切断
			if (rev & POLLIN) {
				if (!_handleRead(fd)) {		// recv<=0 等で false
					_disconnectClient(fd);
					--i;
					continue;
				}
			}
			// (3.5) 書ける状態なら送る  bircd: check_fd.c の if(revents & POLLOUT) fct_write
			if (rev & POLLOUT) {
				if (!_handleWrite(fd)) {	// send 失敗で false
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
				--i;						// _pollfds が縮むので添字を戻す（走査中erase対策）
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
	
	/* 必須!!!! dispatch が client を NULL 前提で deref(参照) するため
	  dispatch は PING/PASS 以外で client->isPassOk() を NULL チェックなしで呼ぶ
	 accept 時に必ず addClient しておけば getClientByFd(fd) が非 NULL になり回避できる。
	*/
	_state.addClient(cs, host);

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
//   Parser に送り　dispatch→applyCommandResult 受け取る
bool Server::_handleRead(int fd) {
	Connection* conn = _connections[fd];
	if (!conn->readFromSocket()) {
		return false;
	}
	
	if (conn->isLineTooLong()) {									// pop する前に弾く
		std::cerr << RED_COLOR << "line too long (#" << fd
					<< ") 長すぎ切断" << RESET_COLOR << std::endl;	// 切断理由を決めるのはA層の責任　とりま　デバッグ出力
		return false;												// run() が _disconnectClient(fd) を呼ぶ
	}
/* 既知の問題点
isLineTooLong() のチェックが readFromSocket() 直後に 1 回だけなので、
1 回の recv で「短い1行 + 改行なしの巨大データ」を受け取った場合に、
最初の短い行だけ pop して巨大な未完了行が _recvBuffer に残り続けます（次の recv が来ないと切断されない）
*/
	while (conn->hasCompleteLine()) {
		std::string line = conn->popLine();
		std::cout << "[recv #" << fd << "] " << line << std::endl;  // 動作確認
		// conn->bufferSend(line + "\r\n");							// ← エコー：送信バッファへ積む
		Message		msg = Parser::parse(line);
		CommandResult result = _dispatcher.dispatch(fd, msg, _state);
		applyCommandResult(result);									// 送信先ごとに bufferSend + _enablePollout 済み
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
		_state.removeClient(fd);			// C層からも除去
	}
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
void Server::applyCommandResult(const CommandResult& result) {
	for (size_t i = 0; i < result.replies.size(); ++i) {
		int target = result.replies[i].fd;
		std::map<int, Connection*>::iterator it = _connections.find(target);
		if (it == _connections.end())
			continue;					// 既に切断済み等。落とさない
		it->second->bufferSend(result.replies[i].message);
		_enablePollout(target);
	}
	//	TODO: result.shouldDisconnect が true なら source fd を切断。
	//	現状 B は QUIT 未実装で常に false。配線は後続（flush後closeのgraceful論点込み）。
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

/*
送信経路（POLLOUT + send）動作確認方法 — エコーで自己検証（B層非依存）

	# 複数行が順に返るか（while ループ＋送信）
(printf 'a\r\nb\r\nc\r\n'; sleep 1) | nc 127.0.0.1 6667
# → a / b / c
# POLLOUT が下りているか（送信後ビジーループしていないか）→ top で ircserv の CPU が張り付かないこと
*/

/*
🎯 初の A↔B↔C 結合テスト — PING/PONG が B層経由で返ることを確認する。
   Phase4 で _handleRead のエコーを Parser::parse → _dispatcher.dispatch
   → applyCommandResult に差し替え。応答は A単体のエコー(PING :foo)ではなく、
   B層 ReplyBuilder::pong 経由の prefix 付き (:irc.local PONG irc.local :foo) になる。
   recv(A) → parse/dispatch(B) → ServerState(C) → CommandResult → 送信(A) が一本に繋がる初めての検証。

	# B経由でPONGが返るか（prefix付きなので判定は " PONG " 含有で見る）
(printf 'PING :foo\r\n'; sleep 1) | nc 127.0.0.1 6667
# → :irc.local PONG irc.local :foo

	# 登録フロー（PASS→NICK→USER）も通るか（B経由）
(printf 'PASS pw\r\nNICK alice\r\nUSER a a a a\r\n'; sleep 1) | nc 127.0.0.1 6667
*/