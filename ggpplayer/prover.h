#ifndef PROVER_H
#define PROVER_H

#include <vector>
#include <set>
#include <unordered_set>
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
	static vector<int> time;

	Propositions bases_;
	Propositions inputs_;
	Propositions roles_;

	vector<int> key_head_;

	Prover(Relations relations);	
	void generateTrueProps(Propositions &true_props, int start_stra, int end_stra);
	Propositions statics_;
	unordered_set<Proposition, hash_Proposition> statics_set_;
	map<int, vector<int> > head_statics_;
	Propositions inits_;
	DependGraph dpg_;
private:
	Relations relations_;
	vector<Derivation> derivations_;
	vector<Derivation> static_derivations_;
	vector<Derivation> nonstatic_derivations_;	
	vector<vector<vector<vector<int> > > > der_multiple_combinations_;	
	vector<vector<vector<vector<int> > > > der_multiple_not_combinations_;	
	void getSubgoalSequence(vector<vector<vector<pair<int, int> > > > & var_candidates);
	vector<int> static_heads_; // get by DPG

	void prepareStaticRelation();
	void markNonStatic(int index, vector<int> & mark);

	vector<int> getCombination(vector<int> all_variables, vector<int> &variables, vector<int> values);
	vector<int> getKeys(vector<int> &combination);
	void uniqCombinations(vector<vector<int> > combinations);
	int compareCombination(vector<int> &comb_a, vector<int> &comb_b, vector<int> &keys);
	void quickSortCombinations(vector<vector<int> > &combinations, vector<int> &keys, vector<int> &idx, int left, int right);
	vector<int> sortCombinations(vector<vector<int> > &combinations, vector<int> &keys);
	vector<vector<int> > mergeTwoCombinations(vector<vector<int> > &a, vector<vector<int> > &b, vector<int> &idx_a, vector<int> &idx_b, vector<int> &keys, int size);
	vector<vector<int> > delNotCombinations(vector<vector<int> > &c, vector<vector<int> > &not_c, vector<int> &idx_c, vector<int> &idx_not_c, vector<int> &keys);
	long long calcCombineCost(vector<vector<int> > &a, vector<vector<int> > &b, vector<int> &keys, vector<int> &idx_a, vector<int> &idx_b);
	vector<vector<int> > & mergeMultipleCombinations(vector<vector<vector<int> > > &multiple_combinations, vector<vector<vector<int> > > &multiple_not_combinations, 
		vector<pair<int, int> > &variable_distincts, vector<pair<int, int> > &constant_distincts);
};

#endif
