#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <time.h>
#include <stdlib.h>
#include <algorithm>

#include "statemachine.h"
#include "prover.h"
#include "reader.h"
#include "relation.h"
#include "client.h"

using namespace std;


StateMachine::StateMachine(Relations description) :prover_(description) { 
	is_terminal_ = false;
	for (int i = 0; i < prover_.statics_.size(); ++i) {
		if (prover_.statics_[i].head_ == r_legal) {
			static_legals_.push_back(prover_.statics_[i]);
		}
	}
	sort(static_legals_.begin(), static_legals_.end());
	for (int i = 0; i < prover_.inits_.size(); ++i){
		Proposition p = prover_.inits_[i];
		p.head_ = r_true;
		trues_.push_back(p);		
	}
	sort(trues_.begin(), trues_.end());
	Propositions ps = trues_;
	prover_.generateTrueProps(ps, 0, prover_.dpg_.legal_level_);
	updateLegals(ps);
}

void StateMachine::updateState(Propositions &ps) {
	trues_.clear();
	legals_.clear();
	goals_.clear();
	tmps_.clear();
	for (int i = 0; i < ps.size(); ++i) {		
		if (ps[i].head_ == r_next) {
			Proposition p = ps[i];
			p.head_ = r_true;
			trues_.push_back(p);
		}
	}
	sort(trues_.begin(), trues_.end());
}

void StateMachine::updateLegals(Propositions &ps) {		
	for (int i = 0; i < ps.size(); ++i) {
		Proposition &p = ps[i];
		if (p.head_ == r_legal) {
			legals_.push_back(p);
		} else if (p.head_ == r_goal) {
			goals_.push_back(p);
		} else if (p.head_ == r_terminal) {
			is_terminal_ = true;
		} else if (p.head_ != r_true) {
			tmps_.push_back(p);
		}
	}
	sort(legals_.begin(), legals_.end());
}

Propositions StateMachine::getLegalMoves(int role) {	
	Propositions rtn;	
	for(int i = 0 ; i < legals_.size(); ++i){
		if (legals_[i].items_[0].head_ == prover_.roles_[role].items_[0].head_) {
			Proposition p = legals_[i];
			p.head_ = r_does;
			rtn.push_back(p);
		}
	}
	for (int i = 0; i < static_legals_.size(); ++i) {
		if (static_legals_[i].items_[0].head_ == prover_.roles_[role].items_[0].head_) {
			Proposition p = static_legals_[i];
			p.head_ = r_does;
			rtn.push_back(p);
		}
	}
	return rtn;
}

Proposition StateMachine::getRandomMove(int role) {
	Propositions moves = getLegalMoves(role);
	srand((unsigned)time(NULL));  
	if (moves.size() == 0) {
		cerr << Client::message("debug", "No legal move.");
	}
	return moves[rand() % moves.size()];
}

void StateMachine::goOneStep(Propositions & move)
{
	Propositions ps;
	ps.insert(ps.end(), move.begin(), move.end());	
	ps.insert(ps.end(), tmps_.begin(), tmps_.end());
	ps.insert(ps.end(), trues_.begin(), trues_.end());
	prover_.generateTrueProps(ps, prover_.dpg_.legal_level_ + 1, prover_.dpg_.stra_deriv_.size() - 1);
	
	updateState(ps);

	if (is_terminal_) {
		return; // impossible
	}
	
	ps.clear();
	ps.insert(ps.end(), trues_.begin(), trues_.end());		
	prover_.generateTrueProps(ps, 0, prover_.dpg_.legal_level_);	

	updateLegals(ps);
}

bool StateMachine::randomGo(int time_limit)
{
	clock_t begin = clock();
	int count = 0;
	while (!is_terminal_) {
		count ++;
		Propositions joint_move;
		for (int i = 0; i < prover_.roles_.size(); ++i) {			
			joint_move.push_back(getRandomMove(i));
		}
		for (int i = 0; i < joint_move.size(); ++i) {
			//cerr << joint_move[i].toString() << endl;
		}		
		goOneStep(joint_move);
		if (clock() > time_limit) {
			return false;
		}
		//if (count > 0) break;
	}
	clock_t end = clock();
	//cerr<< count <<" steps in "<<end - begin<< " ms" <<endl;
	return true;
}

void StateMachine::setState(Propositions &current_state)
{
	if (current_state == trues_) { // the order of propositions may be different, should implement equel(...)
		return;
	}
	legals_.clear();
	goals_.clear();
	tmps_.clear();
	is_terminal_ = false;
	trues_ = current_state;	
	Propositions ps = trues_;
	prover_.generateTrueProps(ps, 0, prover_.dpg_.legal_level_);
	updateLegals(ps);
}

vector<vector<Proposition>> StateMachine::getLegalJointMoves(int role, int mymove)
{
	vector<vector<Proposition>> rtn;
	vector<vector<Proposition>> legal_moves;
	for(int i = 0 ; i < prover_.roles_.size(); ++i){
		if (i == role) {
			vector<Proposition> t;
			t.push_back(getLegalMoves(role)[mymove]);
			legal_moves.push_back(t);
		} else {
			vector<Proposition> t = getLegalMoves(i);
			legal_moves.push_back(t);		
		}
	}
	vector<int> idx(prover_.roles_.size(), 0);
	while(true){
		vector<Proposition> joint_move;
		for(int i = 0 ; i < prover_.roles_.size(); ++i){
			joint_move.push_back(legal_moves[i][idx[i]]);
		}
		rtn.push_back(joint_move);
		int temp = 0;
		while(temp < idx.size() && idx[temp] == legal_moves[temp].size() - 1){
			idx[temp] = 0;
			temp ++;
		}
		if (temp == idx.size()) {
			break;
		} else {
			idx[temp]++;
		}
	}
	return rtn;
}

int StateMachine::getGoal(int role) {
	for(int i = 0 ; i < goals_.size(); ++i){
		if(goals_[i].items_[0].head_ == prover_.roles_[role].items_[0].head_){
			return atoi(Relation::code2symbol[goals_[i].items_[1].head_].c_str());
		}
	}
	return -1;
}
