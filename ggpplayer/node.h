#ifndef NODE_H
#define NODE_H

#include <vector>
#include <set>
#include <map>
#include <string>
#include <cmath>
#include "dependgraph.h"
#include "relation.h"

class Node
{
public:
	Propositions state_;
	long long points_;
	long long attemps_;
	double maximin_;
	vector<vector<Node > > sons_;
	Node * parent_;
	bool is_terminal_;

	Node(Node * p); 
	Node();
	double getScore();
	string toString();
	void update(string s);
	pair<int, int> getMaximinMove();
};

#endif