#ifndef PROVER_H
#define PROVER_H

#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <string>
#include <cmath>
#include <time.h>
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
	static vector<clock_t> time;

	Propositions bases_;
	Propositions inputs_;
	Propositions roles_;
	//vector<int> key_head_;
	Propositions statics_;
	Propositions partly_statics_;
	unordered_set<Proposition, hash_Proposition> statics_set_;
	map<int, vector<int> > head_statics_;
	Propositions inits_;
	DependGraph dpg_;

	Prover(){}
	Prover(Relations relations);	
	void generateTrueProps(Propositions &true_props, int start_stra, int end_stra);
	
private:
	Relations relations_;
	vector<Derivation> derivations_;
	vector<Derivation> static_derivations_;
	vector<Derivation> nonstatic_derivations_;	
	vector<vector<vector<vector<int> > > > der_multiple_combinations_;
	vector<vector<vector<vector<int> > > > der_multiple_not_combinations_;
	vector<vector<vector<vector<int> > > > der_multiple_combinations_indexes_;
	vector<vector<vector<int> > > der_multiple_not_combinations_indexes_;
	void getSubgoalSequence(vector<vector<vector<pair<int, int> > > > & var_candidates);
	vector<vector<vector<int> > > getStaticCombinationsIndexes(vector<vector<vector<int> > > &multiple_combinations);
	vector<vector<int> > getStaticNotCombinationsIndexes(vector<vector<vector<int> > > &multiple_not_combinations);
	vector<int> static_heads_; // get by DPG

	void prepareStaticRelation();
	void markNonStatic(int index, vector<int> & mark);

};

#endif
