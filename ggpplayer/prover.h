#ifndef PROVER_H
#define PROVER_H

#include <vector>
#include <set>
#include <map>
#include <string>
#include "domaingraph.h"
#include "tools.h"

using namespace std;

class Prover {
public:
	map<Relation, int> keyrelation_num_;
	vector<Relation> keyrelations_;

	map<Relation, int> legalaction_num_;
	vector<Relation> legalactions_;

	map<Relation, int> role_num_;
	vector<Relation> roles_;

	Prover(Relations relations);
	string getInitState();
	int askRole(Relation &role);  // 返回role在所有玩家中的编号 
	bool askGoal(vector<int> &result, const string &state);
	bool askTerminal(const string & state);
	string askLegalActions(int role, const string &state);
	string askLegalActions(const string &state);
	string askNextState(const string &currentstate, const string &does);
	

private:
	Relations relations_;
	Relations derivations_;
	DomainGraph dg_;
	bool validateInstance(string instance, set<string> &true_instances, set<string> &false_instances, set<string> &validating_instances);
	void findVarDomainInSingleInstance(Relation r, map<string, set<string>> &var_values);
	string buildNode(string s, int i);

};

#endif