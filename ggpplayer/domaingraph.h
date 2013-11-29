#ifndef DOMAINGRAPH_H
#define DOMAINGRAPH_H

#include <vector>
#include <set>
#include <map>
#include <string>

#include "tools.h"

using namespace std;

class DomainGraph {
public:
	vector<string> nodes_;
	vector<set<int>> edges_;
	map<string, int> node_num_;
	map<string, vector<string>> node_instances_;

	void buildGraph(Relations rs);

private:
	void removeNodesByType(Relation &r, RelationType type);
	void buildGraphBySingleRelation(Relation r);
	void addNode(string node);
	void findVariableNode(Relation r, map<string, set<int>> &m);
	string buildNode(string s, int i);
	void buildMaximalInstancesForNode(string node, set<string> path);
	void removeInpossibleInstancesForNode(string node, set<string> path);
	bool validateInstance(string instance, set<string> &true_instances, set<string> &false_instances, set<string> &validating_instances, Relations &rs);
	void findVarDomainInSingleInstance(Relation r, map<string, set<string>> &var_values);
};

#endif