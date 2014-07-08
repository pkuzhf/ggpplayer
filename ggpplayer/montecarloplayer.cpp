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
#include "statemachine.h"
#include "prover.h"

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
	pair<int, int> update = updateNode(node, tree);
	//cerr << Client::message("debug", "updateNode complete");
	int point = update.first;
	int attemps = update.second;
	if (attemps > 0) {
		vector<Node *> path = map_state_path_[node];
		for (int i = 0; i < path.size(); ++i) {
			for (int j = 0; j < attemps; ++j) {
				path[i]->updatePoints(point);
			}
		}
		//updateParents(node, points, attemps);
		//cerr << Client::message("debug", "updateTree complete");
	}
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

void MonteCarloPlayer::expandeNode(Node * node) {
	state_machine_.setState(node->getState());
	int move_size = state_machine_.getLegalMoves(role_).size();
	vector<int> jointmove_sizes;
	for (int i = 0; i < move_size; ++i) {
		jointmove_sizes.push_back(state_machine_.getLegalJointMoves(role_, i).size());
	}
	for (int i = 0; i < move_size; ++i) {
		vector<Node *> nodes;
		for (int j = 0; j < jointmove_sizes[i]; ++j) {
			state_machine_.setState(node->getState());
			state_machine_.goOneStep(state_machine_.getLegalJointMoves(role_, i)[j]);
			if (map_state_node_.find(Proposition::propsToStr(state_machine_.trues_)) != map_state_node_.end()) {
				Node * used_node = map_state_node_[Proposition::propsToStr(state_machine_.trues_)];
				used_node->parent_.push_back(node);
				nodes.push_back(used_node);
			} else {
				Node * new_node = newNode(node);
				initNode(new_node, state_machine_.trues_, state_machine_.is_terminal_);
				if (new_node->is_terminal_) {
					new_node->attemps_ = 1000000;
					new_node->points_ = new_node->attemps_ * state_machine_.getGoal(role_);
				}
				nodes.push_back(new_node);
			}
		}
		node->sons_.push_back(nodes);
	}
}

Node * MonteCarloPlayer::selectLeafNode(vector<Node *> &path) {
	Node *node = root_;
	path.push_back(node);
	while (node->attemps_ > 0 && !node->is_terminal_) {
		if (node->sons_.size() == 0) {
			expandeNode(node);
		}
		pair<int, int> move = node->getMaximinMove();
		Node * parent = node;
		node = node->sons_[move.first][move.second];
		path.push_back(node);
	}
	return node;
}

Node * MonteCarloPlayer::selectLeafNodeServer() {
	Node *node = root_;
	vector<Node *> path;
	while (node->sons_.size() > 0) {
		path.push_back(node);
		pair<int, int> move = node->getMaximinMove();
		node = node->sons_[move.first][move.second];
	}
	if (node->is_terminal_ && node->attemps_ > 0) {
		for (int i = 0; i < path.size(); ++i) {
			path[i]->updatePoints(node->points_);
		}
		path.clear();
		int try_times = 0;
		do {
			node = root_;
			while (node->sons_.size() > 0) {
				int first = rand() % node->sons_.size();
				int second = rand() % node->sons_[first].size();
				node = node->sons_[first][second];
			}
			++try_times;
		} while (node->is_terminal_ && try_times < 10);
	}
	map_state_path_[node] = path;
	return node;
}

double MonteCarloPlayer::uct(clock_t time_limit, clock_t once_simu_limit, int max_simu_times) {
	int simu_count = 0;
	clock_t start = clock();
	
	if (!root_->is_terminal_) {
		expandeNode(root_);
	}
	while (clock() < start + time_limit) {
		if (simu_count >= max_simu_times || root_->toString().size() > 1000000) {
			continue;
		}
		simu_count++;
		vector<Node *> path;
		Node *node = selectLeafNode(path);		
		int point = -1;
		if (node->is_terminal_ && node->attemps_ > 0) {
			point = node->points_ / node->attemps_;					
		} else {
			state_machine_.setState(node->getState());
			if (state_machine_.randomGo(start + once_simu_limit)) {
				point = state_machine_.getGoal(role_);
			}
		}
		if (point != -1) {
			for (int i = 0; i < path.size(); ++i) {
				path[i]->updatePoints(point);
			}
		}
	}
	clock_t stop = clock();
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
				if (!node->sons_[j][k]->inited()) {
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

pair<int, int> MonteCarloPlayer::updateNode(Node * node, string s) {	
	
	//cerr << Client::message("debug s: ", s);
	//cerr << Client::message("debug node: ", node->toString());
	pair<int, int> ret;

	int start = 2;
	int end = s.find(")", start);
	ret.first = atoi(s.substr(start, end - start).c_str());

	start = end + 2;
	end = s.find(")", start);
	ret.second = atoi(s.substr(start, end - start).c_str());
	
	if (ret.second > 0) {
		ret.first /= ret.second;
	}
	//if (ret.first < 0) {
	//	ostringstream o;
	//	o << "points " << ret.first << " s " << s;
	//	cerr << Client::message("stat", o.str());
	//}
	for (int i = 0; i < ret.second; ++i) {
		node->updatePoints(ret.first);
	}

	start = end + 2; // skip ")("
	end = s.find(" ", start);
	int state_length = atoi(s.substr(start, end - start).c_str());
	start = end + 1; // skip " "
	string s_state = s.substr(start, state_length);
	start += state_length + 1;  // skip state and ")"
	end = s.find(")", start) + 1;
	bool is_terminal = atoi(s.substr(start + 1, end - start - 2).c_str());
	start = end;
	//cerr << Client::message("debug", s.substr(start + 1, end - start - 2));
	if (!node->inited() && s_state != "") {
		//cerr << Client::message("debug state:", s_state);
		if (map_state_node_.find(s_state) != map_state_node_.end()) {
			Node * used_node = map_state_node_[s_state];
			for (int i = 0; i < node->parent_.size(); ++i) {
				Node * parent = node->parent_[i];
				bool find = false;
				for (int j = 0; j < parent->sons_.size() && !find; ++j) {
					for (int k = 0; k < parent->sons_[j].size() && !find; ++k) {
						if (parent->sons_[j][k] == node) {
							find = true;
							parent->sons_[j][k] = used_node;
						}
					}
				}
			}
			used_node->parent_.insert(used_node->parent_.end(), node->parent_.begin(), node->parent_.end());
			delete node;
			node = used_node;
		} else {
			initNode(node, s_state, is_terminal);
		}
	}
	
	//cerr << Client::message("debug", s.substr(start));
	if (s[start] == ')') {
		//cerr << Client::message("debug ~s: ", s);
		return ret;
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
	return ret;
	//cerr << Client::message("debug ~s: ", s);
}

void MonteCarloPlayer::initNode(Node * node, Propositions & state, bool is_terminal) {
	node->init(state, is_terminal);
	map_state_node_[Proposition::propsToStr(state)] = node;
}

void MonteCarloPlayer::initNode(Node * node, string & s_state, bool is_terminal) {
	node->init(s_state, is_terminal);
	map_state_node_[s_state] = node;
}