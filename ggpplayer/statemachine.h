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


typedef string State;// TODO
typedef int Role;
typedef vector<int> Roles;
typedef vector<int> Moves;
typedef vector<int> Goals;

class StateMachine {
public:
	StateMachine(Relations description);
	bool getGoal(Goal &result, const State &state);
	bool isTerminal(const State &state);
	State getInitialState();
	bool getLegalMoves(Moves &moves, const State &state, Role role);
	//bool getLegalMoves(Moves &moves, const State &state);
	State getNextState(const State &state, const Moves &moves);

private:
	static const int cache_size_ = 1000;//Ö»±£´æ1000¸östate
	Prover prover_;
	State initial_state_;
	Roles roles_;
	Cache cache_;
};

#endif