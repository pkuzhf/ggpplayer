#define CHECK_STATEMACHINE
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "reader.h"
#include "relation.h"
#include "prover.h"
#include "domaingraph.h"
#include  "dependgraph.h"
#include "statemachine.h"

using namespace std;

int main() {
#ifdef CHECK_PROVER
	
	Reader r;
	if (r.scanGDLFile(".\\gdl\\2pffa_zerosum.kif");
	Relations rs;
	r.getRelations(rs);
	Prover prover(rs);
		
	Relations initialState = prover.getInitStateByDPG();	
	string state = prover.getInitState();
	
	while(!prover.askTerminal(state)){
		string does1 = prover.askLegalActions(0, state);
		string does2 = prover.askLegalActions(1, state);
		string does;
		for(int i = 0; i < does1.size(); ++i){
			does += '0';
		}
	
		does[does1.find('1')] = '1';
		does[does2.find('1')] = '1';
		state = prover.askNextState(state, does);
		
		for(int i = 0; i < state.size(); ++i){
			if(state[i] == '1'){
				cout << prover.keyrelations_[i].toString() <<endl;
			}
		}
		vector<int> result;
		prover.askGoal(result, state);
		for(int i = 0; i < result.size(); ++i){
			cout << "player" << i << ": "<< result[i]<<endl;
		}
	}
#endif

#ifdef CHECK_STATEMACHINE
	
	Reader r;
	if (!r.scanGDLFile("gdl/rule.txt")) {
        cout << "read file failed." << endl;
        return -1;
    }
	Relations rs;
	r.getRelations(rs);
	StateMachine machine(rs);

	char buf[10000];
	cin.getline(buf, 10000);
	string role;	
	role = buf;
	cout << "ready" << endl;
		
	cin.getline(buf, 10000);
	if (machine.getRandomMove(role).items_[1].items_.size() == 0) {		
		cout << machine.getRandomMove(role).items_[1].toString() << endl;
	} else {
		cout << "( " << machine.getRandomMove(role).items_[1].toString() << " )" << endl;
	}
	while (true) {				
		cin.getline(buf, 10000);	
		Reader move_reader;
		move_reader.file_content_ = buf;
		Relations joint_move;
		move_reader.getMoves(joint_move);
		for (int i = 0; i < joint_move.size(); ++i) {
			Relation does;
			does.content_ = "does";
			does.type_ = r_does;			
			does.items_.push_back(machine.prover_.roles_[i].items_[0]);
			does.items_.push_back(joint_move[i]);
			joint_move[i] = does;
		}
		machine.goOneStep(joint_move);
		if (machine.isTerminal()) {
			//cout << "Terminal." << endl;
			break;
		}
		if (machine.getRandomMove(role).items_[1].items_.size() == 0) {		
			cout << machine.getRandomMove(role).items_[1].toString() << endl;
		} else {
			cout << "( " << machine.getRandomMove(role).items_[1].toString() << " )" << endl;
		}
	}

	//Relations goals = machine.randomGo();
	

#endif

	return 0;
}
