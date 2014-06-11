#ifndef CONNECT_H
#define CLIENT_H

#include <string>
#include "config.h"
#include "montecarloplayer.h"

#ifdef WIN
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif

using namespace std;

class Client {
public:

#ifdef WIN
	SOCKET socket_;
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
