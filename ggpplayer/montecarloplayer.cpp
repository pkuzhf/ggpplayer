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
#include "reader.h"

using namespace std;

void MonteCarloPlayer::updateTree(Propositions state, string tree) {
	if (map_state_node_.find(Proposition::propsToStr(state)) == map_state_node_.end()) {
		return;
	}
	Node * node = map_state_node_[Proposition::propsToStr(state)];
	//cerr << Client::message("debug", "updateTree");
	//cerr << Client::message("debug", node->toString());
	//state_machine_.setState(state);
	//cerr << Client::message("debug", Proposition::propsToStr(state_machine_.getLegalMoves(role_)));
	long long old_points = node->points_;
	long long old_attemps = node->attemps_;
	updateNode(node, tree);
	//cerr << Client::message("debug", "updateNode complete");
	long long points = node->points_ - old_points;
	long long attemps = node->attemps_ - old_attemps;
	updateParents(node, points, attemps);
	//cerr << Client::message("debug", "update tree complete");
}

MonteCarloPlayer::MonteCarloPlayer(){}
MonteCarloPlayer::MonteCarloPlayer(Relations rs, string role):state_machine_(rs) {
	current_state_ = state_machine_.trues_;
	is_terminal_ = state_machine_.is_terminal_;
	int role_code = Relation::symbol2code[string(role)];
	for (int i = 0; i < state_machine_.prover_.roles_.size(); ++i) {
		if (state_machine_.prover_.roles_[i].items_[0].head_ == role_code) {
			role_ = i;
			break;
		}
	}
	root_ = newNode();
	initNode(root_, current_state_, is_terminal_);
	legal_moves_ = state_machine_.getLegalMoves(role_);
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
	initNode(root_, current_state_, is_terminal_);
}

Node * MonteCarloPlayer::selectLeafNode() {
	Node *node = root_;
	while (node->attemps_ > 0 && !node->is_terminal_) {
		if (node->sons_.size() == 0) {
			state_machine_.setState(node->state_);
			int move_size = state_machine_.getLegalMoves(role_).size();
			for (int i = 0; i < move_size; i++) {
				vector<vector<Proposition>> jointmoves = state_machine_.getLegalJointMoves(role_, i);
				vector<Node *> nodes;
				for (int j = 0; j < jointmoves.size(); ++j) {
					nodes.push_back(newNode(node));
				}
				node->sons_.push_back(nodes);
			}
		}
		pair<int, int> move = node->getMaximinMove();
		Node * parent = node;
		node = node->sons_[move.first][move.second];
		if (node->state_.size() == 0){
			state_machine_.setState(parent->state_);
			state_machine_.goOneStep(state_machine_.getLegalJointMoves(role_, move.first)[move.second]);
			if (map_state_node_.find(Proposition::propsToStr(state_machine_.trues_)) != map_state_node_.end()) {
				delete node;
				node = map_state_node_[Proposition::propsToStr(state_machine_.trues_)];
				parent->sons_[move.first][move.second] = node;
				node->parent_.push_back(parent);
			} else {
				initNode(node, state_machine_.trues_, state_machine_.is_terminal_);
			}
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
			point = state_machine_.getGoal(role_);					
		} else if (state_machine_.randomGo(start + once_simu_limit)) {
			point = state_machine_.getGoal(role_);
		}	
		if (point != -1) {
			node->points_ += point;
			++node->attemps_;
			updateParents(node, point, 1);
		}
	}
	int stop = clock();
	return (double)simu_count / (stop - start) * CLOCKS_PER_SEC;
}

Proposition MonteCarloPlayer::stateMachineSelectMove(int time_limit) {
	if (state_machine_.getLegalMoves(role_).size() == 1){
		return state_machine_.getLegalMoves(role_)[0];
	}
	double speed = uct(time_limit, time_limit, 10000000);	
	ostringstream msg;
	msg << "UCT simu times per second: " << speed;
	cerr << Client::message("debug",  msg.str());	

	int best_move = root_->getMaximinMove().first;
	return state_machine_.getLegalMoves(role_)[best_move];
}

