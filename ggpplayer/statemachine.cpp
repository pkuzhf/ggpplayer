#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <time.h>
#include <stdlib.h>

#include "statemachine.h"
#include "prover.h"
#include "reader.h"
#include "relation.h"

using namespace std;


StateMachine::StateMachine(Relations description):prover_(description) {	
	Propositions rs;
	rs.insert(rs.end(), prover_.inits_.begin(), prover_.inits_.end());
	rs.insert(rs.end(), prover_.statics_.begin(), prover_.statics_.end());	
	right_props_ = prover_.generateTrueProps(rs, 0, prover_.dpg_.stra_deriv_.size() - 1);
	current_state_.clear();
	for(int i = 0  ; i < right_props_.size(); ++i){
		if(right_props_[i].head() == r_init){
			Proposition r = right_props_[i];
			r.setHead(r_true);
			current_state_.push_back(r);
		}
	}
	rs.clear();
	rs.insert(rs.end(), current_state_.begin(), current_state_.end());	
	right_props_ = prover_.generateTrueProps(rs, 0, prover_.dpg_.legal_level_);
}

Propositions StateMachine::getGoals()
{
	Propositions rtn;
	for(int i = 0 ; i < right_props_.size(); ++i){
		if(right_props_[i].head() == r_goal){
			rtn.push_back(right_props_[i]);
		}
	}
	return rtn;
}

bool StateMachine::isTerminal() {
	
	for(int i = 0 ; i < right_props_.size(); ++i){
		if(right_props_[i].head() == r_terminal){
			return true;
		}
	}
	return false;
}

Propositions StateMachine::getLegalMoves(int role) {
	
	Propositions rtn;
	for (int i = 0; i < prover_.statics_.size(); ++i) {
		if(prover_.statics_[i].head() == r_legal && prover_.statics_[i].toRelation().items_[0].content_ == role){
			Proposition r = prover_.statics_[i];
			r.setHead(r_does);
			rtn.push_back(r);
		}
	}
	for(int i = 0 ; i < right_props_.size(); ++i){
		if(right_props_[i].head() == r_legal && right_props_[i].toRelation().items_[0].content_ == role){
			Proposition r = right_props_[i];
			r.setHead(r_does);
			rtn.push_back(r);
		}
	}
	return rtn;
}

Proposition StateMachine::getRandomMove(int role) {
	Propositions moves = getLegalMoves(role);
	srand((unsigned)time(NULL));  
	if (moves.size() == 0) {
		cout << "No legal move." <<endl;
	}
	return moves[rand() % moves.size()];
}

void StateMachine::goOneStep(Propositions & move)
{
	Propositions rs;
	rs.insert(rs.end(), move.begin(), move.end());	
	rs.insert(rs.end(), right_props_.begin(), right_props_.end());	
	right_props_ = prover_.generateTrueProps(rs, prover_.dpg_.legal_level_ + 1, prover_.dpg_.stra_deriv_.size() - 1);
	
	if (isTerminal()) {
		return;
	}
	current_state_.clear();
	for(int i = 0  ; i < right_props_.size(); ++i){
		if(right_props_[i].head() == r_next){
			Proposition r = right_props_[i];
			r.setHead(r_true);
			current_state_.push_back(r);
		}
	}

	rs.clear();
	rs.insert(rs.end(), current_state_.begin(), current_state_.end());	
	right_props_ = prover_.generateTrueProps(rs, 0, prover_.dpg_.stra_deriv_.size() - 1);	
}

Propositions StateMachine::randomGo()
{
	clock_t begin = clock();
	int count = 0;
	while (!isTerminal()) {
		count ++;
		Propositions joint_move;
		for (int i = 0; i < prover_.roles_.size(); ++i) {
			Proposition role = prover_.roles_[i];
			joint_move.push_back(getRandomMove(role.toRelation().items_[0].content_));
		}
		cout << joint_move[0].toRelation().toString() << endl;
		cout << joint_move[1].toRelation().toString() << endl;
		goOneStep(joint_move);		
	}
	clock_t end = clock();
	cout<< count <<" steps in "<<end - begin<< " ms" <<endl;
	return getGoals();
}
