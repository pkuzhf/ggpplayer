#ifndef PROVER_H
#define PROVER_H

#include <vector>
#include <set>
#include <map>
#include <string>
#include "dependgraph.h"
#include "relation.h"

using namespace std;

class Prover {
public:
	map<Relation, int> keyrelation_num_;
	vector<Relation> keyrelations_;

	map<Relation, int> legalaction_num_;
	vector<Relation> legalactions_;

	map<Relation, int> role_num_;
	vector<Relation> roles_;

	vector<int> key_head_;

	Prover(Relations relations);
	void init();
	
	int askRole(Relation &role);  // 返回role在所有玩家中的编号 
	bool askGoal(vector<int> &result, const string &state);
	bool askTerminal(const string & state);

	void askNextStateByDPG(Relations &currentstate, Relations &does);	
	Relations generateTrueProps(Relations true_props);
	Relations statics_;
	Relations inits_;
private:
	Relations relations_;
	Relations derivations_;
	Relations static_derivations_;
	Relations nonstatic_derivations_;	
	Relations bases_;
	Relations inputs_;
	
	DependGraph dpg_;		

	// get by DPG
	vector<int> static_relation_;

	bool validateInstance(string instance, set<string> &true_instances, set<string> &false_instances, set<string> &validating_instances);
	void findVarDomainInSingleInstance(Relation r, map<string, set<string> > &var_values);
	string buildNode(string s, int i);
	bool conditions_satisfied(Relation relation, map<int, int> var_value, vector<string> vars, vector<vector<string> > values, int condition_count , set<string> &true_instances, set<string> &false_instances, set<string> &validating_instances);
	bool contain_var(Relation r, string var);
	
	void getStaticRelation();
	void markNonStatic(int index, vector<int> & mark);
};

#endif