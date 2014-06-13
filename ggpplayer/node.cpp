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

using namespace std;

Node::Node(){
	points_ = 0;
	attemps_ = 0;
	is_terminal_ = false;
	parent_ = NULL;
}

Node::Node(Node * p) {
	points_ = 0;
	attemps_ = 0;
	is_terminal_ = false;
	parent_ = p;
}

double Node::getScore() {
	int c = 1;
	int max_score = 101;
	if (attemps_ == 0) {
		return max_score;
	} else if (parent_ != NULL) {
		return (double)points_ / attemps_ + c * sqrt(2 * log((double)parent_->attemps_) / (double)attemps_);
	} else {
		return (double)points_ / attemps_;
	}
}

string Node::toString() {
	ostringstream ret;	
	ret << "(";
	ret << "(" << points_ << ")";
	ret << "(" << attemps_ << ")";
	for (int i = 0; i < sons_.size(); ++i) {
		ret << "(";
		for (int j = 0; j < sons_[i].size(); ++j) {
			ret << sons_[i][j].toString();
		}
		ret << ")";
	}
	ret << ")";
	return ret.str();
}

void Node::update(string s) {	
	//cerr << Client::message("debug s: ", s);
	//cerr << Client::message("debug node: ", toString());
	int start = 2;
	int end = start + 1;
	while (s[end] != ')') ++end;
	points_ += atoi(s.substr(start, end - start).c_str());
	start = end + 2;
	end = start + 1;
	while (s[end] != ')') ++end;
	attemps_ += atoi(s.substr(start, end - start).c_str());
	start = end + 1;
	if (s[start] == ')') {
		return;
	}
	if (sons_.size() == 0) {
		int i = start;
		while (s[i] == '(') {
			vector<Node> nodes;
			++i;
			while (s[i] == '(') {
				nodes.push_back(Node(this));
				int count = 0;
				do {
					if (s[i] == '(') ++count;
					if (s[i] == ')') --count;
					++i;
				} while (count > 0);
			}
			++i;
			sons_.push_back(nodes);
		}
	}
	for (int i = 0; i < sons_.size(); ++i) {
		++start;
		for (int j = 0; j < sons_[i].size(); ++j) {			
			end = start;
			int count = 0;
			do {
				if (s[end] == '(') ++count;
				if (s[end] == ')') --count;
				++end;
			} while (count > 0);
			sons_[i][j].update(s.substr(start, end - start));
			start = end;
		}
		++start;
	}
}