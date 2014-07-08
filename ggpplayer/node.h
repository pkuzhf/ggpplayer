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
	long long points_;
	long long attemps_;	
	bool is_terminal_;
	vector<vector<Node *> > sons_;
	vector<Node *> parent_;
private:
	Propositions state_;
	string s_state_;

public:
	Node(Node * p); 
	Node();
	double getScore();
	string toString();
	pair<int, int> getMaximinMove();
	void init(string &s_state, bool is_terminal);
	void init(Propositions &state, bool is_terminal);
	bool inited();
	Propositions & getState();
	void updatePoints(int point);
};

#endif