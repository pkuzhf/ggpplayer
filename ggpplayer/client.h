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
	int connectServer();
	void sendMessage(string msg);
#endif

	string buffer_;
	int length_;
	MonteCarloPlayer player_;

	Client();
	void receiveData(string data);
	void handleMessage(string msg);
	static string message(string cmd, string content);
};

#endif