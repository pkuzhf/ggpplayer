#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "prover.h"
#include "relation.h"

using namespace std;

class StateMachine {
public:
	Prover prover_;	
	Propositions trues_;
	Propositions legals_;
	Propositions static_legals_;
	Propositions goals_;
	Propositions tmps_;
	bool is_terminal_;

	StateMachine(Relations description);		
	void updateState(Propositions &ps);
	Propositions getLegalMoves(int role);	
	Proposition getRandomMove(int role);	
	void goOneStep(Propositions & move);	
	Propositions randomGo();
	int getGoal(int role);
	void setState(Propositions &currentState);
	vector<vector<Proposition>> getLegalJointMoves(int role, Proposition mymove);
	
};

#endif
