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
typedef Relation Role;
typedef Relations Roles;
typedef string Move;
typedef string Moves;// 应该是数组形式吧？
typedef Relations Goals;

class StateMachine {
public:
	StateMachine(Relations description);
	Goals getGoal(const State &state);// TODO
	bool isTerminal(const State &state);
	State getInitialState();
	Moves getLegalMoves(const State &state, Role role);
	Moves getLegalMoves(const State &state);//TODO
	State getNextState(const State &state, const Moves &moves);

private:
	static const int cache_size_ = 1000;//只保存1000个state
	Prover prover_;
	State initial_state_;
	Roles roles_;
	Cache cache_;
};

#endif