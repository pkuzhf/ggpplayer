#ifndef CONNECT_H
#define CONNECT_H

#define WIN

#include <string>
#include "montecarloplayer.h"

#ifdef WIN
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif

using namespace std;

class Connect {
public:
	SOCKET socket_;
	string buffer_;
	int length_;
	string game_;
	MonteCarloPlayer player_;

	Connect();
	static string message(string cmd, string content);
	int connectServer();
	void receiveData(string data);
	void handleMessage(string msg);
};

#endif