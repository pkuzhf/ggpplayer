#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <algorithm>
#include <unordered_set>

#include "prover.h"
#include "relation.h"
#include "reader.h"
#include "dependgraph.h"
#include "time.h"
using namespace std;

vector<clock_t> Prover::time = vector<clock_t>(16, 0);

Prover::Prover(Relations relations) {
	relations_ = relations;
	for (int i = 0; i < relations_.size(); ++i) {
		if (relations_[i].head_ == r_derivation) {
			Derivation d = relations_[i].toDerivation();
			d.prepareVariables();
			derivations_.push_back(d);			
		}
	}
	
	dpg_.buildGraph(derivations_);  // build graph for the first time
	prepareStaticRelation();
	Propositions true_rs;
	for(int i = 0 ; i < relations_.size(); ++i){
		if (relations_[i].head_ != r_derivation) {
			true_rs.push_back(relations_[i].toProposition());
			if (find(static_heads_.begin(), static_heads_.end(), relations_[i].head_) == static_heads_.end()) {
				partly_statics_.push_back(relations_[i].toProposition());
			}
		}
	}
	der_multiple_combinations_.resize(derivations_.size(), vector<vector<vector<int> > >());
	der_multiple_not_combinations_.resize(derivations_.size(), vector<vector<vector<int> > >());
	der_multiple_combinations_indexes_.resize(derivations_.size(), vector<vector<vector<int> > >());
	der_multiple_not_combinations_indexes_.resize(derivations_.size(), vector<vector<int> >());
	generateTrueProps(true_rs, 0, dpg_.stra_deriv_.size() - 1);
	der_multiple_combinations_.clear();
	der_multiple_not_combinations_.clear();
	der_multiple_combinations_indexes_.clear();
	der_multiple_not_combinations_indexes_.clear();
	for(int i = 0; i < true_rs.size(); ++i){
		if (true_rs[i].head_ == r_init) {
			inits_.push_back(true_rs[i]);
		} else if (true_rs[i].head_ == r_base) {
			bases_.push_back(true_rs[i]);			
		} else if (true_rs[i].head_ == r_input) {
			inputs_.push_back(true_rs[i]);			
		} else if (true_rs[i].head_ == r_role) {
			roles_.push_back(true_rs[i]);
			statics_.push_back(true_rs[i]);	
			statics_set_.insert(true_rs[i]);
			head_statics_[true_rs[i].head_].push_back(statics_.size() - 1);
		} else if (find(static_heads_.begin(), static_heads_.end(), true_rs[i].head_) != static_heads_.end()) {
			statics_.push_back(true_rs[i]);	
			statics_set_.insert(true_rs[i]);
			head_statics_[true_rs[i].head_].push_back(statics_.size() - 1);
		}
	}
	sort(roles_.begin(), roles_.end());
	// add key_head which isn't in base sentence
	/*for(int i = 0 ; i < relations_.size(); ++i){
		if(relations_[i].type_ == r_base){
			int temp = relations_[i].items_[0].head_;
			if(find(key_head_.begin(), key_head_.end(), temp) == key_head_.end()){
				key_head_.push_back(temp);
			}
		} else if(relations_[i].type_ == r_derivation && (relations_[i].items_[0].type_ == r_next)){
			int temp = relations_[i].items_[0].items_[0].head_;
			if(find(key_head_.begin(), key_head_.end(), temp) == key_head_.end()){
				key_head_.push_back(temp);
			}
		} else if(relations_[i].type_ == r_role){
			roles_.push_back(relations_[i].toProposition());
		}
	}*/

	for (int i = 0; i < nonstatic_derivations_.size(); ++i) {
		vector<vector<vector<int> > > multiple_combinations;
		vector<vector<vector<int> > > multiple_not_combinations;
		Derivation &d = nonstatic_derivations_[i];
		for (int j = 0; j < d.subgoals_.size(); ++j) {
			Proposition &subgoal = d.subgoals_[j];
			if (subgoal.head_ == r_not) {
				if (find(static_heads_.begin(), static_heads_.end(), subgoal.items_[0].head_) == static_heads_.end()) {
					continue;
				}
				vector<vector<int> > not_combinations;
				vector<int> ps = head_statics_[subgoal.items_[0].head_];
				for (int k = 0; k < ps.size(); ++k) {
					vector<int> variables;
					vector<int> values;
					if (statics_[ps[k]].matches(subgoal.items_[0], variables, values)) {
						not_combinations.push_back(getCombination(d.variables_, variables, values));
					}
				}
				if (not_combinations.size() > 0) {
					multiple_not_combinations.push_back(not_combinations);
				}
			} else if (subgoal.head_ != r_distinct) {				
				if (find(static_heads_.begin(), static_heads_.end(), subgoal.head_) == static_heads_.end()) {
					continue;
				}
				vector<vector<int> > combinations;
				vector<int> ps = head_statics_[subgoal.head_];
				for (int k = 0; k < ps.size(); ++k) {
					vector<int> variables;
					vector<int> values;
					if (statics_[ps[k]].matches(subgoal, variables, values)) {
						combinations.push_back(getCombination(d.variables_, variables, values));
					}
				}
				if (combinations.size() > 0) {
					multiple_combinations.push_back(combinations);
				}
			}
		}
		der_multiple_combinations_.push_back(multiple_combinations);
		der_multiple_combinations_indexes_.push_back(getStaticCombinationsIndexes(multiple_combinations));
		der_multiple_not_combinations_.push_back(multiple_not_combinations);
		der_multiple_not_combinations_indexes_.push_back(getStaticNotCombinationsIndexes(multiple_not_combinations));
		for (Propositions::iterator j = d.subgoals_.begin(); j != d.subgoals_.end(); ) {
			if ((j->head_ != r_not && find(static_heads_.begin(), static_heads_.end(), j->head_) != static_heads_.end())
				|| (j->head_ == r_not && find(static_heads_.begin(), static_heads_.end(), j->items_[0].head_) != static_heads_.end())) {
				j = d.subgoals_.erase(j);
			} else {
				++j;
			}
		}
	}

	DependGraph dpg2;
	dpg2.buildGraph(nonstatic_derivations_); 
	dpg_ = dpg2;
}

