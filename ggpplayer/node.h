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
	static int C; 
	static double maxScore;

	Relations moves_;
	Relation mymove_;
	int totalAttemps_;
	int nPoints_;
	int nAttemps_;
	vector<vector<Node > > sons_;
	Node * parent_;
	bool isTerminal_;
	

	double getScore();
	Node(Relations & ms, Relation & m, Node * p, bool t, Relations &state); 
	Node();
};