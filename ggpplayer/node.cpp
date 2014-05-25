#include <vector>
#include <set>
#include <map>
#include <string>
#include<cmath>
#include "dependgraph.h"
#include "node.h"

int Node::C = 1;
double Node::maxScore = 100000000000;

Node::Node(){
	moves_ = Relations();
	mymove_ = Relation();
	totalAttemps_ = 0;
	nPoints_ = 0;
	nAttemps_ = 0;
	isTerminal_ = false;
	sons_ = vector<vector<Node>>();
	parent_ = this;
}

Node::Node(Relations & ms, Relation & m, Node * p, bool t, Relations &state) {
	moves_ = ms;
	mymove_ = m;
	nPoints_ = 0;
	nAttemps_ = 0;
	parent_ = p;
	isTerminal_ = t;
}

double Node::getScore(){
	if(nAttemps_ != 0){
		return (double)nPoints_ / nAttemps_ + C * sqrt(2 * log((double)totalAttemps_) / (double)nAttemps_);
	} else{
		return maxScore;
	}
}