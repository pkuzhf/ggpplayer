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
	code_ = -1;
}

Node::Node(Node * p) {
	points_ = 0;
	attemps_ = 0;
	is_terminal_ = false;
	parent_ = p;
	code_ = -1;
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