vector<vector<vector<int> > > Prover::getStaticCombinationsIndexes(vector<vector<vector<int> > > &multiple_combinations) {
	vector<vector<vector<int> > > multiple_combinations_indexes;
	for (int i = 0; i < multiple_combinations.size(); ++i) {
		vector<int> keys = getKeys(multiple_combinations[i][0]);
		vector<vector<int> > combinations_indexes;
		vector<int> idx;
		for (int j = 0; j < multiple_combinations[i].size(); ++j) {
			idx.push_back(j);
		}
		combinations_indexes.push_back(idx);
		for (int j = 1; j < (1 << keys.size()); ++j) {
			vector<int> tmp_keys;
			for (int k = 0; k < keys.size(); ++k) {
				if ((j >> (keys.size() - 1 - k) & 1) == 1) {
					tmp_keys.push_back(keys[k]);
				}
			}
			combinations_indexes.push_back(sortCombinations(multiple_combinations[i], tmp_keys));
		}
		multiple_combinations_indexes.push_back(combinations_indexes);
	}
	return multiple_combinations_indexes;
}

vector<vector<int> > Prover::getStaticNotCombinationsIndexes(vector<vector<vector<int> > > &multiple_not_combinations) {
	vector<vector<int> > multiple_not_combinations_indexes;
	for (int i = 0; i < multiple_not_combinations.size(); ++i) {
		vector<int> keys = getKeys(multiple_not_combinations[i][0]);
		multiple_not_combinations_indexes.push_back(sortCombinations(multiple_not_combinations[i], keys));
	}
	return multiple_not_combinations_indexes;
}

int Prover::compareCombination(vector<int> &comb_a, vector<int> &comb_b, vector<int> &keys) {
	for (int i = 0; i < keys.size(); ++i) {
		if (comb_a[keys[i]] < comb_b[keys[i]]) {
			return -1;
		} else if (comb_a[keys[i]] > comb_b[keys[i]]) {
			return 1;
		}
	}
	return 0;
}

