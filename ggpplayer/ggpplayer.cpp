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

	//r.scanGDLFile("gdl\\lights_out.txt");
	r.scanGDLFile("gdl\\2pffa_zerosum.kif");

	Relations rs;
	r.getRelations(rs);
	StateMachine machine(rs);

	Relations state = machine.getInitialState();
	
	while (!machine.isTerminal(state)) {
		Relations moves;
		for (int i = 0; i < machine.getRoleSum(); ++i) {
			Relation role = machine.prover_.roles_[i];
			moves.push_back(machine.getRandomMove(state, role));
		}
		state = machine.getNextState(state, moves);
	}
	Relations goals = machine.getGoals(state);
	

#endif

	return 0;
}
