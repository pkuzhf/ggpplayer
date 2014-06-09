//#define WIN

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <sstream>
#include <time.h>
#include <string.h>

#include "reader.h"
#include "relation.h"
#include "prover.h"
#include  "dependgraph.h"
#include "statemachine.h"
#include "montecarloplayer.h"
#include "connect.h"

#ifdef WIN
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif

using namespace std;

#ifdef WIN
int connect() {
	WSADATA wsaData;
	SOCKET s;
	SOCKADDR_IN ServerAddr;
	int Port = 80;
	char Addr[] = "162.105.81.73";

	//初始化Windows Socket 2.2

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( s == INVALID_SOCKET ) {
		cout << "Create Socket Failed::" << GetLastError() <<endl;
		return -1;
	}

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(Port); 
	ServerAddr.sin_addr.s_addr = inet_addr(Addr);
	memset(ServerAddr.sin_zero, 0x00, 8);

	int Ret = connect(s, (SOCKADDR *)&ServerAddr, sizeof(ServerAddr));
	if ( Ret == SOCKET_ERROR ) {
		cout << "Connect Error::" << GetLastError() << endl;
		return -1;
	}
	
	// 新的连接建立后，就可以互相通信了，在这个简单的例子中，我们直接关闭连接，
	// 并关闭监听Socket，然后退出应用程序
	char buf[1000];
	int len;

	while(true) {
		cin.getline(buf, 1000);
		len = send(s, buf, strlen(buf), 0);
		cout << len <<endl;
		len = recv(s, buf, 1000, 0);
		cout << len << buf <<endl;
	}
	closesocket(s);
	WSACleanup();
}
#endif

int main() {

	srand(time(0));
	for(int i = 0 ; i < relation_type_num; ++i){		
		Relation::addSymbol(relation_type_words[i]);
	}
	Reader r;
	if (!r.readFile("gdl/rule.txt")) {
	//if (!r.readFile("gdl/connect_four.txt")) {
	//if (!r.readFile("gdl/2pffa_zerosum.kif")) {
		cout << Connect::message("debug", "read file failed.");
        return -1;
    }
	char buf[100000];
	cin.getline(buf, 100000);
	//r.readLine(buf);
	Relations rs;
	r.getRelations(rs);
	StateMachine machine(rs);
	for (int i = 0; i < Prover::time.size(); ++i) {
		ostringstream msg;
		msg << "time" << i << ": " << Prover::time[i];
		cout << Connect::message("debug", msg.str());
	}
	//machine.randomGo(clock() + 100000);
	cin.getline(buf, 10000);
	string game = string(buf);
    cin.getline(buf, 10000);
	int role, playclock;
	role = Relation::symbol2code[string(buf)];
	cin.getline(buf, 10000);
	playclock = (atoi(buf) - 2) * CLOCKS_PER_SEC;
	MonteCarloPlayer player(rs, role);   // montecarlo player
	// check montecarlo player
	/*MonteCarloPlayer Mplayer1(rs, 0);
	MonteCarloPlayer Mplayer2(rs, 1);
	while(!Mplayer1.is_terminal_){
		Propositions moves;
		moves.push_back(Mplayer1.stateMachineSelectMove(2000));
		moves.push_back(Mplayer2.stateMachineSelectMove(2000));
		Mplayer1.goOneStep(moves);
		Mplayer2.goOneStep(moves);
		cout<<"real moves:"<< moves[0].toString()<<endl;
		cout<<"real moves:"<< moves[1].toString()<<endl;
	}*/	
		
	cin.getline(buf, 10000);
	cout << Connect::message("client", Proposition::propsToStr(player.current_state_));
	cout << Connect::message("server", player.getRandomMove().items_[1].toString());

	while (true) {				
		cin.getline(buf, 10000);
		char * space = strstr(buf, " ");
		if (space == NULL) continue;
		*space = '\0';
		string cmd = string(buf);		
		if (cmd == "server") {
			Reader move_reader;
			move_reader.file_content_ = string(space + 1);
			Propositions joint_move;
			move_reader.getPropositions(joint_move);
			for (int i = 0; i < joint_move.size(); ++i) {
				Relation does;
				does.head_ = r_does;						
				does.items_.push_back(player.state_machine_.prover_.roles_[i].toRelation().items_[0]);
				does.items_.push_back(joint_move[i].toRelation());
				joint_move[i] = does.toProposition();
			}		
			player.goOneStep(joint_move);
			if (player.is_terminal_) {
				break;
			}
			//Proposition move = player.stateMachineSelectMove(playclock);
			//cout << move.items_[1].toString() << endl;
			cout << Connect::message("server", player.getRandomMove().items_[1].toString());
			cout << Connect::message("client", Proposition::propsToStr(player.selectLeafNode()->state_));
		} else if (cmd == "client") {
			char * semi = strstr(space + 1, ";");
			if (semi == NULL) continue;
			*semi = '\0';
			Reader state_reader;
			state_reader.file_content_ = string(space + 1);
			Propositions state;
			state_reader.getPropositions(state);
			player.updateTree(state, string(semi + 1));
			cout << Connect::message("server", player.getBestMove().items_[1].toString());
			cout << Connect::message("client", Proposition::propsToStr(player.selectLeafNode()->state_));
		}
	}
	return 0;
}
