#ifndef CONNECT_H
#define CLIENT_H

#include <string>
#include "config.h"
#include "montecarloplayer.h"

#ifdef WIN
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#endif

using namespace std;

class Client {
public:

#ifdef WIN
	SOCKET socket_;
#else
	struct sockaddr_in client_addr;
#endif
	string buffer_;
	int length_;
	MonteCarloPlayer player_;

	Client();
	int connectServer();
	void sendMessage(string msg);
	void receiveData(string data);
	void handleMessage(string msg);
	static string message(string cmd, string content);
};

#endif
