#ifndef Node_H
#define Node_H

#include <vector>
#include <set>
#include <map>
#include <string>
#include<cmath>
#include "dependgraph.h"
#include "relation.h"

class Node
{
public:
	Propositions state_;
	int points_;
	int attemps_;
	vector<vector<Node > > sons_;
	Node * parent_;
	bool is_terminal_;

	Node(Node * p); 
	Node();
	double getScore();
};

#endif