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
	Relations rs;
	rs.insert(rs.end(), prover_.inits_.begin(), prover_.inits_.end());
	rs.insert(rs.end(), prover_.statics_.begin(), prover_.statics_.end());	
	right_props_ = prover_.generateTrueProps(rs, 0, prover_.dpg_.stra_deriv_.size() - 1);
	current_state_.clear();
	for(int i = 0  ; i < right_props_.size(); ++i){
		if(right_props_[i].type_ == r_init){
			Relation r = right_props_[i];
			r.type_ = r_true;
			r.content_ = r_true;
			current_state_.push_back(r);
		}
	}
	rs.clear();
	rs.insert(rs.end(), current_state_.begin(), current_state_.end());	
	right_props_ = prover_.generateTrueProps(rs, 0, prover_.dpg_.legal_level_);
}

Relations StateMachine::getGoals()
{
	Relations rtn;
	for(int i = 0 ; i < right_props_.size(); ++i){
		if(right_props_[i].type_ == r_goal){
			rtn.push_back(right_props_[i]);
		}
	}
	return rtn;
}

bool StateMachine::isTerminal() {
	
	for(int i = 0 ; i < right_props_.size(); ++i){
		if(right_props_[i].type_ == r_terminal){
			return true;
		}
	}
	return false;
}

Relations StateMachine::getLegalMoves(int role) {
	
	Relations rtn;
	for (int i = 0; i < prover_.statics_.size(); ++i) {
		if(prover_.statics_[i].type_ == r_legal && prover_.statics_[i].items_[0].content_ == role){
			Relation r = prover_.statics_[i];
			r.content_ = r_does;
			r.type_ = r_does;
			rtn.push_back(r);
		}
	}
	for(int i = 0 ; i < right_props_.size(); ++i){
		if(right_props_[i].type_ == r_legal && right_props_[i].items_[0].content_ == role){
			Relation r = right_props_[i];
			r.content_ = r_does;
			r.type_ = r_does;
			rtn.push_back(r);
		}
	}
	return rtn;
}

Relation StateMachine::getRandomMove(int role) {
	Relations moves = getLegalMoves(role);
	srand((unsigned)time(NULL));  
	if (moves.size() == 0) {
		cout << "No legal move." <<endl;
	}
	return moves[rand() % moves.size()];
}

void StateMachine::goOneStep(Relations & move)
{
	Relations rs;
	rs.insert(rs.end(), move.begin(), move.end());	
	rs.insert(rs.end(), right_props_.begin(), right_props_.end());
	for (int i = 0; i < rs.size(); ++i) {
		rs[i].s_ = rs[i].toString();
	}

	right_props_ = prover_.generateTrueProps(rs, prover_.dpg_.legal_level_ + 1, prover_.dpg_.stra_deriv_.size() - 1);

	for (int i = 0; i < right_props_.size(); ++i) {
		right_props_[i].s_ = right_props_[i].toString();
	}
	if (isTerminal()) {
		return;
	}
	current_state_.clear();
	for(int i = 0  ; i < right_props_.size(); ++i){
		if(right_props_[i].type_ == r_next){
			Relation r = right_props_[i];
			r.type_ = r_true;
			r.content_ = r_true;
			current_state_.push_back(r);
		}
	}

	rs.clear();
	rs.insert(rs.end(), current_state_.begin(), current_state_.end());	
	right_props_ = prover_.generateTrueProps(rs, 0, prover_.dpg_.legal_level_);
	for (int i = 0; i < right_props_.size(); ++i) {
		right_props_[i].s_ = right_props_[i].toString();
	}
}

Relations StateMachine::randomGo()
{
	clock_t begin = clock();
	int count = 0;
	while (!isTerminal()) {
		count ++;
		Relations joint_move;
		for (int i = 0; i < prover_.roles_.size(); ++i) {
			Relation role = prover_.roles_[i];
			joint_move.push_back(getRandomMove(role.items_[0].content_));
		}
		cout << joint_move[0].toString() << endl;
		cout << joint_move[1].toString() << endl;
		goOneStep(joint_move);		
	}
	clock_t end = clock();
	cout<< count <<" steps in "<<end - begin<< " ms" <<endl;
	return getGoals();
}
