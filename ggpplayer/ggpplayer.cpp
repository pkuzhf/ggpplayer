#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <sstream>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "reader.h"
#include "relation.h"
#include "prover.h"
#include  "dependgraph.h"
#include "statemachine.h"
#include "montecarloplayer.h"
#include "client.h"
#include "config.h"

using namespace std;

void run_client() {
	Client client;
	client.connectServer();
}

void run_server() {
	//Reader r;
	//if (!r.readFile("gdl/tic_tac_toe.txt")) {
	//	cerr << Client::message("debug", "read file failed.");
 //   }
	//Relations rs;
	//r.getRelations(rs);
	//StateMachine machine(rs);
	//machine.randomGo(clock() + 100000);
	//
	string buf;

	Reader r;
	getline(cin, buf);
	r.readLine(buf);
	Relations rs;
	r.getRelations(rs);

    getline(cin, buf);
	string role = buf;

	MonteCarloPlayer player(rs, role);   // montecarlo player
	//for (int i = 0; i < 10000; ++i) {
	//	player.uct(CLOCKS_PER_SEC * 1, CLOCKS_PER_SEC * 5, 10000000);
	//}
	/*for (int i = 0; i < Prover::time.size(); ++i) {
		ostringstream msg;
		msg << "time" << i << ": " << Prover::time[i];
		cerr << Client::message("debug", msg.str());
	}*/
	cerr << Client::message("ready", "");

	ostringstream s;
	s << Proposition::propsToStr(player.root_->getState());
	cerr << Client::message("state", s.str());
	cerr << Client::message("move", player.getRandomMove().items_[1].toString());
	
	int io_time = 1;
	int total_time = 1;
	int update_time = 1;
	char * buffer = new char[1024 * 1024 * 1024];
	while (true) {
		int start = clock();
		//getline(cin, buf);
		fgets(buffer, 1024 * 1024 * 1024, stdin);
		buf = string(buffer);
		io_time += clock() - start;
		int p_space = buf.find(" ");
		if (p_space == buf.npos) continue;
		string cmd = buf.substr(0, p_space);
		if (cmd == "server") {
			string move = buf.substr(p_space + 1);
			if (move == "nil") {
				continue;
			}
			Reader move_reader;
			move_reader.file_content_ = move;
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
			//cerr << move.items_[1].toString() << endl;
			cerr << Client::message("move", player.getRandomMove().items_[1].toString());
			ostringstream s;
			s << Proposition::propsToStr(player.root_->getState());
			cerr << Client::message("state", s.str());
		} else if (cmd == "client") {
			int p_state = p_space + 1;
			int p_semi = buf.find(";", p_state);
			string s_state = buf.substr(p_state, p_semi - p_state);
			string s_tree = buf.substr(p_semi + 1);
			Reader state_reader;
			state_reader.file_content_ = s_state;
			Propositions state;
			state_reader.getPropositions(state);
			player.updateTree(state, s_tree);
			ostringstream o;
			o << "(" << player.root_->points_ / (player.root_->attemps_ + 1) << "/" << player.root_->attemps_ << ") ";
			for (int i = 0; i < player.root_->sons_.size(); ++i) {
				o << "{ ";
				for (int j = 0; j < player.root_->sons_[i].size(); j++) {
					o << "<" << player.root_->sons_[i][j]->points_ / (player.root_->sons_[i][j]->attemps_ + 1) << "/" << player.root_->sons_[i][j]->attemps_ << "> ";
				}
				o << "} ";
			}
			o << player.map_state_node_.size();
			o << " | " << io_time / (double)total_time << " | " << update_time / (double)total_time;
			cerr << Client::message("stat", o.str());
			cerr << Client::message("move", player.getBestMove().items_[1].toString());
			int s_update = clock();
			Node * node = player.selectLeafNodeServer();
			update_time += clock() - s_update;
			ostringstream s;
			s << Proposition::propsToStr(node->getState());
			cerr << Client::message("state", s.str());
			cerr << Client::message("updated", "");
		}
		total_time += clock() - start;
	}
}

int main() {
	srand(time(0));
	Relation::initSymbolTable();

#ifdef CLIENT
	run_client();
#else
	run_server();
#endif
	return 0;
}