void Prover::quickSortCombinations(vector<vector<int> > &combinations, vector<int> &keys, vector<int> &idx, int left, int right) {
	if (left >= right) return;
	//int pos = rand() % (right - left + 1) + left;
	int pos = (left + right) / 2;
	int x = idx[pos];
	idx[pos] = idx[left];
	int i = left;
	int j = right;
	while (i < j) {
		while (i < j && compareCombination(combinations[x], combinations[idx[j]], keys) <= 0) {
			--j;
		}
		if (i < j) {
			idx[i] = idx[j];
			++i;
		}
		while (i < j && compareCombination(combinations[idx[i]], combinations[x], keys) == -1) {
			++i;
		}
		if (i < j) {
			idx[j] = idx[i];
			--j;
		}
	}
	idx[i] = x;
	quickSortCombinations(combinations, keys, idx, left, i - 1);
	quickSortCombinations(combinations, keys, idx, i + 1, right);
}

vector<int> Prover::sortCombinations(vector<vector<int> > &combinations, vector<int> &keys) {
	clock_t s1 = clock();
	vector<int> idx;
	idx.reserve(combinations.size());
	for (int i = 0; i < combinations.size(); ++i) {
		idx.push_back(i);
	}
	if (keys.size() > 0) {
		quickSortCombinations(combinations, keys, idx, 0, idx.size() - 1);
	}
	time[1] += clock() - s1;
	return idx;
}

vector<vector<int> > Prover::delNotCombinations(vector<vector<int> > &c, vector<vector<int> > &not_c, vector<int> &idx_c, vector<int> &idx_not_c, vector<int> &keys) {
	vector<vector<int> > ret;
	ret.reserve(c.size());
	int i = 0;
	int j = 0;
	
	while (i < idx_c.size() && j < idx_not_c.size()) {
		int compare = compareCombination(c[idx_c[i]], not_c[idx_not_c[j]], keys);
		if (compare == -1) {
			ret.push_back(c[idx_c[i]]);
			++i;
		} else if (compare == 1) {
			++j;
		} else {
			++i;
		}
	}
	while (i < idx_c.size()) {
		ret.push_back(c[idx_c[i]]);
		++i;
	}
	return ret;
}

vector<vector<int> > Prover::mergeTwoCombinations(vector<vector<int> > &a, vector<vector<int> > &b, vector<int> &idx_a, vector<int> &idx_b, vector<int> &keys, int size) {
	clock_t s3 = clock();
	vector<vector<int> > ret;
	ret.reserve(size);
	int a_begin = 0;
	int b_begin = 0;
	while (a_begin < idx_a.size() && b_begin < idx_b.size()) {
		int compare = compareCombination(a[idx_a[a_begin]], b[idx_b[b_begin]], keys);
		if (compare == -1) {
			++a_begin;
		} else if (compare == 1) {
			++b_begin;
		} else {
			int a_end = a_begin;
			int b_end = b_begin;
			while (a_end < idx_a.size() && compareCombination(a[idx_a[a_begin]], a[idx_a[a_end]], keys) == 0) {
				++a_end;
			}
			while (b_end < idx_b.size() && compareCombination(b[idx_b[b_begin]], b[idx_b[b_end]], keys) == 0) {
				++b_end;
			}
			for (int i = a_begin; i < a_end; ++i) {
				for (int j = b_begin; j < b_end; ++j) {
					vector<int> &c1 = a[idx_a[i]];
					vector<int> &c2 = b[idx_b[j]];
					int size = c1.size();
					vector<int> c3(size, -1);
					for (int k = 0; k < size; ++k) {
						if (c1[k] != -1) c3[k] = c1[k];
						if (c2[k] != -1) c3[k] = c2[k];
					}
					ret.push_back(c3);
				}
			}
			a_begin = a_end;
			b_begin = b_end;
		}
	}
	time[3] += clock() - s3;
	return ret;
}

