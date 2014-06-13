#include <iostream>
#include <vector>
#include <time.h>
#include <string>
#include <sstream>
#include <cmath>
#include <stdlib.h>

#include "dependgraph.h"
#include "node.h"
#include "montecarloplayer.h"
#include "client.h"

using namespace std;

void MonteCarloPlayer::updateTree(Propositions state, string tree) {
	if (state_node_.find(state) == state_node_.end()) {
		return;
	}
	Node * node = state_node_[state];
	node->update(tree);
	int points = node->points_;
	int attemps = node->attemps_;
	while (node->parent_) {
		node = node->parent_;
		node->points_ += points;
		node->attemps_ += attemps;
	}
}

MonteCarloPlayer::MonteCarloPlayer(){}
MonteCarloPlayer::MonteCarloPlayer(Relations rs, string role):state_machine_(rs) {
	current_state_ = state_machine_.trues_;
	is_terminal_ = false;
	int role_num = Relation::symbol2code[string(role)];
	for (int i = 0; i < state_machine_.prover_.roles_.size(); ++i) {
		if (state_machine_.prover_.roles_[i].items_[0].head_ == role_num) {
			role_num_ = i;
			break;
		}
	}
	root_.state_ = current_state_;
	root_.is_terminal_ = is_terminal_;
	state_node_[current_state_] = &root_;
	legal_moves_ = state_machine_.getLegalMoves(role_num_);
}

Proposition MonteCarloPlayer::getRandomMove() {
	return legal_moves_[rand() % legal_moves_.size()];
}

Proposition MonteCarloPlayer::getBestMove() {
	int best_move = getBestMoveOfNode(&root_);	
	return legal_moves_[best_move];
}

void MonteCarloPlayer::setState(Propositions state) {
	state_machine_.setState(state);
	current_state_ = state;
	is_terminal_ = state_machine_.is_terminal_;
	state_node_.clear();
	root_ = Node();
	root_.state_ = current_state_;
	root_.is_terminal_ = is_terminal_;
}

Node * MonteCarloPlayer::selectLeafNode() {
	Node *node = &root_;
	bool stop = false;
	while (!node->is_terminal_ && !stop) {
		if (node->sons_.size() == 0) {
			state_machine_.setState(node->state_);
			int move_size = state_machine_.getLegalMoves(role_num_).size();
			for (int i = 0; i < move_size; i++) {
				vector<vector<Proposition>> jointmoves = state_machine_.getLegalJointMoves(role_num_, i);
				vector<Node> nodes;				
				for (int j = 0; j < jointmoves.size(); j++) {					
					nodes.push_back(Node(node));
				}
				node->sons_.push_back(nodes);
			}
			stop = true;
		}
		int best_move = getBestMoveOfNode(node);
		vector<int> joint_move_candidates;
		joint_move_candidates.push_back(0);
		for (int i = 1; i < node->sons_[best_move].size(); ++i) {
			if (node->sons_[best_move][i].attemps_ < node->sons_[best_move][joint_move_candidates[0]].attemps_) {
				joint_move_candidates.clear();
				joint_move_candidates.push_back(i);
			} else if (node->sons_[best_move][i].attemps_ == node->sons_[best_move][joint_move_candidates[0]].attemps_) {
				joint_move_candidates.push_back(i);
			}
		}
		int joint_move = joint_move_candidates[rand() % joint_move_candidates.size()];
		node = &node->sons_[best_move][joint_move];
		if (node->state_.size() == 0){				
			state_machine_.setState(node->parent_->state_);	
			state_machine_.goOneStep(state_machine_.getLegalJointMoves(role_num_, best_move)[joint_move]);						
			node->state_ = state_machine_.trues_;
			node->is_terminal_ = state_machine_.is_terminal_;
			state_node_[node->state_] = node;
		}
	}
	return node;
}

int MonteCarloPlayer::getBestMoveOfNode(Node * node) {
	int max = 0;
	double maxscore = 0;
	for (int i = 0; i < node->sons_.size(); i++) {
		vector<Node> nodes = node->sons_[i];
		double total_score = 0;
		for (int j = 0; j < nodes.size(); j++) {
			total_score += nodes[j].getScore();
		}
		if (total_score / nodes.size() > maxscore) {
			max = i;
			maxscore = total_score / nodes.size();
		}					
	}
	return max;
}

double MonteCarloPlayer::uct(int time_limit) {
	int simu_count = 0;
	int start = clock();
	int finish_by = start + time_limit;
	
	while (clock() < finish_by) {		
		Node *node = selectLeafNode();		
		int point = -1;
		state_machine_.setState(node->state_);
		if (node->is_terminal_) {
			point = state_machine_.getGoal(role_num_);					
		} else {			
			if (state_machine_.randomGo(finish_by)) {
				simu_count++;
				point = state_machine_.getGoal(role_num_);
			}				
		}		
		if (point != -1) {			
			do {
				node->points_ += point;
				++node->attemps_;
				node = node->parent_;
			} while (node != NULL);
		}
	}
	int stop = clock();
	return (double)simu_count / (stop - start) * CLOCKS_PER_SEC;
}

Proposition MonteCarloPlayer::stateMachineSelectMove(int time_limit) {
	if (state_machine_.getLegalMoves(role_num_).size() == 1){
		return state_machine_.getLegalMoves(role_num_)[0];
	}
	double speed = uct(time_limit);	
	ostringstream msg;
	msg << "UCT simu times per second: " << speed;
	cerr << Client::message("debug",  msg.str());	

	int best_move = getBestMoveOfNode(&root_);
	return state_machine_.getLegalMoves(role_num_)[best_move];
}

void MonteCarloPlayer::goOneStep(Propositions moves) {	
	state_machine_.setState(current_state_);	
	state_machine_.goOneStep(moves);
	current_state_ = state_machine_.trues_;
	is_terminal_ = state_machine_.is_terminal_;
	bool found = false;
	//for (int i = 0; i < root_.sons_.size() && !found; ++i) {
	//	for (int j = 0; j < root_.sons_[i].size() && !found; ++j) {
	//		if (root_.sons_[i][j].state_ == current_state_) {
	//			root_ = root_.sons_[i][j];
	//			root_.parent_ = NULL;
	//			found = true;
	//		}
	//	}
	//}
	if (!found) {
		state_node_.clear();
		root_ = Node();
		root_.state_ = current_state_;
		root_.is_terminal_ = is_terminal_;
		//cerr << Client::message("debug", "node not found error");
	}
	legal_moves_ = state_machine_.getLegalMoves(role_num_);
}