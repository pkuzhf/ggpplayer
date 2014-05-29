#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include<cmath>
#include "dependgraph.h"
#include "node.h"

using namespace std;

int Node::C = 1;
double Node::maxScore = 100000000000;

Node::Node(){
	totalAttemps_ = 0;
	nPoints_ = 0;
	nAttemps_ = 0;

	isTerminal_ = false;
	parent_ = NULL;
}

Node::Node(Node * p) {
	Node();
	parent_ = p;
}

double Node::getScore(){
	if(nAttemps_ != 0){
		return (double)nPoints_ / nAttemps_ + C * sqrt(2 * log((double)totalAttemps_) / (double)nAttemps_);
	} else{
		return maxScore;
	}
}