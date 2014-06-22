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
	//cerr << Client::message("debug", "updateTree");
	//cerr << Client::message("debug", node->toString());
	state_machine_.setState(state);
	//cerr << Client::message("debug", Proposition::propsToStr(state_machine_.getLegalMoves(role_num_)));
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
	int role_code = Relation::symbol2code[string(role)];
	for (int i = 0; i < state_machine_.prover_.roles_.size(); ++i) {
		if (state_machine_.prover_.roles_[i].items_[0].head_ == role_code) {
			role_num_ = i;
			break;
		}
	}
	root_.state_ = current_state_;
	root_.is_terminal_ = is_terminal_;
	state_node_[root_.state_] = &root_;
	legal_moves_ = state_machine_.getLegalMoves(role_num_);
}

Proposition MonteCarloPlayer::getRandomMove() {
	return legal_moves_[rand() % legal_moves_.size()];
}

Proposition MonteCarloPlayer::getBestMove() {
	int best_move = root_.getMaximinMove().first;
	if (best_move == -1) {
		best_move = rand() % legal_moves_.size();
	}
	return legal_moves_[best_move];
}

void MonteCarloPlayer::setState(Propositions state) {
	state_machine_.setState(state);
	current_state_ = state;
	is_terminal_ = state_machine_.is_terminal_;
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
				vector<Node> nodes(jointmoves.size(), Node(node));
				node->sons_.push_back(nodes);
			}
			stop = true;
		}
		pair<int, int> move = node->getMaximinMove();
		node = &node->sons_[move.first][move.second];
		if (node->state_.size() == 0){				
			state_machine_.setState(node->parent_->state_);	
			state_machine_.goOneStep(state_machine_.getLegalJointMoves(role_num_, move.first)[move.second]);						
			node->state_ = state_machine_.trues_;
			node->is_terminal_ = state_machine_.is_terminal_;
			state_node_[node->state_] = node;
		}
	}
	return node;
}

double MonteCarloPlayer::uct(int time_limit, int once_simu_limit, int max_simu_times) {
	int simu_count = 0;
	int start = clock();
	
	while (clock() < start + time_limit && simu_count < max_simu_times) {
		simu_count++;
		Node *node = selectLeafNode();		
		int point = -1;
		state_machine_.setState(node->state_);
		if (node->is_terminal_) {
			point = state_machine_.getGoal(role_num_);					
		} else if (state_machine_.randomGo(start + once_simu_limit)) {
			
			point = state_machine_.getGoal(role_num_);
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
	double speed = uct(time_limit, time_limit, 10000000);	
	ostringstream msg;
	msg << "UCT simu times per second: " << speed;
	cerr << Client::message("debug",  msg.str());	

	int best_move = root_.getMaximinMove().first;
	return state_machine_.getLegalMoves(role_num_)[best_move];
}

void MonteCarloPlayer::goOneStep(Propositions moves) {	
	state_machine_.setState(current_state_);	
	state_machine_.goOneStep(moves);
	current_state_ = state_machine_.trues_;
	is_terminal_ = state_machine_.is_terminal_;
	
	root_ = Node();
	root_.state_ = current_state_;
	root_.is_terminal_ = is_terminal_;

	state_node_.clear();
	state_node_[root_.state_] = &root_;
	
	legal_moves_ = state_machine_.getLegalMoves(role_num_);
}