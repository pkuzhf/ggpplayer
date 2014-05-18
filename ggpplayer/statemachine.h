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
	Relations getLegalMoves(Relation role);
	//bool getLegalMoves(Moves &moves, const State &state);
	Relations getNextState(Relations &moves);
	Relation getRandomMove( Relation role);
	int getRoleSum();
	Prover prover_;
	void setState(Relations & state);
	Relations right_props_;
	Relations randomGo();
private:
	static const int cache_size_ = 1000;//Ö»±£´æ1000¸östate
	Relations current_state_;
	Relations initial_state_;
	int role_n_;
	Cache cache_;
};

#endif