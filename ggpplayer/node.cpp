#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include<cmath>
#include "dependgraph.h"
#include "node.h"

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
	int max_score = 100;
	if (attemps_ == 0) {
		return max_score;
	} else if (parent_ != NULL) {
		return (double)points_ / attemps_ + c * sqrt(2 * log((double)parent_->attemps_) / (double)attemps_);
	} else {
		return (double)points_ / attemps_;
	}
}