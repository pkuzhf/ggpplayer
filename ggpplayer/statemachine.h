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


typedef int Role;
typedef vector<int> Moves;
typedef vector<int> Goals;

class StateMachine {
public:
	StateMachine(Relations description);
	bool getGoals(Goals &result, const string &state);
	bool isTerminal(const string &state);
	string getInitialState();
	bool getLegalMoves(Moves &moves, const string &state, Role role);
	//bool getLegalMoves(Moves &moves, const State &state);
	string getNextState(const string &state, const Moves &moves);
	int getRandomMove(const string &state, Role role);

private:
	static const int cache_size_ = 1000;//Ö»±£´æ1000¸östate
	Prover prover_;
	string initial_state_;
	int role_n_;
	Cache cache_;
};

#endif