#ifndef PROVER_H
#define PROVER_H

#include <vector>
#include <set>
#include <map>
#include <string>
#include<cmath>
#include "dependgraph.h"
#include "relation.h"


using namespace std;

typedef struct{
	size_t operator()(const Relation & r) const{
		unsigned long __h = 0;
		string str = r.toString();
		for (int i = 0 ; i < str.size() ; i += 1)
			__h = 5*__h + str[i];
		return size_t(__h);
	} 
	enum   
	{   //   parameters   for   hash   table   
		bucket_size   =   8,   //   0   <   bucket_size   
		min_buckets   =   16  //   min_buckets   =   2   ^^   N,   0   <   N   
	};   
	bool operator()(const Relation & a1, const Relation & a2)const{
		return  a1.toString().compare(a2.toString()) < 0;
	}
} RelationCMP;


class Prover {
public:
	static int generate_time;
	static int time1;
	static int time2;
	static int time3;
	static int time4;
	static int time5;
	static int time6;
	static int time7;
	static int time8;
	static int time9;
	static int time10;
	static int time11;
	static int time12;
	static int time13;
	static int time14;
	static int time15;
	static int time16;
	static int time17;
	static int time18;
	static int time19;
	static int time20;
	static int time21;

	map<Proposition, int> keyrelation_num_;
	Propositions keyrelations_;

	map<Proposition, int> legalaction_num_;
	Propositions legalactions_;

	map<Proposition, int> role_num_;
	Propositions roles_;

	vector<int> key_head_;

	Prover(Relations relations);
	void init();

	int askRole(Proposition &role);  // 返回role在所有玩家中的编号 
	bool askGoal(vector<int> &result, const string &state);
	bool askTerminal(const string & state);
	
	Propositions generateTrueProps(Propositions true_props, int start_stra, int end_stra);
	Propositions statics_;
	set<Proposition> statics_set_;
	Propositions inits_;
	DependGraph dpg_;
private:
	Relations relations_;
	vector<Derivation> derivations_;
	vector<Derivation> static_derivations_;
	vector<Derivation> nonstatic_derivations_;	
	vector<vector<vector<int> > > non_der_var_values_;
	Propositions bases_;
	Propositions inputs_;

	void getSubgoalSequence(vector<vector<vector<pair<int, int> > > > & var_candidates);

	// get by DPG
	vector<int> static_heads_;

	void getStaticRelation();
	void markNonStatic(int index, vector<int> & mark);
};

#endif
