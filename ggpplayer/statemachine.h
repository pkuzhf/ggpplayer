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

public:
	StateMachine(Relations description);
	Propositions getGoals();
	bool isTerminal();
	Propositions getInitialState();
	Propositions getLegalMoves(int role);
	//bool getLegalMoves(Moves &moves, const State &state);	
	Proposition getRandomMove(int role);
	Prover prover_;
	void goOneStep(Propositions & move);
	Propositions right_props_;
	Propositions randomGo();
private:
	static const int cache_size_ = 1000;//ֻ����1000��state
	Propositions current_state_;	
};

#endif
