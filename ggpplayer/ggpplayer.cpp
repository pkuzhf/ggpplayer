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

using namespace std;

int main() {
	srand(time(0));
	for(int i = 0 ; i < relation_type_num; ++i){		
		Relation::addSymbol(relation_type_words[i]);
	}
	Reader r;
	//if (!r.scanGDLFile("rule.txt")) {
	//if (!r.scanGDLFile("gdl/connect_four.txt")) {
	if (!r.scanGDLFile("gdl/quarto.kif")) {
        cout << "read file failed." << endl;
        return -1;
    }
	Relations rs;
	r.getRelations(rs);
	StateMachine machine(rs);
	machine.randomGo(clock() + 100000);
	//cout << "generate: " << Prover::generate_time << endl;
	cout << "time1: " << Prover::time1 << endl;
	cout << "time2: " << Prover::time2 << endl;
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
	char buf[10000];
	cin.getline(buf, 10000);
	int role, runtime;
	role = Relation::symbol2code[string(buf)];
	cin.getline(buf, 10000);
	runtime = (atoi(buf) - 2) * CLOCKS_PER_SEC;
	MonteCarloPlayer Mplayer(rs, role);   // montecarlo player
	

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
	

	cout << "ready" << endl;
		
	cin.getline(buf, 10000);
	cout << Mplayer.stateMachineSelectMove(runtime).items_[1].toString() << endl;
	
	while (true) {				
		cin.getline(buf, 10000);	
		Reader move_reader;
		move_reader.file_head_ = buf;		
		Propositions joint_move;
		move_reader.getMoves(joint_move);
		for (int i = 0; i < joint_move.size(); ++i) {
			Relation does;
			does.head_ = r_does;						
			does.items_.push_back(Mplayer.stateMachine_.prover_.roles_[i].toRelation().items_[0]);
			does.items_.push_back(joint_move[i].toRelation());
			joint_move[i] = does.toProposition();
		}		
		Mplayer.goOneStep(joint_move);
		if (Mplayer.is_terminal_) {
			//cout << "Terminal." << endl;
			break;
		}
		Proposition move = Mplayer.stateMachineSelectMove(runtime);
		cout << move.items_[1].toString() << endl;
	}
	return 0;
}
