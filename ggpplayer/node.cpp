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
	double mean;
	int range;
	if (attemps_ == 0) {
		mean = 50;
		range = 100;
	} else {
		mean = (double)points_ / attemps_;
		range = 100 / (log((double)attemps_ + 1) / log(2.0));
	}
	int ret = mean - (double)range / 2 + rand() % (range + 1);
	return ret;
}

pair<int, int> Node::getMaximinMove() {
	int x = -1;
	int y = -1;
	double maximin_value;
	for (int i = 0; i < sons_.size(); ++i) {
		int min = 0;
		double min_value = sons_[i][0].getScore();
		for (int j = 1; j < sons_[i].size(); ++j) {
			double value = sons_[i][j].getScore();
			if (value < min_value) {
				min_value = value;
				min = j;
			}
		}
		if (x == -1 || min_value > maximin_value) {
			maximin_value = min_value;
			x = i;
			y = min;
		}
	}
	return pair<int, int>(x, y);
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
	bool check = false;
	if (sons_.size() == 0) {
		check = true;
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
			sons_[i][j].update(s.substr(start, end - start));
			start = end;
		}
		++start;
	}
}