long long Prover::calcCombineCost(
	vector<vector<int> > &a, 
	vector<vector<int> > &b, 
	vector<int> &keys,
	vector<int> &idx_a,
	vector<int> &idx_b) {

	long long ret = 0;
	int a_begin = 0;
	int b_begin = 0;
	while (a_begin < idx_a.size() && b_begin < idx_b.size()) {
		int compare = compareCombination(a[idx_a[a_begin]], b[idx_b[b_begin]], keys);
		if (compare == -1) {
			++a_begin;
		} else if (compare == 1) {
			++b_begin;
		} else {
			int a_end = a_begin;
			int b_end = b_begin;
			while (a_end < idx_a.size() && compareCombination(a[idx_a[a_begin]], a[idx_a[a_end]], keys) == 0) {
				++a_end;
			}
			while (b_end < idx_b.size() && compareCombination(b[idx_b[b_begin]], b[idx_b[b_end]], keys) == 0) {
				++b_end;
			}
			ret += (a_end - a_begin) * (b_end - b_begin);			
			a_begin = a_end;
			b_begin = b_end;
		}
	}
	return ret;
}

vector<int> Prover::getCombination(vector<int> all_variables, vector<int> &variables, vector<int> values) {
	vector<int> ret(all_variables.size(), -1);
	for (int i = 0; i < all_variables.size(); ++i) {
		for (int j = 0; j < variables.size(); ++j) {
			if (all_variables[i] == variables[j]) {
				ret[i] = values[j];
				break;
			}
		}
	}
	return ret;
}

vector<int> Prover::getKeys(vector<int> &combination) {
	vector<int> ret;
	for (int i = 0; i < combination.size(); ++i) {
		if (combination[i] != -1) {
			ret.push_back(i);
		}
	}
	return ret;
}

vector<int> Prover::getCommonKeys(vector<int> &a, vector<int> &b, int &idxa, int &idxb) {
	vector<int> ret;
	idxa = 0;
	idxb = 0;
	for (int i = 0; i < a.size(); ++i) {
		if (a[i] != -1) {
			idxa *= 2;
		}
		if (b[i] != -1) {
			idxb *= 2;
		}
		if (a[i] != -1 && b[i] != -1) {
			ret.push_back(i);
			++idxa;
			++idxb;
		}		
	}
	return ret;
}

void Prover::uniqCombinations(vector<vector<int> > combinations) {
	clock_t s4 = clock();
	vector<vector<int> > new_c;
	vector<int> idx;
	vector<int> keys;
	if (combinations.size() > 0) {
		keys = getKeys(combinations[0]);
	}
	idx = sortCombinations(combinations, keys);
	bool has_duplicates = false;
	for (int i = 1; i < idx.size(); ++i) {
		if (compareCombination(combinations[idx[i]], combinations[idx[i - 1]], keys) == 0) {
			has_duplicates = true;
			break;
		}
	}
	if (has_duplicates) {		
		for (int i = 0; i < idx.size(); ++i) {
			if (i == 0 || compareCombination(combinations[idx[i]], combinations[idx[i - 1]], keys) != 0) {
				new_c.push_back(combinations[idx[i]]);
			}
		}
		combinations = new_c;
	}
	time[4] += clock() - s4;
}

