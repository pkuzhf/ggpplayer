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
#include "cache.h"

using namespace std;




class StateMachine {
public:

public:
	StateMachine(Relations description);
	Relations getGoals();
	bool isTerminal();
	Relations getInitialState();
	Relations getLegalMoves(string role);
	//bool getLegalMoves(Moves &moves, const State &state);	
	Relation getRandomMove(string role);
	Prover prover_;
	void goOneStep(Relations & move);
	Relations right_props_;
	Relations randomGo();
private:
	static const int cache_size_ = 1000;//ֻ����1000��state
	Relations current_state_;	
	Cache cache_;
};

#endif
