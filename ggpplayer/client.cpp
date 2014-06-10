#include <string>
#include <sstream>
#include <iostream>

#include "client.h"
#include "reader.h"
#include "relation.h"

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
	int Port = 80;
	char Addr[] = "162.105.81.73";

	//��ʼ��Windows Socket 2.2

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( socket_ == INVALID_SOCKET ) {
		cout << "Create Socket Failed::" << GetLastError() <<endl;
		return -1;
	}

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(Port); 
	ServerAddr.sin_addr.s_addr = inet_addr(Addr);
	memset(ServerAddr.sin_zero, 0x00, 8);

	int Ret = connect(socket_, (SOCKADDR *)&ServerAddr, sizeof(ServerAddr));
	if ( Ret == SOCKET_ERROR ) {
		cout << "Connect Error::" << GetLastError() << endl;
		return -1;
	}
	
	// �µ����ӽ����󣬾Ϳ��Ի���ͨ���ˣ�������򵥵������У�����ֱ�ӹر����ӣ�
	// ���رռ���Socket��Ȼ���˳�Ӧ�ó���
	char buf[100000];
	int len;

	while(true) {
		len = recv(socket_, buf, 100000, 0);
		cout << len << buf <<endl;
		len = send(socket_, buf, strlen(buf), 0);
		cout << len <<endl;
	}
	closesocket(socket_);
	WSACleanup();
}

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
	int i = 0;
	while (msg[i] != ' ' && i < msg.size()) ++i;
	if (i < msg.size()) {
		string game = msg.substr(0, i);
		msg = msg.substr(i + 1);
		i = 0;
		while (msg[i] != ' ' && i < msg.size()) ++i;
		if (i < msg.size()) {
			string cmd = msg.substr(0, i);
			msg = msg.substr(i + 1);
			i = 0;
			if (cmd == "rule") {
				while (msg[i] != ' ' && i < msg.size()) ++i;
				if (i < msg.size()) {
					Reader rule_reader;
					rule_reader.file_content_ = msg.substr(i + 1);
					Relations rs;

					player_ = MonteCarloPlayer(
				}
			} else if (cmd == "state") {
				Reader state_reader;
				state_reader.file_content_ = msg.substr(i + 1);
				Propositions state;
				state_reader.getPropositions(state);
			}
		}

	}
}

#endif