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
#include "propnet.h"

using namespace std;

class StateMachine {

//#ifdef PROVER
//private:
//	Prover prover_;	
//	Propositions trues_;
//	Propositions legals_;
//	Propositions static_legals_;
//	Propositions goals_;
//	Propositions tmps_;
//	bool is_terminal_;
//	Propositions roles_;
//public:
//	StateMachine(){}
//	StateMachine(Relations description);	
//	void updateState(Propositions &ps);
//	void updateLegals(Propositions &ps);
//	Propositions getLegalMoves(int role);	
//	Proposition getRandomMove(int role);	
//	void goOneStep(Propositions & move);		
//	bool randomGo(clock_t time_limit);
//	int getGoal(int role);
//	void setState(Propositions &currentState);
//	vector<vector<Proposition>> getLegalJointMoves(int role, int mymove);
//	Propositions getState();
//	Propositions getRoles();
//	bool isTerminal();
//#else
private:
	Propnet propnet_;
	Prover prover_;	
	Propositions static_legals_;
	Propositions roles_;
public:
	StateMachine(){}
	StateMachine(Relations description);	
	Propositions getLegalMoves(int role);	
	Proposition getRandomMove(int role);	
	void goOneStep(Propositions & move);		
	bool randomGo(clock_t time_limit);
	int getGoal(int role);
	void setState(Propositions &currentState);
	vector<vector<Proposition>> getLegalJointMoves(int role, int mymove);
	Propositions getState();
	Propositions getRoles();
	bool isTerminal();
//#endif
};

#endif
