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
#include "client.h"
#include "config.h"

using namespace std;

#ifdef CLIENT

int main() {
	Client client;
	client.connectServer();
}

#else

int main() {
	srand(time(0));
	Relation::initSymbolTable();
	Reader r;
	if (!r.readFile("gdl/rule.txt")) {
	//if (!r.readFile("gdl/tic_tac_toe.txt")) {
	//if (!r.readFile("gdl/2pffa_zerosum.kif")) {
		cerr << Client::message("debug", "read file failed.");
        return -1;
    }
	Relations rs;
	r.getRelations(rs);
	StateMachine machine(rs);
	//machine.randomGo(clock() + 100000);

	const int buf_size = 100000;
	char buf[buf_size];
	cin.getline(buf, buf_size);
	//r.readLine(buf);
	//Relations rs;
	//r.getRelations(rs);
	//StateMachine machine(rs);

	/*for (int i = 0; i < Prover::time.size(); ++i) {
		ostringstream msg;
		msg << "time" << i << ": " << Prover::time[i];
		cerr << Client::message("debug", msg.str());
	}*/

    cin.getline(buf, buf_size);
	string role = string(buf);

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
		cerr<<"real moves:"<< moves[0].toString()<<endl;
		cerr<<"real moves:"<< moves[1].toString()<<endl;
	}*/	
		
	cin.getline(buf, buf_size);
	cerr << Client::message("state", Proposition::propsToStr(player.current_state_));
	cerr << Client::message("move", player.getRandomMove().items_[1].toString());

	while (true) {				
		cin.getline(buf, buf_size);
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
			//cerr << move.items_[1].toString() << endl;
			cerr << Client::message("move", player.getRandomMove().items_[1].toString());
			cerr << Client::message("state", Proposition::propsToStr(player.selectLeafNode()->state_));
		} else if (cmd == "client") {
			char * semi = strstr(space + 1, ";");
			if (semi == NULL) continue;
			*semi = '\0';
			Reader state_reader;
			state_reader.file_content_ = string(space + 1);
			Propositions state;
			state_reader.getPropositions(state);			
			player.updateTree(state, string(semi + 1));
			cerr << Client::message("move", player.getBestMove().items_[1].toString());
			cerr << Client::message("state", Proposition::propsToStr(player.selectLeafNode()->state_));
		}
	}
	return 0;
}

#endif
