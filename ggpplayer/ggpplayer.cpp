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
#include "tools.h"

using namespace std;

int main() {
	Reader r;
	r.scanGDLFile("gdl\\tic_tac_toe.txt");
	Relations rs;
	r.getRelations(rs);
	Prover prover(rs);
	
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
		
	}
	
	return 0;
}
