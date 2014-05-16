#define CHECK_PROVER
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
#include "tools.h"
#include "statemachine.h"

using namespace std;

int main() {

#ifdef CHECK_PROVER
	
	Reader r;
	r.scanGDLFile("gdl\\2pffa_zerosum.kif");

	Relations rs;
	r.getRelations(rs);
	Prover prover(rs);
		
	prover.getInitStateByDPG();	
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

	//r.scanGDLFile("gdl\\lights_out.txt");
	r.scanGDLFile("gdl\\tic_tac_toe.txt");

	Relations rs;
	r.getRelations(rs);
	StateMachine machine(rs);

	string state = machine.getInitialState();
	cout << "state:" << endl;
	cout << "	" << state << endl;
	while (!machine.isTerminal(state)) {
		Moves moves;
		for (int i = 0; i < machine.getRoleSum(); ++i) {
			moves.push_back(machine.getRandomMove(state, i));
		}
		state = machine.getNextState(state, moves);
		cout << "	" << state << endl;
	}
	Goals goals;
	machine.getGoals(goals, state);
	cout << "score:" << endl;
	for (int i = 0; i < machine.getRoleSum(); ++i) {
		cout << "	player" << i << ":" << goals.at(i) << endl;
	}

#endif

	return 0;
}
