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
	const int buf_size = 100000;
	char buf[buf_size];

	Reader r;
	cin.getline(buf, buf_size);
	r.readLine(buf);
	Relations rs;
	r.getRelations(rs);

    cin.getline(buf, buf_size);
	string role = string(buf);

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
	s << player.root_->code_ << ";" << Proposition::propsToStr(player.root_->state_);
	cerr << Client::message("state", s.str());
	cerr << Client::message("move", player.getRandomMove().items_[1].toString());
	cin.getline(buf, buf_size);

	while (true) {				
		cin.getline(buf, buf_size);
		char * p_space = strstr(buf, " ");
		if (p_space == NULL) continue;
		*p_space = '\0';
		string cmd = string(buf);		
		if (cmd == "server") {
			Reader move_reader;
			move_reader.file_content_ = string(p_space + 1);
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
			s << player.root_->code_ << ";" << Proposition::propsToStr(player.root_->state_);
			cerr << Client::message("state", s.str());
		} else if (cmd == "client") {
			char * p_code = p_space + 1;
			char * p_semi = strstr(p_code, ";");
			*p_semi = '\0';
			char * p_state = p_semi + 1;
			p_semi = strstr(p_state, ";");
			*p_semi = '\0';
			char * p_tree = p_semi + 1;
			Reader state_reader;
			state_reader.file_content_ = string(p_state);
			Propositions state;
			state_reader.getPropositions(state);			
			player.updateTree(atoi(p_code), state, string(p_tree));
			ostringstream o;
			o << "(" << player.root_->points_ / (player.root_->attemps_ + 1) << "/" << player.root_->attemps_ << ") ";
			for (int i = 0; i < player.root_->sons_.size(); ++i) {
				o << "{ ";
				for (int j = 0; j < player.root_->sons_[i].size(); j++) {
					o << "<" << player.root_->sons_[i][j]->points_ / (player.root_->sons_[i][j]->attemps_ + 1) << "/" << player.root_->sons_[i][j]->attemps_ << "> ";
				}
				o << "} ";
			}
			pair<int, int> move = player.root_->getMaximinMove();
			o << player.nodes_.size();
			//o << player.root_->toString();
			cerr << Client::message("stat", o.str());
			cerr << Client::message("move", player.getBestMove().items_[1].toString());
			Node * node = player.selectLeafNode();
			ostringstream s;
			s << node->code_ << ";" << Proposition::propsToStr(node->state_);
			cerr << Client::message("state", s.str());
			cerr << Client::message("updated", "");
		}
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
