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
	initial_state_ = prover_.getInitStateByDPG();
	role_n_ = prover_.roles_.size();
}

Relations StateMachine::getGoals(Relations &state)
{
	for(int i = 0 ;  i < prover_.statics_.size(); ++i){
		state.push_back(prover_.statics_[i]);
	}
	Relations true_rs = prover_.generateTrueProps(state);
	Relations rtn;
	for(int i = 0 ; i < true_rs.size(); ++i){
		if(true_rs[i].type_ == RelationType::r_goal){
			rtn.push_back(true_rs[i]);
		}
	}
	return rtn;
}

bool StateMachine::isTerminal(Relations &state) {
	for(int i = 0 ;  i < prover_.statics_.size(); ++i){
		state.push_back(prover_.statics_[i]);
	}
	Relations true_rs = prover_.generateTrueProps(state);
	for(int i = 0 ; i < true_rs.size(); ++i){
		if(true_rs[i].type_ == RelationType::r_terminal){
			return true;
		}
	}
	return false;
}

Relations StateMachine::getInitialState() {
	return initial_state_;
}


Relations StateMachine::getLegalMoves( Relations &state, Relation role) {
	for(int i = 0 ;  i < prover_.statics_.size(); ++i){
		state.push_back(prover_.statics_[i]);
	}
	Relations true_rs = prover_.generateTrueProps(state);
	Relations rtn;
	for(int i = 0 ; i < true_rs.size(); ++i){
		if(true_rs[i].type_ == RelationType::r_legal && true_rs[i].items_[0].content_ == role.items_[0].content_){
			Relation r = true_rs[i];
			r.content_ = "does";
			r.type_ = RelationType::r_does;
			rtn.push_back(r);
		}
	}
	return rtn;
}



Relations StateMachine::getNextState(Relations &state, Relations &moves) {
	Relations true_rs;
	for(int i = 0 ;  i < prover_.statics_.size(); ++i){
		true_rs.push_back(prover_.statics_[i]);
	}
	for(int i = 0 ;  i < state.size(); ++i){
		true_rs.push_back(state[i]);
	}
	for(int i = 0 ;  i < moves.size(); ++i){
		true_rs.push_back(moves[i]);
	}
	true_rs = prover_.generateTrueProps(true_rs);
	Relations rtn;
	for(int i = 0 ; i < true_rs.size(); ++i){
		if(true_rs[i].type_ == RelationType::r_next){
			rtn.push_back(true_rs[i]);
		}
	}
	return rtn;
}

Relation StateMachine::getRandomMove(Relations &state, Relation role) {
	Relations moves = getLegalMoves(state, role);
	srand((unsigned)time(NULL));  
	return moves[rand() % moves.size()];
}

int StateMachine::getRoleSum() {
	return role_n_;
}