vector<vector<int> > & Prover::mergeMultipleCombinations(
	vector<vector<vector<int> > > &multiple_combinations, 
	vector<vector<vector<int> > > &multiple_not_combinations,
	vector<vector<vector<int> > > &multiple_combinations_indexes,
	vector<vector<int> > &multiple_not_combinations_indexes,
	vector<pair<int, int> > &variable_distincts,
	vector<pair<int, int> > &constant_distincts) {	
		
	clock_t s2 = clock();
	int size = multiple_combinations.size();
	if (size == 0) {
		multiple_combinations.push_back(vector<vector<int> >());
		time[2] += clock() - s2;
		return multiple_combinations[multiple_combinations.size() - 1];
	}
	for (int i = 0; i < size; ++i) {
		vector<vector<int> > &c = multiple_combinations[i];		
		for (vector<vector<int> >::iterator j = c.begin(); j != c.end(); ) {
			bool deleted = false;
			for (int k = 0; k < constant_distincts.size() && !deleted; ++k) {
				pair<int, int> d = constant_distincts[k];
				if ((*j)[d.first] != -1 && (*j)[d.first] == d.second) {
					deleted = true;					
				}
			}
			for (int k = 0; k < variable_distincts.size() && !deleted; ++k) {
				pair<int, int> d = variable_distincts[k];
				if ((*j)[d.first] != -1 && (*j)[d.second] != -1 && (*j)[d.first] == (*j)[d.second]) {
					deleted = true;
				}
			}
			if (deleted) {
				j = c.erase(j);
			} else {
				++j;
			}
		}
	}
	
	vector<vector<long long> > combine_cost(size, vector<long long>(size, -1));
	vector<vector<vector<int> > > keys(size, vector<vector<int> >(size, vector<int>()));
	vector<vector<pair<vector<int>, vector<int> > > > idxes(size, vector<pair<vector<int>, vector<int> > >(size, pair<vector<int>, vector<int> >()));	
	for (int i = 0; i < size; ++i) {
		for (int j = i + 1; j < size; ++j) {
			vector<vector<int> > &a = multiple_combinations[i];
			vector<vector<int> > &b = multiple_combinations[j];
			int idxa;
			int idxb;
			if (a.size() > 0 && b.size() > 0) {
				keys[i][j] = getCommonKeys(a[0], b[0], idxa, idxb);
			}
			if (i < multiple_combinations_indexes.size()) {
				idxes[i][j].first = multiple_combinations_indexes[i][idxa];
			} else {
				idxes[i][j].first = sortCombinations(a, keys[i][j]);
			}
			if (j < multiple_combinations_indexes.size()) {
				idxes[i][j].second = multiple_combinations_indexes[j][idxb];
			} else {
				idxes[i][j].second = sortCombinations(b, keys[i][j]);
			}
			combine_cost[i][j] = calcCombineCost(a, b, keys[i][j], idxes[i][j].first, idxes[i][j].second);
			if (combine_cost[i][j] == 0) {
				multiple_combinations.push_back(vector<vector<int> >());
				time[2] += clock() - s2;
				return multiple_combinations[multiple_combinations.size() - 1];;
			}
		}
	}

	vector<bool> deleted(size, false);	
	vector<vector<int> > not_keys(multiple_not_combinations.size(), vector<int>());
	vector<vector<int> > not_idxes(multiple_not_combinations.size(), vector<int>());
	vector<bool> not_used(multiple_not_combinations.size(), false);
	for (int i = 0; i < multiple_not_combinations.size(); ++i) {
		if (multiple_not_combinations[i].size() == 0) {
			not_used[i] = true;
		} else {
			for (int j = 0; j < multiple_not_combinations[i][0].size(); ++j) {
				if (multiple_not_combinations[i][0][j] != -1) {
					not_keys[i].push_back(j);
				}
			}
			if (i < multiple_not_combinations_indexes.size()) {
				not_idxes[i] = multiple_not_combinations_indexes[i];
			} else {
				not_idxes[i] = sortCombinations(multiple_not_combinations[i], not_keys[i]);
			}
		}		
	}
	while(true) {
		long long min_cost = -1;
		int c1 = -1;
		int c2 = -1;
		for (int j = 0; j < size; ++j) {
			if (deleted[j]) continue;
			for (int k = j + 1; k < size; ++k) {
				if (deleted[k]) continue;
				if (combine_cost[j][k] < min_cost || min_cost == -1) {
					min_cost = combine_cost[j][k];
					c1 = j;
					c2 = k;
				}
			}
		}
		if (min_cost == -1) {
			break;
		}
		multiple_combinations.push_back(mergeTwoCombinations(multiple_combinations[c1], multiple_combinations[c2], idxes[c1][c2].first, idxes[c1][c2].second, keys[c1][c2], min_cost));
		keys.push_back(vector<vector<int> >(size + 1, vector<int>()));
		idxes.push_back(vector<pair<vector<int>, vector<int> > > (size + 1, pair<vector<int>, vector<int> >()));
		combine_cost.push_back(vector<long long>(size + 1, 0));
		deleted.push_back(false);

		deleted[c1] = true;
		deleted[c2] = true;		
		for (int j = 0; j < size; ++j) {
			if (deleted[j]) continue;
			int x = j;
			int y = multiple_combinations.size() - 1;
			vector<vector<int> > &a = multiple_combinations[x];
			vector<vector<int> > &b = multiple_combinations[y];
			keys[x].push_back(vector<int>());	
			int idxa;
			int idxb;
			if (a.size() > 0 && b.size() > 0) {
				keys[x][y] = getCommonKeys(a[0], b[0], idxa, idxb);
			}
			idxes[x].push_back(pair<vector<int>, vector<int> >());
			if (x < multiple_combinations_indexes.size()) {
				idxes[x][y].first = multiple_combinations_indexes[x][idxa];
			} else {
				idxes[x][y].first = sortCombinations(a, keys[x][y]);
			}
			idxes[x][y].second = sortCombinations(b, keys[x][y]);
			combine_cost[x].push_back(0);
			combine_cost[x][y] = calcCombineCost(a, b, keys[x][y], idxes[x][y].first, idxes[x][y].second);
			if (combine_cost[x][y] == 0) {
				multiple_combinations.push_back(vector<vector<int> >());
				time[2] += clock() - s2;
				return multiple_combinations[multiple_combinations.size() - 1];
			}
		}
		size = multiple_combinations.size();
	}
	
	vector<vector<int> > &ret = multiple_combinations[size - 1];
	for (int j = 0; j < multiple_not_combinations.size(); ++j) {
		if (not_used[j] || ret.size() == 0) continue;
		vector<vector<int> > &not_c = multiple_not_combinations[j];
		bool matched = true;				
		for (int k = 0; k < not_keys[j].size(); ++k) {
			if (not_c[0][not_keys[j][k]] != -1 && ret[0][not_keys[j][k]] == -1) {
				matched = false;
				break;						
			}
		}				
		if (matched) {
			vector<int> idx = sortCombinations(ret, not_keys[j]);
			ret = delNotCombinations(ret, not_c, idx, not_idxes[j], not_keys[j]);
			not_used[j] = true;
		}
	}
	for (vector<vector<int> >::iterator j = ret.begin(); j != ret.end();) {
		bool deleted = false;				
		for (int k = 0; k < variable_distincts.size() && !deleted; ++k) {
			pair<int, int> d = variable_distincts[k];
			if ((*j)[d.first] != -1 && (*j)[d.second] != -1 && (*j)[d.first] == (*j)[d.second]) {
				deleted = true;
			}
		}
		if (deleted) {
			j = ret.erase(j);
		} else {
			++j;
		}
	}
	time[2] += clock() - s2;
	return ret;
}

