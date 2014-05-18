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

Relations StateMachine::getGoals()
{
	Relations rtn;
	for(int i = 0 ; i < right_props_.size(); ++i){
		if(right_props_[i].type_ == RelationType::r_goal){
			rtn.push_back(right_props_[i]);
		}
	}
	return rtn;
}

bool StateMachine::isTerminal() {
	
	for(int i = 0 ; i < right_props_.size(); ++i){
		if(right_props_[i].type_ == RelationType::r_terminal){
			return true;
		}
	}
	return false;
}



Relations StateMachine::getInitialState() {
	return initial_state_;
}


Relations StateMachine::getLegalMoves(Relation role) {
	
	Relations rtn;
	for(int i = 0 ; i < right_props_.size(); ++i){
		if(right_props_[i].type_ == RelationType::r_legal && right_props_[i].items_[0].content_ == role.items_[0].content_){
			Relation r = right_props_[i];
			r.content_ = "does";
			r.type_ = RelationType::r_does;
			rtn.push_back(r);
		}
	}
	return rtn;
}



Relations StateMachine::getNextState( Relations &moves) {
	Relations true_rs;
	for(int i = 0 ;  i < right_props_.size(); ++i){
		true_rs.push_back(right_props_[i]);
	}
	for(int i = 0 ;  i < moves.size(); ++i){
		true_rs.push_back(moves[i]);
	}
	true_rs = prover_.generateTrueProps(true_rs);
	Relations rtn;
	for(int i = 0 ; i < true_rs.size(); ++i){
		if(true_rs[i].type_ == RelationType::r_next){
			Relation r = true_rs[i];
			r.content_ = "true";
			r.type_ = RelationType::r_true;
			rtn.push_back(r);
		}
	}
	return rtn;
}

Relation StateMachine::getRandomMove(Relation role) {
	Relations moves = getLegalMoves(role);
	srand((unsigned)time(NULL));  
	return moves[rand() % moves.size()];
}

int StateMachine::getRoleSum() {
	return role_n_;
}

void StateMachine::setState(Relations & state)
{
	Relations rs;
	current_state_.clear();
	for(int i = 0  ; i < state.size(); ++i){
		current_state_.push_back(state[i]);
		rs.push_back(state[i]);
	}
	for(int i = 0 ; i < prover_.statics_.size(); ++i){
		rs.push_back(prover_.statics_[i]);
	}
	right_props_ = prover_.generateTrueProps(rs);
}

Relations StateMachine::randomGo()
{
	int count = 0;
	clock_t begin, end;
	begin = clock();
	while (!isTerminal()) {
		count ++;
		Relations moves;
		for (int i = 0; i < getRoleSum(); ++i) {
			Relation role = prover_.roles_[i];
			moves.push_back(getRandomMove(role));
		}
		setState(getNextState(moves));
	}
	end = clock();
	cout<< count << "steps in "<< end - begin<<" ms"<<endl;
	return getGoals();
}