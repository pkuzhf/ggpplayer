#ifndef CONNECT_H
#define CLIENT_H

#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <string>
#include "config.h"
#include "montecarloplayer.h"

#ifdef WIN
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#else

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
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
	int client_socket;
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
