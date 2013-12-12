#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <time.h>

#include "statemachine.h"
#include "prover.h"
#include "reader.h"
#include "tools.h"

using namespace std;


StateMachine::StateMachine(Relations description):prover_(description), cache_(cache_size_) {
	initial_state_ = prover_.getInitState();
	role_n_ = prover_.roles_.size();
}

bool StateMachine::getGoals(Goals &result, const string &state) {
	return prover_.askGoal(result, state);
}

bool StateMachine::isTerminal(const string &state) {
	Data *data = cache_.get(state);
	if (data == NULL) {
		data = new Data();
		if (prover_.askTerminal(state)) {
			data->terminal_ = 1;
		} else {
			data->terminal_ = -1;
		}
		cache_.put(state, *data);// TODO
	} else if (data->terminal_ == 0) {
		if (prover_.askTerminal(state)) {
			data->terminal_ = 1;
		} else {
			data->terminal_ = -1;
		}
	}
	return data->terminal_ == 1;
}

string StateMachine::getInitialState() {
	return initial_state_;
}


bool StateMachine::getLegalMoves(Moves &moves, const string &state, Role role) {
	string ret;
	Data *data = cache_.get(state);
	if (data == NULL) {
		data = new Data();
		ret = prover_.askLegalActions(role, state);
		data->legalActions_.insert(pair<Role, string>(role, ret));
		cache_.put(state, *data);// TODO
	} else {
		map<Role, string>::iterator i = (data->legalActions_).find(role);
		if (i == data->legalActions_.end()) {
			ret = prover_.askLegalActions(role, state);
			data->legalActions_.insert(pair<Role, string>(role, ret));
		} else {
			ret = i->second;
		}
	}
	for (int i = 0; i < ret.size(); ++i) {
		if (ret[i] == '1')
			moves.push_back(i);
	}
	return true;
}

/*
Moves StateMachine::getLegalMoves(const State &state) {
	return prover_.askLegalActions(state);
}
*/

string StateMachine::getNextState(const string &state, const Moves &moves) {
	string ret;
	string moveString(prover_.legalactions_.size(), '0');
	for (int i = 0; i < moves.size(); ++i) {
		moveString[moves[i]] = '1';
	}
	Data *data = cache_.get(state);
	if (data == NULL) {
		data = new Data();
		ret = prover_.askNextState(state, moveString);
		data->nextState_.insert(pair<string, string>(moveString, ret));
		cache_.put(state, *data);// TODO
	} else {
		map<string, string>::iterator i = (data->nextState_).find(ret);
		if (i == data->nextState_.end()) {
			ret = prover_.askNextState(state, moveString);
			data->nextState_.insert(pair<string, string>(moveString, ret));
		} else {
			ret = i->second;
		}
	}
	return ret;
}

int StateMachine::getRandomMove(const string &state, Role role) {
	Moves moves;
	getLegalMoves(moves, state, role);
	srand((unsigned)time(NULL));  
	return moves[rand() % moves.size()];
}