void Prover::prepareStaticRelation()
{
	vector<int> mark(dpg_.init_nodes_.size(), 0);	
	markNonStatic(dpg_.node_num_[r_does], mark);
	markNonStatic(dpg_.node_num_[r_true], mark);
	for(int i = 0 ; i < mark.size(); ++i){
		if(mark[i] == 0){
			static_heads_.push_back(dpg_.init_nodes_[i]);
		}
	}
	for(int i = 0 ; i < derivations_.size(); ++i){
		if(find(static_heads_.begin(), static_heads_.end(), derivations_[i].target_.head_) == static_heads_.end()){
			nonstatic_derivations_.push_back(derivations_[i]);
		}else {
			static_derivations_.push_back(derivations_[i]);
		}
	}
}

void Prover::markNonStatic(int index, vector<int> & mark)
{
	mark[index] = 1;
	for(int i = 0 ; i < dpg_.init_edges_out_[index].size(); ++i){
		if(mark[dpg_.init_edges_out_[index][i]] == 0){
			markNonStatic(dpg_.init_edges_out_[index][i], mark);
		}
	}
}

void Prover::generateTrueProps(Propositions &true_props, int start_stra, int end_stra) {
	clock_t s0 = clock();
	static int true_props_size = true_props.size() * 10;
	true_props.reserve(true_props_size * 2);		
	/*vector<string> input;
	for (int i = 0; i < true_props.size(); ++i) {
		input.push_back(true_props[i].toRelation().toString());
	}*/
	map<int, vector<int> > head_propositions;
	unordered_set<Proposition, hash_Proposition> true_props_set;	
	for (int i = 0; i < true_props.size(); ++i) {
		head_propositions[true_props[i].head_].push_back(i);		
		true_props_set.insert(true_props[i]);		
	}
	
	for (int i = start_stra; i <= end_stra; ++i) {
		vector<Derivation> derivations;
		vector<vector<vector<vector<int> > > > der_multiple_combinations;
		vector<vector<vector<vector<int> > > > der_multiple_not_combinations;
		vector<vector<vector<vector<int> > > > der_multiple_combinations_indexes;
		vector<vector<vector<int> > > der_multiple_not_combinations_indexes;
		vector<vector<pair<int, int> > > der_constant_distincts;
		vector<vector<pair<int, int> > > der_variable_distincts;		
		Propositions current_stratum_props;
		current_stratum_props.reserve(true_props_size / 5);
		clock_t s5 = clock();
		for (int j = 0; j < dpg_.stra_deriv_[i].size(); ++j) {
			//clock_t s8 = clock();
			//clock_t s6 = clock();
			int der_id = dpg_.stra_deriv_[i][j];
			Derivation &d = dpg_.derivations_[der_id];
			vector<int> current_stratum_subgoals;
			vector<vector<vector<int> > > multiple_combinations;
			vector<vector<vector<int> > > multiple_not_combinations;
			multiple_combinations = der_multiple_combinations_[der_id];
			multiple_not_combinations = der_multiple_not_combinations_[der_id];
			vector<pair<int, int> > constant_distincts;
			vector<pair<int, int> > variable_distincts;
			bool impossible = false;
			for (int k = 0; k < d.subgoals_.size(); ++k) {								
				Proposition &subgoal = d.subgoals_[k];
				if (subgoal.head_ == r_not) {					
					//not_subgoals.push_back(k);															
					vector<vector<int> > not_combinations;
					vector<int> &ps = head_propositions[subgoal.items_[0].head_];					
					for (int ii = 0; ii < ps.size(); ++ii) {
						vector<int> variables;
						vector<int> values;
						if (true_props[ps[ii]].matches(subgoal.items_[0], variables, values)) {																
							not_combinations.push_back(getCombination(d.variables_, variables, values));
						}
					}
					if (not_combinations.size() > 0) {
						multiple_not_combinations.push_back(not_combinations);
					}														
				} else if (subgoal.head_ == r_distinct) {					
					//distinct_subgoals.push_back(k);
					int first = -1;
					int second = -1;
					for (int ii = 0; ii < d.variables_.size(); ++ii) {
						if (subgoal.items_[0].is_variable_ && d.variables_[ii] == subgoal.items_[0].head_) {
							first = ii;
						}
						if (subgoal.items_[1].is_variable_ && d.variables_[ii] == subgoal.items_[1].head_) {
							second = ii;
						}
					}
					if (subgoal.items_[0].is_variable_ && subgoal.items_[1].is_variable_) {						
						variable_distincts.push_back(pair<int, int>(first, second));
					} else {
						if (subgoal.items_[0].is_variable_) {
							constant_distincts.push_back(pair<int, int>(first, subgoal.items_[1].head_));
						} else if (subgoal.items_[1].is_variable_) {
							constant_distincts.push_back(pair<int, int>(second, subgoal.items_[0].head_));
						}
					}					
				} else if (dpg_.node_stra_[dpg_.node_num_[subgoal.head_]] < i) { // lower stratum subgoals										
					//lower_stratum_subgoals.push_back(k);
					vector<vector<int> > combinations;
					vector<int> &ps = head_propositions[subgoal.head_];					
					for (int ii = 0; ii < ps.size(); ++ii) { // scan all true props to generate var-value maps						
						Proposition &p = true_props[ps[ii]];
						vector<int> variables;
						vector<int> values;
						if (subgoal.matches(p, variables, values)) {							
							combinations.push_back(getCombination(d.variables_, variables, values));							
						}						
					}
					if (combinations.size() == 0) { // size of combinations should be greater than 0
						impossible = true;	
						break;
					}					
					multiple_combinations.push_back(combinations);					
				} else {
					current_stratum_subgoals.push_back(k);
				}
			}
			//time[6] += clock() - s6;
			//clock_t s7 = clock();
			if (impossible) {
				//time[8] += clock() - s8;
				continue;
			}
			if (multiple_combinations.size() == 0) {
				multiple_combinations.push_back(vector<vector<int> >(1, vector<int>())); // in case there are only 'not' subgoals
			}
			for (int k = der_multiple_combinations_[der_id].size(); k < multiple_combinations.size(); ++k) {
				uniqCombinations(multiple_combinations[k]);
			}
			for (int k = der_multiple_not_combinations_[der_id].size(); k < multiple_not_combinations.size(); ++k) {
				uniqCombinations(multiple_not_combinations[k]);
			}						
			if (current_stratum_subgoals.size() == 0) {				
				vector<vector<int> > new_combinations = mergeMultipleCombinations(multiple_combinations, 
					multiple_not_combinations, der_multiple_combinations_indexes_[der_id], der_multiple_not_combinations_indexes_[der_id], variable_distincts, constant_distincts);				
				for (int k = 0; k < new_combinations.size(); ++k) {					
					Proposition p = d.target_;
					p.replaceVariables(d.variables_, new_combinations[k]);					
					if (true_props_set.find(p) == true_props_set.end()) {																	
						head_propositions[p.head_].push_back(true_props.size());												
						true_props.push_back(p);
						true_props_set.insert(p);						
						current_stratum_props.push_back(p);						
					}
				}				
			} else {
				Derivation d2;
				d2.target_ = d.target_;
				d2.variables_ = d.variables_;
				for (int ii = 0; ii < current_stratum_subgoals.size(); ++ii) {
					d2.subgoals_.push_back(d.subgoals_[current_stratum_subgoals[ii]]);
				}
				derivations.push_back(d2);
				der_multiple_combinations.push_back(multiple_combinations);
				der_multiple_not_combinations.push_back(multiple_not_combinations);
				der_multiple_combinations_indexes.push_back(der_multiple_combinations_indexes_[der_id]);
				der_multiple_not_combinations_indexes.push_back(der_multiple_not_combinations_indexes_[der_id]);
				der_constant_distincts.push_back(constant_distincts);
				der_variable_distincts.push_back(variable_distincts);
			}
			//time[7] += clock() - s7;
			//time[8] += clock() - s8;
			
		}
		time[5] += clock() - s5;
		for (int j = 0; j < current_stratum_props.size(); ++j) { // size of current_stratum_props would be changed in the loop
			Proposition true_p = current_stratum_props[j];			
			for (int k = 0; k < derivations.size(); ++k) {
				Derivation &d = derivations[k];				
				for (int ii = 0; ii < d.subgoals_.size(); ++ii) {
					Proposition &subgoal = d.subgoals_[ii];
					vector<int> variables;
					vector<int> values;
					if (subgoal.matches(true_p, variables, values)) {						
						vector<int> c = getCombination(d.variables_, variables, values);
						vector<vector<vector<int> > > new_multiple_combinations = der_multiple_combinations[k];
						new_multiple_combinations.push_back(vector<vector<int> >(1, c));
						if (d.subgoals_.size() == 1) {
							vector<vector<int>> new_combinations = mergeMultipleCombinations(new_multiple_combinations, der_multiple_not_combinations[k], 
								der_multiple_combinations_indexes[k], der_multiple_not_combinations_indexes[k], der_variable_distincts[k], der_constant_distincts[k]);							
							for (int ii = 0; ii < new_combinations.size(); ++ii) {
								Proposition p = d.target_;
								p.replaceVariables(d.variables_, new_combinations[ii]);
								if (true_props_set.find(p) == true_props_set.end()) {									
									head_propositions[p.head_].push_back(true_props.size());
									true_props.push_back(p);									
									true_props_set.insert(p);									
									current_stratum_props.push_back(p);									
								}
							}
						} else {
							Derivation d2;
							d2.target_ = d.target_;
							d2.variables_ = d.variables_;
							for (int ii = 0; ii < d.subgoals_.size(); ++ii) {
								if (ii != k) {
									d2.subgoals_.push_back(d.subgoals_[ii]);
								}											
							}							
							derivations.push_back(d2);
							der_multiple_combinations.push_back(new_multiple_combinations);
							der_multiple_not_combinations.push_back(der_multiple_not_combinations[k]);
							der_constant_distincts.push_back(der_constant_distincts[k]);
							der_variable_distincts.push_back(der_variable_distincts[k]);
						}						
					}					
				}
			}
		}				
	}	
	/*vector<string> output;
	for (int i = 0; i < true_props.size(); ++i) {
		output.push_back(true_props[i].toRelation().toString());
	}	*/
	true_props_size = true_props.size();	
	time[0] += clock() - s0;
}
