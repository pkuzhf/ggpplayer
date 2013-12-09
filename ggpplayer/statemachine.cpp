#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "statemachine.h"
#include "prover.h"
#include "tools.h"

using namespace std;

StateMachine::StateMachine(Relations description):prover_(description),cache_(cache_size_) {
	initial_state_ = prover_.getInitState();
	roles_ = prover_.roles_;
}

// TODO:
Goals StateMachine::getGoal(const State &state) {
	Goals goals;
	//prover_.askGoal(goals, state);// 传string还是state，需要转？
	return goals;
}

bool StateMachine::isTerminal(const State &state) {
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

State StateMachine::getInitialState() {
	return initial_state_;
}


Moves StateMachine::getLegalMoves(const State &state, Role role) {
	Moves ret;
	Data *data = cache_.get(state);
	if (data == NULL) {
		data = new Data();
		ret = prover_.askLegalActions(prover_.askRole(role), state);
		data->legalActions_.insert(pair<Role, Moves>(role, ret));
		cache_.put(state, *data);// TODO
	} else {
		map<Role, Moves>::iterator i = (data->legalActions_).find(role);
		if (i == data->legalActions_.end()) {
			ret = prover_.askLegalActions(prover_.askRole(role), state);
		} else {
			ret = i->second;
		}
	}
	return ret;
}

// TODO
Moves StateMachine::getLegalMoves(const State &state) {
	return prover_.askLegalActions(state);
}

State StateMachine::getNextState(const State &state, const Move &moves) {
	State ret;
	Data *data = cache_.get(state);
	if (data == NULL) {
		data = new Data();
		ret = prover_.askNextState(state, moves);
		data->nextState_.insert(pair<Moves, State>(moves, ret));
		cache_.put(state, *data);// TODO
	} else {
		map<Moves, State>::iterator i = (data->nextState_).find(moves);
		if (i == data->nextState_.end()) {
			ret = prover_.askNextState(state, moves);
		} else {
			ret = i->second;
		}
	}
	return ret;
}