void MonteCarloPlayer::goOneStep(Propositions moves) {	
	sort(moves.begin(), moves.end());
	state_machine_.setState(current_state_);	
	state_machine_.goOneStep(moves);
	current_state_ = state_machine_.trues_;
	is_terminal_ = state_machine_.is_terminal_;
	
	if (map_state_node_.find(Proposition::propsToStr(current_state_)) != map_state_node_.end()) {
		root_ = map_state_node_[Proposition::propsToStr(current_state_)];
	} else {
		deleteNodes();
		root_ = newNode();
		initNode(root_, current_state_, is_terminal_);
	}
	legal_moves_ = state_machine_.getLegalMoves(role_);
}

Node * MonteCarloPlayer::newNode(Node * parent) {
	Node * node = new Node(parent);
	return node;
}

void MonteCarloPlayer::deleteNodes() {
	for (unordered_map<string, Node *>::iterator i = map_state_node_.begin(); i != map_state_node_.end(); ++i) {
		Node * node = i->second;
		for (int j = 0; j < node->sons_.size(); ++j) {
			for (int k = 0; k < node->sons_[j].size(); ++k) {
				if (node->sons_[j][k]->state_.size() == 0) {
					delete node->sons_[j][k];
				}
			}
		}
	}
	for (unordered_map<string, Node *>::iterator i = map_state_node_.begin(); i != map_state_node_.end(); ++i) {
		delete i->second;
	}
	map_state_node_.clear();
}

void MonteCarloPlayer::updateNode(Node * node, string s) {	
	//cerr << Client::message("debug s: ", s);
	//cerr << Client::message("debug node: ", node->toString());
	int start = 2;
	int end = start + 1;
	while (s[end] != ')') ++end;
	node->points_ += atoi(s.substr(start, end - start).c_str());
	start = end + 2;
	end = start + 1;
	while (s[end] != ')') ++end;
	node->attemps_ += atoi(s.substr(start, end - start).c_str());
	start = end + 1;
	end = start;
	int count = 0;
	do {
		if (s[end] == '(') ++count;
		if (s[end] == ')') --count;
		++end;
	} while(count > 0);
	if (node->state_.size() == 0) {
		cerr << Client::message("debug state:", s.substr(start + 1, end - start - 2));
		Reader r;
		r.file_content_ = s.substr(start + 1, end - start - 1);
		r.getPropositions(node->state_);
	}
	start = end;
	if (s[start] == ')') {
		//cerr << Client::message("debug ~s: ", s);
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
				nodes.push_back(newNode(node));
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
			if (start >= s.size() || s[start] == ')') {
				break;
			}
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
	//cerr << Client::message("debug ~s: ", s);
}

void MonteCarloPlayer::updateParents(Node * node, long long points, long long attemps) {
	unordered_set<Node *> ancients;
	getAncients(node, ancients);
	for (unordered_set<Node *>::iterator i = ancients.begin(); i != ancients.end(); ++i) {
		(*i)->points_ += points;
		(*i)->attemps_ += attemps;
	}
}

void MonteCarloPlayer::getAncients(Node * node, unordered_set<Node *> &ancients) {
	if (find(ancients.begin(), ancients.end(), node) != ancients.end()) {
		return;
	}
	ancients.insert(node);
	if (node == root_) {
		return;
	}
	for (int i = 0; i < node->parent_.size(); ++i) {
		getAncients(node->parent_[i], ancients);
	}
}

void MonteCarloPlayer::initNode(Node * node, Propositions & state, bool is_terminal) {
	node->state_ = state;
	node->is_terminal_ = is_terminal;
	map_state_node_[Proposition::propsToStr(state)] = node;
}