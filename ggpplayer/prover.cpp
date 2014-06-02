#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <algorithm>

#include "prover.h"
#include "relation.h"
#include "reader.h"
#include "dependgraph.h"
#include "time.h"
using namespace std;

int Prover::generate_time = 0;
int Prover::time1 = 0;
int Prover::time2 = 0;
int Prover::time3 = 0;
int Prover::time4 = 0;
int Prover::time5 = 0;
int Prover::time6 = 0;
int Prover::time7 = 0;
int Prover::time8 = 0;
int Prover::time9 = 0;
int Prover::time10 = 0;
int Prover::time11 = 0;
int Prover::time12 = 0;
int Prover::time13 = 0;
int Prover::time14 = 0;
int Prover::time15 = 0;
int Prover::time16 = 0;
int Prover::time17 = 0;
int Prover::time18 = 0;
int Prover::time19 = 0;
int Prover::time20 = 0;
int Prover::time21 = 0;


Prover::Prover(Relations relations) : relations_(relations) {
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
		if(relations_[i].head_ == r_role 
			|| relations_[i].head_ == r_init
			|| relations_[i].head_ >= relation_type_num){
				true_rs.push_back(relations_[i].toProposition());
		}
	}
	true_rs = generateTrueProps(true_rs, 0, dpg_.stra_deriv_.size() - 1);
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
		} else if (find(static_heads_.begin(), static_heads_.end(), true_rs[i].head_) != static_heads_.end()) {
			statics_.push_back(true_rs[i]);	
			statics_set_.insert(true_rs[i]);
		}
	}
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
	/*for (int i = 0; i < nonstatic_derivations_.size(); ++i) {
		vector<vector<int> > tmp;
		non_der_var_values_.push_back(tmp);
		Derivation &d = nonstatic_derivations_[i];		
		vector<vector<vector<int> > > multiple_combinations;
		vector<int> idx;
		bool impossible = false;		
		for (int j = 0; j < d.subgoals_.size(); ++j) {
			if (find(static_heads_.begin(), static_heads_.end(), d.subgoals_[j].head_) == static_heads_.end()) {
				continue;
			}
			vector<vector<int> > combinations;
			Proposition &subgoal = d.subgoals_[j];			
			for (int ii = 0; ii < statics_.size(); ++ii) {				
				vector<int> variables;
				vector<int> values;				
				if (subgoal.matches(statics_[ii], variables, values)) {					
					bool duplicated = false;
					for (int jj = 0; jj < combinations.size(); ++jj) {
						bool equal = true;
						vector<int> &c = combinations[jj];						
						for (int kk = 0; kk < values.size() && equal; ++kk) {
							if (c[kk] != values[kk]) {
								equal = false;
							}
						}
						if (equal) {
							duplicated = true;
							break;
						}
					}
					if (!duplicated) {						
						combinations.push_back(values);
					}
				}
			}
			if (combinations.size() == 0) {
				impossible = true;
				break;
			}
			idx.push_back(0);
			vector<vector<int> > combinations2;
			vector<int> variables;
			subgoal.getVariables(variables);
			for (int ii = 0; ii < combinations.size(); ++ii) {
				vector<int> &c = combinations[ii];
				vector<int> c2;
				for (int jj = 0; jj < d.variables_.size(); ++jj) {
					int val = -1;
					for (int kk = 0; kk < variables.size(); ++kk) {
						if (variables[kk] == d.variables_[jj]) {
							val = c[kk];
							break;
						}
					}
					c2.push_back(val);
				}
				combinations2.push_back(c2);
			}
			multiple_combinations.push_back(combinations2);			
		}
		if (idx.size() == 0) {
			continue;
		}
		int k = 0;
		while (true) {
			while (k >= 0 && idx[k] == multiple_combinations[k].size()) {
				idx[k] = 0;
				--k;
				if (k >= 0) {
					++idx[k];
				}
			}
			if (k < 0) {
				break;
			}
			bool combined = true;
			vector<int> &c1 = multiple_combinations[k][idx[k]];			
			for (int ii = 0; ii < k && combined; ++ii) {
				vector<int> &c2 = multiple_combinations[ii][idx[ii]];
				int size = d.variables_.size();
				for (int jj = 0; jj < size && combined; ++jj) {
					if (c1[jj] != -1 && c2[jj] != -1 && c1[jj] != c2[jj]) {							
						combined = false;
					}
				}
			}
			
			if (combined) {															
				if (k == multiple_combinations.size() - 1) {
					int size = d.variables_.size();
					vector<int> m(size, -1);
					for (int ii = 0; ii <= k; ++ii) {
						vector<int> &c = multiple_combinations[ii][idx[ii]];					
						for (int jj = 0; jj < size; ++jj) {
							m[jj] = c[jj] == -1 ? m[jj] : c[jj];
						}										
					}
					non_der_var_values_[i].push_back(m);
					++idx[k];
				} else {						
					++k;
				}
			} else {
				++idx[k];						
			}			
		}
		for (Propositions::iterator j = d.subgoals_.begin(); j != d.subgoals_.end(); ) {
			if (find(static_heads_.begin(), static_heads_.end(), j->head_) != static_heads_.end()) {
				j = d.subgoals_.erase(j);
			} else {
				++j;
			}
		}
	}	*/

	DependGraph dpg2;
	dpg2.buildGraph(nonstatic_derivations_); 
	dpg_ = dpg2;
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
	int pos = rand() % (right - left + 1) + left;
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
	vector<int> idx;
	for (int i = 0; i < combinations.size(); ++i) {
		idx.push_back(i);
	}
	quickSortCombinations(combinations, keys, idx, 0, idx.size() - 1);
	return idx;
}

