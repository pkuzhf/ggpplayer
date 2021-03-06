#include <string>
#include <sstream>
#include <iostream>
#include <time.h>

#include "client.h"
#include "reader.h"
#include "relation.h"
#include "montecarloplayer.h"

using namespace std;

Client::Client():player_() {
	length_ = -1;
}
string Client::message(string cmd, string content) {
	ostringstream ret;
	ret << cmd.size() + 1 + content.size();
	ret << " ";
	ret << cmd;
	ret << " ";
	ret << content;
	return ret.str();
}

#ifdef WIN

int Client::connectServer() {
	WSADATA wsaData;
	SOCKADDR_IN ServerAddr;
	int Port = 10000;
	char Addr[] = "162.105.81.43";

	//初始化Windows Socket 2.2

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( socket_ == INVALID_SOCKET ) {
		cerr << "Create Socket Failed::" << GetLastError() <<endl;
		return -1;
	}

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(Port); 
	ServerAddr.sin_addr.s_addr = inet_addr(Addr);
	memset(ServerAddr.sin_zero, 0x00, 8);

	int Ret = connect(socket_, (SOCKADDR *)&ServerAddr, sizeof(ServerAddr));
	if ( Ret == SOCKET_ERROR ) {
		cerr << "Connect Error::" << GetLastError() << endl;
		return -1;
	}
	
	// 新的连接建立后，就可以互相通信了，在这个简单的例子中，我们直接关闭连接，
	// 并关闭监听Socket，然后退出应用程序
	const int buf_size = 1000000;
	char buf[buf_size];
	int len;

	while(true) {
		len = recv(socket_, buf, buf_size, 0);
		if (len > 0) {
			buf[len] = '\0';
			receiveData(string(buf));
		}
	}

	closesocket(socket_);
	WSACleanup();
}

void Client::sendMessage(string msg) {
	send(socket_, msg.c_str(), msg.size(), 0);
	cerr << "send: " + msg << endl;
}

#else

int Client::connectServer() {

	int Port = 10000;
	char Addr[] = "162.105.81.43";

	struct sockaddr_in client_addr;
    bzero(&client_addr,sizeof(client_addr)); //把一段内存区的内容全部设置为0
    client_addr.sin_family = AF_INET;    //internet协议族
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);//INADDR_ANY表示自动获取本机地址
    client_addr.sin_port = htons(0);    //0表示让系统自动分配一个空闲端口
    //创建用于internet的流协议(TCP)socket,用client_socket代表客户机socket
    client_socket = socket(AF_INET,SOCK_STREAM,0);
    if( client_socket < 0)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }
    //把客户机的socket和客户机的socket地址结构联系起来
    if( bind(client_socket,(struct sockaddr*)&client_addr,sizeof(client_addr)))
    {
        printf("Client Bind Port Failed!\n"); 
        exit(1);
    }

    //设置一个socket地址结构server_addr,代表服务器的internet地址, 端口
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(inet_aton(Addr,&server_addr.sin_addr) == 0) //服务器的IP地址来自程序的参数
    {
        printf("Server IP Address Error!\n");
        exit(1);
    }
    server_addr.sin_port = htons(Port);
    socklen_t server_addr_length = sizeof(server_addr);
    //向服务器发起连接,连接成功后client_socket代表了客户机和服务器的一个socket连接
    if(connect(client_socket,(struct sockaddr*)&server_addr, server_addr_length) < 0)
    {
        printf("Can Not Connect To!\n");
        exit(1);
    }

	
	// 新的连接建立后，就可以互相通信了，在这个简单的例子中，我们直接关闭连接，
	// 并关闭监听Socket，然后退出应用程序
	int buf_size = 1000000;
	char buf[buf_size];
	int len;

	while(true) {
		len = recv(client_socket, buf, buf_size, 0);
		if (len > 0) {
			buf[len] = '\0';
			receiveData(string(buf));
		}
	}
	close(client_socket);
	return 0;
}
void Client::sendMessage(string msg) {
	send(client_socket, msg.c_str(), msg.size(),0);
	cerr << "send: " + msg << endl;
}

#endif

void Client::receiveData(string data) {
	if (length_ == -1) {
		int i = 0;
		while (i < data.size() && data[i] != ' ') ++i;
		if (i < data.size()) {
			length_ = atoi(data.substr(0, i).c_str());
			data = data.substr(i + 1);
		}
	}
	if (length_ != -1) {
		buffer_ += data;
		if (length_ == buffer_.size()) {
			handleMessage(buffer_);
			length_ = -1;
			buffer_ = "";
		} else if (length_ < buffer_.size()) {
			handleMessage(buffer_.substr(0, length_));
			string rest = buffer_.substr(length_);
			length_ = -1;
			buffer_ = "";
			receiveData(rest);
		}
	}
}

void Client::handleMessage(string msg) {
	cerr << msg << endl;
	int i = 0;
	while (i < msg.size() && msg[i] != ' ') ++i;
	if (i < msg.size()) {
		string cmd = msg.substr(0, i);
		msg = msg.substr(i + 1);
		if (cmd == "rule") {
			i = 0;
			while (i < msg.size() && msg[i] != ' ') ++i;
			if (i < msg.size()) {
				string role = msg.substr(0, i);
				Relation::initSymbolTable();
				Reader rule_reader;
				rule_reader.file_content_ = msg.substr(i + 1);
				Relations rs;
				rule_reader.getRelations(rs);
				player_ = MonteCarloPlayer(rs, role);
				sendMessage(message("ready", ""));
			}
		} else if (cmd == "state") {
			Reader state_reader;
			state_reader.file_content_ = msg;
			Propositions state;
			state_reader.getPropositions(state);
			player_.setState(state);
			player_.uct(CLOCKS_PER_SEC * 0.5, CLOCKS_PER_SEC * 10, 1000);
			sendMessage(message("uct", player_.root_->toString()));
		}
	}
}

