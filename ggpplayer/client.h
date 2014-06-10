#ifndef CONNECT_H
#define CLIENT_H

#define WIN

#include <string>
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
	string buffer_;
	int length_;
	string game_;
	MonteCarloPlayer player_;

	Client();
	int connectServer();
	void receiveData(string data);
	void handleMessage(string msg);
	void sendMessage(string msg);
#endif
	static string message(string cmd, string content);
};

#endif