vector<vector<int> > Prover::delNotCombinations(vector<vector<int> > &c, vector<vector<int> > &not_c, vector<int> &idx_c, vector<int> &idx_not_c, vector<int> &keys) {
	vector<vector<int> > ret;
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

vector<vector<int> > Prover::mergeTwoCombinations(vector<vector<int> > &a, vector<vector<int> > &b, vector<int> &idx_a, vector<int> &idx_b, vector<int> &keys) {

	vector<vector<int> > ret;
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

vector<int> getCommonKeys(vector<int> &a, vector<int> &b) {
	vector<int> ret;
	for (int i = 0; i < a.size(); ++i) {
		if (a[i] != -1 && b[i] != -1) {
			ret.push_back(i);
		}
	}
	return ret;
}

void Prover::uniqCombinations(vector<vector<int> > combinations) {
	vector<vector<int> > new_c;
	vector<int> idx;
	vector<int> keys;
	if (combinations.size() > 0) {
		keys = getKeys(combinations[0]);
	}
	idx = sortCombinations(combinations, keys);
	for (int i = 0; i < idx.size(); ++i) {
		if (i == 0 || compareCombination(combinations[idx[i]], combinations[idx[i - 1]], keys) != 0) {
			new_c.push_back(combinations[idx[i]]);
		}
	}
	combinations = new_c;
}

vector<vector<int> > Prover::mergeMultipleCombinations(
	vector<vector<vector<int> > > &multiple_combinations, 
	vector<vector<vector<int> > > &multiple_not_combinations,
	vector<pair<int, int> > &variable_distincts,
	vector<pair<int, int> > &constant_distincts) {

	int size = multiple_combinations.size();
	if (size == 0) {
		return vector<vector<int> >();
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
			if (a.size() > 0 && b.size() > 0) {
				keys[i][j] = getCommonKeys(a[0], b[0]);
			}
			idxes[i][j].first = sortCombinations(a, keys[i][j]);
			idxes[i][j].second = sortCombinations(b, keys[i][j]);
			combine_cost[i][j] = calcCombineCost(a, b, keys[i][j], idxes[i][j].first, idxes[i][j].second);			
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
			not_idxes[i] = sortCombinations(multiple_not_combinations[i], not_keys[i]);
		}		
	}
	for (int i = 0; i < size - 1; ++i) {
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
		vector<vector<int> > new_c;
		new_c = mergeTwoCombinations(multiple_combinations[c1], multiple_combinations[c2], idxes[c1][c2].first, idxes[c1][c2].second, keys[c1][c2]);		
		if (new_c.size() > 0) {
			for (int j = 0; j < multiple_not_combinations.size(); ++j) {
				if (not_used[j]) continue;
				vector<vector<int> > &not_c = multiple_not_combinations[j];
				bool matched = true;				
				for (int k = 0; k < not_keys[j].size(); ++k) {
					if (not_c[0][not_keys[j][k]] != -1 && new_c[0][not_keys[j][k]] == -1) {
						matched = false;
						break;						
					}
				}				
				if (matched) {
					vector<int> idx = sortCombinations(new_c, not_keys[j]);
					new_c = delNotCombinations(new_c, not_c, idx, not_idxes[j], not_keys[j]);
					not_used[j] = true;
				}
			}
			for (vector<vector<int> >::iterator j = new_c.begin(); j != new_c.end();) {
				bool deleted = false;				
				for (int k = 0; k < variable_distincts.size() && !deleted; ++k) {
					pair<int, int> d = variable_distincts[k];
					if ((*j)[d.first] != -1 && (*j)[d.second] != -1 && (*j)[d.first] == (*j)[d.second]) {
						deleted = true;
					}
				}
				if (deleted) {
					j = new_c.erase(j);
				} else {
					++j;
				}
			}
		}
		multiple_combinations[c1] = new_c;
		deleted[c2] = true;
		for (int j = 0; j < size; ++j) {
			if (deleted[j] || j == c1) continue;
			int x = min(j, c1);
			int y = max(j, c1);			
			vector<vector<int> > &a = multiple_combinations[x];
			vector<vector<int> > &b = multiple_combinations[y];			
			keys[x][y].clear();
			if (a.size() > 0 && b.size() > 0) {
				keys[x][y] = getCommonKeys(a[0], b[0]);
			}
			idxes[x][y].first = sortCombinations(a, keys[x][y]);
			idxes[x][y].second = sortCombinations(b, keys[x][y]);
			combine_cost[x][y] = calcCombineCost(a, b, keys[x][y], idxes[x][y].first, idxes[x][y].second);	
		}
	}
	for (int i = 0; i < size; ++i) {
		if (!deleted[i]) {
			return multiple_combinations[i];
		}
	}
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

Propositions Prover::generateTrueProps(Propositions true_props, int start_stra, int end_stra) {	
	true_props.insert(true_props.begin(), statics_.begin(), statics_.end());
	vector<string> input;
	for (int i = 0; i < true_props.size(); ++i) {
		input.push_back(true_props[i].toRelation().toString());
	}
	map<int, vector<int> > head_propositions;
	set<Proposition> true_props_set;
	for (int i = 0; i < true_props.size(); ++i) {
		if (head_propositions.find(true_props[i].head_) == head_propositions.end()) {
			vector<int> rs;
			head_propositions[true_props[i].head_] = rs;
		}
		head_propositions[true_props[i].head_].push_back(i);
		true_props_set.insert(true_props[i]);
	}
	for (int i = start_stra; i <= end_stra; ++i) {
		vector<Derivation> derivations;
		vector<vector<vector<vector<int> > > > der_multiple_combinations;
		vector<vector<vector<vector<int> > > > der_multiple_not_combinations;
		vector<vector<pair<int, int> > > der_constant_distincts;
		vector<vector<pair<int, int> > > der_variable_distincts;
		Propositions current_stratum_props;		
		for (int j = 0; j < dpg_.stra_deriv_[i].size(); ++j) {
			Derivation d = dpg_.derivations_[dpg_.stra_deriv_[i][j]];
			vector<int> lower_stratum_subgoals;
			vector<int> current_stratum_subgoals;
			vector<int> not_subgoals;
			vector<int> distinct_subgoals;
			vector<vector<vector<int> > > multiple_combinations;
			vector<vector<vector<int> > > multiple_not_combinations;
			vector<pair<int, int> > constant_distincts;
			vector<pair<int, int> > variable_distincts;
			/*if (non_der_var_values_.size() > 0 && non_der_var_values_[dpg_.stra_deriv_[i][j]].size() > 0) {
				multiple_combinations.push_back(non_der_var_values_[dpg_.stra_deriv_[i][j]]);				
			} else {
				multiple_combinations.push_back(vector<vector<int> >());
			}*/
			bool impossible = false;
			for (int k = 0; k < d.subgoals_.size(); ++k) {								
				Proposition &subgoal = d.subgoals_[k];
				if (subgoal.head_ == r_not) {
					//not_subgoals.push_back(k);										
					if (head_propositions.find(subgoal.items_[0].head_) != head_propositions.end()) {
						vector<vector<int> > not_combinations;
						vector<int> &ps = head_propositions[subgoal.items_[0].head_];
						for (int ii = 0; ii < ps.size(); ++ii) {
							vector<int> variables;
							vector<int> values;
							if (true_props[ps[ii]].matches(subgoal.items_[0], variables, values)) {								
								vector<int> c = getCombination(d.variables_, variables, values);								
								not_combinations.push_back(c);
							}
						}
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
					lower_stratum_subgoals.push_back(k);
					vector<vector<int> > combinations;										
					vector<int> &true_rs = head_propositions[subgoal.head_];
					for (int ii = 0; ii < true_rs.size(); ++ii) { // scan all true props to generate var-value maps
						Proposition &p = true_props[true_rs[ii]];
						vector<int> variables;
						vector<int> values;
						if (subgoal.matches(p, variables, values)) {
							vector<int> c = getCombination(d.variables_, variables, values);
							combinations.push_back(c);
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
			if (impossible) {
				continue;
			}
			for (int k = 0; k < multiple_combinations.size(); ++k) {
				uniqCombinations(multiple_combinations[k]);
			}
			for (int k = 0; k < multiple_not_combinations.size(); ++k) {
				uniqCombinations(multiple_not_combinations[k]);
			}
			if (current_stratum_subgoals.size() == 0) {
				vector<vector<int>> new_combinations = mergeMultipleCombinations(multiple_combinations, 
					multiple_not_combinations, variable_distincts, constant_distincts);
				for (int k = 0; k < new_combinations.size(); ++k) {
					Proposition p = d.target_;
					p.replaceVariables(d.variables_, new_combinations[k]);
					if (true_props_set.find(p) == true_props_set.end()) {											
						if (head_propositions.find(p.head_) == head_propositions.end()) {							
							head_propositions[p.head_] = vector<int>();
						}
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
				der_constant_distincts.push_back(constant_distincts);
				der_variable_distincts.push_back(variable_distincts);
			}	
		}
		for (int j = 0; j < current_stratum_props.size(); ++j) { // size of current_stratum_props would be changed in the loop
			Proposition &p = current_stratum_props[j];
			for (int k = 0; k < derivations.size(); ++k) {
				Derivation &d = derivations[k];				
				for (int ii = 0; ii < d.subgoals_.size(); ++ii) {
					Proposition &subgoal = d.subgoals_[ii];
					vector<int> variables;
					vector<int> values;
					if (subgoal.matches(p, variables, values)) {						
						vector<int> c = getCombination(d.variables_, variables, values);
						vector<vector<vector<int> > > new_multiple_combinations = der_multiple_combinations[k];
						new_multiple_combinations.push_back(vector<vector<int> >(1, c));
						if (d.subgoals_.size() == 1) {
							vector<vector<int>> new_combinations = mergeMultipleCombinations(new_multiple_combinations, 
								der_multiple_not_combinations[k], der_variable_distincts[k], der_constant_distincts[k]);
							for (int ii = 0; ii < new_combinations.size(); ++ii) {
								Proposition p = d.target_;
								p.replaceVariables(d.variables_, new_combinations[ii]);
								if (true_props_set.find(p) == true_props_set.end()) {											
									if (head_propositions.find(p.head_) == head_propositions.end()) {							
										head_propositions[p.head_] = vector<int>();
									}
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
	vector<string> output;
	for (int i = 0; i < true_props.size(); ++i) {
		output.push_back(true_props[i].toRelation().toString());
	}
	return true_props;
}
