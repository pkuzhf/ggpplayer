#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <cmath>
#include <sstream>
#include "dependgraph.h"
#include "node.h"
#include "client.h"
#include "reader.h"

using namespace std;

Node::Node(){
	points_ = 0;
	attemps_ = 0;
	is_terminal_ = false;
}

Node::Node(Node * p) {
	points_ = 0;
	attemps_ = 0;
	is_terminal_ = false;
	if (p != NULL) {
		parent_.push_back(p);
	}
}

double Node::getScore() {
	double mean;
	int times;
	if (attemps_ == 0) {
		mean = 50;
		times = 1;
	} else {
		mean = (double)points_ / attemps_;
		times = sqrt((double)attemps_);
		//times = attemps_;
	}
	if (times > 1000) {
		times = 1000;
	}
	for (int i = 0; i < times; ++i) {
		mean += (rand() % 201 - 100) / (double)times;
	}
	return mean;
}

pair<int, int> Node::getMaximinMove() {
	int x = -1;
	int y = -1;
	double maximin_value;
	for (int i = 0; i < sons_.size(); ++i) {
		int min_node = 0;
		double min_value = sons_[i][0]->getScore();
		for (int j = 1; j < sons_[i].size(); ++j) {
			double value = sons_[i][j]->getScore();
			if (value < min_value) {
				min_value = value;
				min_node = j;
			}
		}
		if (x == -1 || min_value > maximin_value) {
			maximin_value = min_value;
			x = i;
			y = min_node;
		}
	}
	return pair<int, int>(x, y);
}

string Node::toString() {
	ostringstream ret;	
	ret << "(";
	ret << "(" << points_ << ")";
	ret << "(" << attemps_ << ")";
	string s = Proposition::propsToStr(state_);
	ret << "(" << s.size() << " " << s << ")";
	ret << "(" << (is_terminal_ ? 1 : 0) << ")";
	for (int i = 0; i < sons_.size(); ++i) {
		ret << "(";
		for (int j = 0; j < sons_[i].size(); ++j) {
			ret << sons_[i][j]->toString();
		}
		ret << ")";
	}
	ret << ")";
	return ret.str();
}

void Node::init(string &s_state, bool is_terminal) {
	s_state_ = s_state;
	is_terminal_ = is_terminal;
}

void Node::init(Propositions &state, bool is_terminal) {
	state_ = state;
	is_terminal_ = is_terminal;
}

bool Node::inited() {
	if (s_state_ == "" && state_.size() == 0) {
		return false;
	} else {
		return true;
	}
}

Propositions & Node::getState() {
	if (state_.size() == 0 && s_state_ != "") {
		Reader r;
		r.file_content_ = s_state_;
		r.getPropositions(state_);
	}
	return state_;
}

void Node::updatePoints(int point, int attemps) {
	static double factor = 0.99;
	static vector<double> factors(1001);
	static bool init = false;
	if (!init) {
		factors[0] = 1;
		for (int i = 1; i < factors.size(); ++i) {
			factors[i] = factors[i - 1] * factor;
		}
		init = true;
	}

	points_ /= attemps_;
	attemps_ += attemps;
	double f = factors[attemps > 1000 ? 1000 : attemps];
	if (attemps_ == 1) {
		points_ = point;
	} else {
		points_ = (points_ * f + point * (1 - f)) * attemps_;
	}
}