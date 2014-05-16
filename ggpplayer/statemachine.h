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
	Relations getGoals( Relations &state);
	bool isTerminal(Relations &state);
	Relations getInitialState();
	Relations getLegalMoves(Relations &state, Relation role);
	//bool getLegalMoves(Moves &moves, const State &state);
	Relations getNextState(Relations &state, Relations &moves);
	Relation getRandomMove(Relations &state, Relation role);
	int getRoleSum();
	Prover prover_;
	void setState(Relations & state);
	Relations right_props_;
private:
	static const int cache_size_ = 1000;//Ö»±£´æ1000¸östate
	Relations current_state_;
	Relations initial_state_;
	int role_n_;
	Cache cache_;
};

#endif