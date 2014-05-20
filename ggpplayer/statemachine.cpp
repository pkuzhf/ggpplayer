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


StateMachine::StateMachine(Relations description):prover_(description), cache_(cache_size_) {	
	Relations rs;
	rs.insert(rs.end(), prover_.inits_.begin(), prover_.inits_.end());
	rs.insert(rs.end(), prover_.statics_.begin(), prover_.statics_.end());	
	right_props_ = prover_.generateTrueProps(rs);
	current_state_.clear();
	for(int i = 0  ; i < right_props_.size(); ++i){
		if(right_props_[i].type_ == r_init){
			Relation r = right_props_[i];
			r.type_ = r_true;
			r.content_ = "true";
			current_state_.push_back(r);
		}
	}
	rs.clear();
	rs.insert(rs.end(), current_state_.begin(), current_state_.end());
	rs.insert(rs.end(), prover_.statics_.begin(), prover_.statics_.end());	
	right_props_ = prover_.generateTrueProps(rs);
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

Relations StateMachine::getLegalMoves(string role) {
	
	Relations rtn;
	for(int i = 0 ; i < right_props_.size(); ++i){
		if(right_props_[i].type_ == r_legal && right_props_[i].items_[0].content_ == role){
			Relation r = right_props_[i];
			r.content_ = "does";
			r.type_ = r_does;
			rtn.push_back(r);
		}
	}
	return rtn;
}

Relation StateMachine::getRandomMove(string role) {
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
	rs.insert(rs.end(), current_state_.begin(), current_state_.end());
	rs.insert(rs.end(), prover_.statics_.begin(), prover_.statics_.end());	
	right_props_ = prover_.generateTrueProps(rs);
	current_state_.clear();
	for(int i = 0  ; i < right_props_.size(); ++i){
		if(right_props_[i].type_ == r_next){
			Relation r = right_props_[i];
			r.type_ = r_true;
			r.content_ = "true";
			current_state_.push_back(r);
		}
	}
	rs.clear();
	rs.insert(rs.end(), current_state_.begin(), current_state_.end());
	rs.insert(rs.end(), prover_.statics_.begin(), prover_.statics_.end());	
	right_props_ = prover_.generateTrueProps(rs);
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
		goOneStep(joint_move);		
	}
	clock_t end = clock();
	cout<< count <<" steps in "<<end - begin<< " ms" <<endl;
	return getGoals();
}
