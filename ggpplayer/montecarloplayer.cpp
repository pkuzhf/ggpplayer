#include <iostream>
#include <vector>
#include <time.h>
#include <string>
#include <sstream>
#include <cmath>
#include <stdlib.h>
#include <algorithm>

#include "dependgraph.h"
#include "node.h"
#include "montecarloplayer.h"
#include "client.h"

using namespace std;

void MonteCarloPlayer::updateTree(int code, Propositions state, string tree) {
	if (code >= nodes_.size() || nodes_[code]->state_ != state) {
		return;
	}
	Node * node = nodes_[code];
	cerr << Client::message("debug", "updateTree");
	cerr << Client::message("debug", node->toString());
	state_machine_.setState(state);
	cerr << Client::message("debug", Proposition::propsToStr(state_machine_.getLegalMoves(role_num_)));
	long long old_points = node->points_;
	long long old_attemps = node->attemps_;
	updateNode(node, tree);
	long long points = node->points_ - old_points;
	long long attemps = node->attemps_ - old_attemps;
	while (node->parent_) {
		node = node->parent_;
		node->points_ += points;
		node->attemps_ += attemps;
	}
}

MonteCarloPlayer::MonteCarloPlayer(){}
MonteCarloPlayer::MonteCarloPlayer(Relations rs, string role):state_machine_(rs) {
	current_state_ = state_machine_.trues_;
	is_terminal_ = state_machine_.is_terminal_;
	int role_code = Relation::symbol2code[string(role)];
	for (int i = 0; i < state_machine_.prover_.roles_.size(); ++i) {
		if (state_machine_.prover_.roles_[i].items_[0].head_ == role_code) {
			role_num_ = i;
			break;
		}
	}
	root_ = newNode();
	root_->init(current_state_, is_terminal_);
	legal_moves_ = state_machine_.getLegalMoves(role_num_);
}

Proposition MonteCarloPlayer::getRandomMove() {
	return legal_moves_[rand() % legal_moves_.size()];
}

Proposition MonteCarloPlayer::getBestMove() {
	int best_move = root_->getMaximinMove().first;
	if (best_move == -1) {
		best_move = rand() % legal_moves_.size();
	}
	return legal_moves_[best_move];
}

void MonteCarloPlayer::setState(Propositions state) {
	state_machine_.setState(state);
	current_state_ = state;
	is_terminal_ = state_machine_.is_terminal_;
	deleteNodes();
	root_ = newNode();
	root_->init(current_state_, is_terminal_);
}

Node * MonteCarloPlayer::selectLeafNode() {
	Node *node = root_;
	while (!node->attemps_ == 0 && !node->is_terminal_) {
		cerr << Client::message("debug", Proposition::propsToStr(node->state_));
		if (node->sons_.size() == 0) {
			state_machine_.setState(node->state_);
			int move_size = state_machine_.getLegalMoves(role_num_).size();
			for (int i = 0; i < move_size; i++) {
				vector<vector<Proposition>> jointmoves = state_machine_.getLegalJointMoves(role_num_, i);
				vector<Node *> nodes;
				for (int j = 0; j < jointmoves.size(); ++j) {
					nodes.push_back(newNode(node));
				}
				node->sons_.push_back(nodes);
			}
		}
		pair<int, int> move = node->getMaximinMove();
		node = node->sons_[move.first][move.second];
		cerr << Client::message("debug", Proposition::propsToStr(node->state_));
		if (node->state_.size() == 0){				
			state_machine_.setState(node->parent_->state_);	
			state_machine_.goOneStep(state_machine_.getLegalJointMoves(role_num_, move.first)[move.second]);						
			node->init(state_machine_.trues_, state_machine_.is_terminal_);
		}
	}
	return node;
}

double MonteCarloPlayer::uct(int time_limit, int once_simu_limit, int max_simu_times) {
	int simu_count = 0;
	int start = clock();
	
	while (clock() < start + time_limit) {
		if (simu_count > max_simu_times) {
			continue;
		}
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

	int best_move = root_->getMaximinMove().first;
	return state_machine_.getLegalMoves(role_num_)[best_move];
}

void MonteCarloPlayer::goOneStep(Propositions moves) {	
	sort(moves.begin(), moves.end());
	state_machine_.setState(current_state_);	
	state_machine_.goOneStep(moves);
	current_state_ = state_machine_.trues_;
	is_terminal_ = state_machine_.is_terminal_;
	
	if (root_->sons_.size() > 0) {
		bool find = false;
		for (int i = 0; i < root_->sons_.size() && !find; ++i) {
			for (int j = 0; j < root_->sons_[i].size() && !find; ++j) {
				if (root_->sons_[i][j]->state_ == current_state_) {
					find = true;
					root_ = root_->sons_[i][j];
				}
			}
		}
	} else {
		deleteNodes();
		root_ = newNode();
	}
	root_->init(current_state_, is_terminal_);
	legal_moves_ = state_machine_.getLegalMoves(role_num_);
}

Node * MonteCarloPlayer::newNode(Node * parent) {
	Node * node = new Node(parent);
	nodes_.push_back(node);
	node->code_ = nodes_.size() - 1;
	return node;
}

void MonteCarloPlayer::deleteNodes() {
	for (int i = 0; i < nodes_.size(); ++i) {
		delete nodes_[i];
	}
	nodes_.clear();
}

void MonteCarloPlayer::updateNode(Node * node, string s) {	
	//cerr << Client::message("debug s: ", s);
	//cerr << Client::message("debug node: ", toString());
	int start = 2;
	int end = start + 1;
	while (s[end] != ')') ++end;
	node->points_ += atoi(s.substr(start, end - start).c_str());
	start = end + 2;
	end = start + 1;
	while (s[end] != ')') ++end;
	node->attemps_ += atoi(s.substr(start, end - start).c_str());
	start = end + 1;
	if (s[start] == ')') {
		return;
	}
	bool check = false;
	if (node->sons_.size() == 0) {
		check = true;
		int i = start;
		while (s[i] == '(') {
			vector<Node *> nodes;
			++i;
			while (s[i] == '(') {
				nodes.push_back(newNode());
				int count = 0;
				do {
					if (s[i] == '(') ++count;
					if (s[i] == ')') --count;
					++i;
				} while (count > 0);
			}
			++i;
			node->sons_.push_back(nodes);
		}
	}
	for (int i = 0; i < node->sons_.size(); ++i) {
		++start;
		for (int j = 0; j < node->sons_[i].size(); ++j) {
			/*if (start >= s.size() || s[start] == ')') {
				break;
			}*/
			end = start;
			int count = 0;
			do {
				if (s[end] == '(') ++count;
				if (s[end] == ')') --count;
				++end;
			} while (count > 0);
			updateNode(node->sons_[i][j], s.substr(start, end - start));
			start = end;
		}
		++start;
	}
}