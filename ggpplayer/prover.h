#ifndef PROVER_H
#define PROVER_H

#include <vector>
#include <set>
#include <map>
#include <string>
#include "domaingraph.h"
#include "dependgraph.h"
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
	void init();
	string getInitState();
	int askRole(Relation &role);  // ����role����������еı�� 
	bool askGoal(vector<int> &result, const string &state);
	bool askTerminal(const string & state);
	string askLegalActions(int role, const string &state);
	string askLegalActions(const string &state);
	string askNextState(const string &currentstate, const string &does);
	void askNextStateByDPG(Relations &currentstate, Relations &does);
	Relations getInitStateByDPG();
	

private:
	Relations relations_;
	Relations derivations_;
	Relations inits_;
	Relations statics_;
	DomainGraph dg_;
	DependGraph dpg_;	

	// get by DPG
	vector<int> static_relation_;

	bool validateInstance(string instance, set<string> &true_instances, set<string> &false_instances, set<string> &validating_instances);
	void findVarDomainInSingleInstance(Relation r, map<string, set<string>> &var_values);
	string buildNode(string s, int i);
	bool conditions_satisfied(Relation relation, map<string, string> var_value, vector<string> vars, vector<vector<string>> values, int condition_count , set<string> &true_instances, set<string> &false_instances, set<string> &validating_instances);
	bool contain_var(Relation r, string var);
	Relations generateTrueProps(Relations true_props);
	void getStaticRelation();
	void markNonStatic(int index, vector<int> & mark);
};

#endif