#define WIN

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <time.h>

#include "reader.h"
#include "relation.h"
#include "prover.h"
#include  "dependgraph.h"
#include "statemachine.h"
#include "montecarloplayer.h"

#ifdef WIN
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif

using namespace std;

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

int main() {

	srand(time(0));
	for(int i = 0 ; i < relation_type_num; ++i){		
		Relation::addSymbol(relation_type_words[i]);
	}
	Reader r;
	if (!r.readFile("gdl/rule.txt")) {
	//if (!r.scanGDLFile("gdl/connect_four.txt")) {
	//if (!r.scanGDLFile("gdl/2pffa_zerosum.kif")) {
        cout << "read file failed." << endl;
        return -1;
    }
	char buf[10000];
	cin.getline(buf, 100000);
	r.readLine(buf);
	Relations rs;
	r.getRelations(rs);
	StateMachine machine(rs);
	//machine.randomGo(clock() + 100000);
	//cout << "generate: " << Prover::generate_time << endl;
	//cout << "time1: " << Prover::time1 << endl;
	//cout << "time2: " << Prover::time2 << endl;
	//cout << "time3: " << Prover::time3 << endl;
	//cout << "time4: " << Prover::time4 << endl;
	//cout << "time5: " << Prover::time5 << endl;
	//cout << "time6: " << Prover::time6 << endl;
	//cout << "time7: " << Prover::time7 << endl;
	//cout << "time8: " << Prover::time8 << endl;
	//cout << "time9: " << Prover::time9 << endl;
	//cout << "time10: " << Prover::time10 << endl;
	//cout << "time11: " << Prover::time11 << endl;
	//cout << "time12: " << Prover::time12 << endl;
	//cout << "time13: " << Prover::time13 << endl;
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
	cout << player.stateMachineSelectMove(playclock).items_[1].toString() << endl;
	
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
				//cout << "Terminal." << endl;
				break;
			}
			Proposition move = player.stateMachineSelectMove(playclock);
			cout << move.items_[1].toString() << endl;
		} else if (cmd == "client") {
			char * semi = strstr(space + 1, ";");
			if (semi == NULL) continue;
			*semi = '\0';
			Reader state_reader;
			state_reader.file_content_ = string(space + 1);
			Propositions state;
			state_reader.getPropositions(state);
			player.updateTree(state, string(semi + 1));
		}
	}
	return 0;
}
