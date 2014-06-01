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
	for (int i = 0; i < nonstatic_derivations_.size(); ++i) {
		vector<vector<int> > tmp;
		non_der_var_values_.push_back(tmp);
		Derivation &d = nonstatic_derivations_[i];		
		vector<vector<vector<int> > > var_candidates;
		vector<int> idx;
		bool impossible = false;		
		for (int j = 0; j < d.subgoals_.size(); ++j) {
			if (find(static_heads_.begin(), static_heads_.end(), d.subgoals_[j].head_) == static_heads_.end()) {
				continue;
			}
			vector<vector<int> > candidates;
			Proposition &subgoal = d.subgoals_[j];			
			for (int ii = 0; ii < statics_.size(); ++ii) {				
				vector<int> variables;
				vector<int> values;				
				if (subgoal.matches(statics_[ii], variables, values)) {					
					bool duplicated = false;
					for (int jj = 0; jj < candidates.size(); ++jj) {
						bool equal = true;
						vector<int> &c = candidates[jj];						
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
						candidates.push_back(values);
					}
				}
			}
			if (candidates.size() == 0) {
				impossible = true;
				break;
			}
			idx.push_back(0);
			vector<vector<int> > candidates2;
			vector<int> variables;
			subgoal.getVariables(variables);
			for (int ii = 0; ii < candidates.size(); ++ii) {
				vector<int> &c = candidates[ii];
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
				candidates2.push_back(c2);
			}
			var_candidates.push_back(candidates2);			
		}
		if (idx.size() == 0) {
			continue;
		}
		int k = 0;
		while (true) {
			while (k >= 0 && idx[k] == var_candidates[k].size()) {
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
			vector<int> &c1 = var_candidates[k][idx[k]];			
			for (int ii = 0; ii < k && combined; ++ii) {
				vector<int> &c2 = var_candidates[ii][idx[ii]];
				int size = d.variables_.size();
				for (int jj = 0; jj < size && combined; ++jj) {
					if (c1[jj] != -1 && c2[jj] != -1 && c1[jj] != c2[jj]) {							
						combined = false;
					}
				}
			}
			
			if (combined) {															
				if (k == var_candidates.size() - 1) {
					int size = d.variables_.size();
					vector<int> m(size, -1);
					for (int ii = 0; ii <= k; ++ii) {
						vector<int> &c = var_candidates[ii][idx[ii]];					
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
	}	

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

vector<vector<int> > Prover::mergeTwoCombinations(
	vector<vector<int> > &a, 
	vector<vector<int> > &b, 
	vector<int> &idx_a, 
	vector<int> &idx_b, 
	vector<int> &keys) {

	vector<vector<int> > ret;
	int a_begin = 0;
	int b_begin = 0;
	while (a_begin < idx_a.size() && b_begin < idx_b.size()) {
		int compare = compareCombination(a[idx_a[a_begin]], b[idx_b[b_begin]], keys);
		if (compare == -1) {
			++a_begin;
		} if (compare == 1) {
			++b_begin;
		} else {
			int a_end = a_begin;
			int b_end = b_begin;
			while (compareCombination(a[idx_a[a_begin]], a[idx_a[a_end]], keys) == 0) {
				++a_end;
			}
			while (compareCombination(b[idx_b[b_begin]], b[idx_b[b_end]], keys) == 0) {
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
		} if (compare == 1) {
			++b_begin;
		} else {
			int a_end = a_begin;
			int b_end = b_begin;
			while (compareCombination(a[idx_a[a_begin]], a[idx_a[a_end]], keys) == 0) {
				++a_end;
			}
			while (compareCombination(b[idx_b[b_begin]], b[idx_b[b_end]], keys) == 0) {
				++b_end;
			}
			ret += (a_end - a_begin) * (b_end - b_begin);			
			a_begin = a_end;
			b_begin = b_end;
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

vector<vector<int> > Prover::mergeMultipleCombinations(
	vector<vector<vector<int> > > &multiple_combinations, 
	vector<vector<vector<int> > > &multiple_not_combinations,
	vector<vector<vector<int> > > &distincts) {

	int size = multiple_combinations.size();
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
		multiple_combinations[c1] = mergeTwoCombinations(multiple_combinations[c1], multiple_combinations[c2], idxes[c1][c2].first, idxes[c1][c2].second, keys[c1][c2]);
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
	vector<string> input;
	for (int i = 0; i < true_props.size(); ++i) {
		input.push_back(true_props[i].toRelation().toString());
	}
	map<int, vector<int> > head_relations;
	set<Proposition> true_props_set;
	for (int i = 0; i < true_props.size(); ++i) {
		if (head_relations.find(true_props[i].head_) == head_relations.end()) {
			vector<int> rs;
			head_relations[true_props[i].head_] = rs;
		}
		head_relations[true_props[i].head_].push_back(i);
		true_props_set.insert(true_props[i]);
	}
	for (int i = start_stra; i <= end_stra; ++i) {
		vector<Derivation> derivations;
		vector<vector<vector<int> > > der_var_candidates;
		Propositions current_stratum_props;		
		for (int j = 0; j < dpg_.stra_deriv_[i].size(); ++j) {
			Derivation d = dpg_.derivations_[dpg_.stra_deriv_[i][j]];
			vector<int> lower_stratum_subgoals;
			vector<int> current_stratum_subgoals;
			vector<int> not_subgoals;
			vector<int> distinct_subgoals;
			vector<vector<vector<int> > > var_candidates;			
			if (non_der_var_values_.size() > 0 && non_der_var_values_[dpg_.stra_deriv_[i][j]].size() > 0) {
				var_candidates.push_back(non_der_var_values_[dpg_.stra_deriv_[i][j]]);				
			}
			bool impossible = false;
			for (int k = 0; k < d.subgoals_.size(); ++k) {								
				Proposition &subgoal = d.subgoals_[k];
				if (subgoal.head_ == r_not) {
					not_subgoals.push_back(k);
				} else if (subgoal.head_ == r_distinct) {
					distinct_subgoals.push_back(k);
				} else if (dpg_.node_stra_[dpg_.node_num_[subgoal.head_]] < i) { // lower stratum subgoals					
					lower_stratum_subgoals.push_back(k);
					vector<vector<int> > candidates;										
					vector<int> &true_rs = head_relations[subgoal.head_];
					for (int ii = 0; ii < true_rs.size(); ++ii) { // scan all true props to generate var-value maps
						Proposition &p = true_props[true_rs[ii]];
						vector<int> variables;
						vector<int> values;
						if (subgoal.matches(p, variables, values)) {							
							bool duplicated = false;
							for (int jj = 0; jj < candidates.size(); ++jj) {
								bool equal = true;
								vector<int> &c = candidates[jj];
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
								candidates.push_back(values);
							}
						}
					}
					if (candidates.size() == 0) { // size of candidates should be greater than 0
						impossible = true;
						break;
					}					
					vector<vector<int> > candidates2;
					vector<int> variables;
					subgoal.getVariables(variables);
					for (int ii = 0; ii < candidates.size(); ++ii) {
						vector<int> &c = candidates[ii];
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
						candidates2.push_back(c2);
					}
					var_candidates.push_back(candidates2);						
				} else {
					current_stratum_subgoals.push_back(k);
				}
			}
			if (impossible) {
				continue;
			}						
			if (var_candidates.size() == 0) {
				vector<int> candidate;
				vector<vector<int> > candidates;
				candidates.push_back(candidate);
				var_candidates.push_back(candidates);				
			}
			vector<int> idx;
			for (int k = 0; k < var_candidates.size(); ++k) {
				idx.push_back(0);
			}
			
			int y = 1;
			for (int x = 0; x < var_candidates.size(); ++x) {
				y *= var_candidates[x].size();
			}

			// right?
			//getSubgoalSequence(var_candidates);
			int k = 0;
			vector<vector<int> > combined_candidates;
			set<vector<int> > combined_candidates_set;
			int variable_size = d.variables_.size();
			while (true) {
				while (k >= 0 && idx[k] == var_candidates[k].size()) {
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
				vector<int> &c1 = var_candidates[k][idx[k]];				
				for (int ii = 0; ii < k && combined; ++ii) {
					vector<int> &c2 = var_candidates[ii][idx[ii]];
					int size = d.variables_.size();
					for (int jj = 0; jj < size && combined; ++jj) {
						if (c1[jj] != -1 && c2[jj] != -1 && c1[jj] != c2[jj]) {							
							combined = false;
						}
					}
				}
				if (combined) {		
					if (k == var_candidates.size() - 1) {
						vector<int> m(variable_size, -1);
						for (int ii = 0; ii <= k; ++ii) {
							vector<int> &c = var_candidates[ii][idx[ii]];					
							for (int jj = 0; jj < variable_size; ++jj) {
								m[jj] = c[jj] == -1 ? m[jj] : c[jj];
							}										
						}
						bool check_not_and_distinct = true;						
						for (int ii = 0; ii < distinct_subgoals.size() && check_not_and_distinct; ++ii) {							
							Proposition &distinct = d.subgoals_[distinct_subgoals[ii]];														
							int first = distinct.items_[0].head_;
							int second = distinct.items_[1].head_;
							for (int jj = 0; jj < variable_size; ++jj) {
								if (d.variables_[jj] == distinct.items_[0].head_) {
									first = m[jj];
								}
								if (d.variables_[jj] == distinct.items_[1].head_) {
									second = m[jj];
								}
							}
							if (first != -1 && second != -1 && first == second) {
								check_not_and_distinct = false;
							}							
						}
						for (int ii = 0; ii < not_subgoals.size() && check_not_and_distinct; ++ii) {
							Proposition not_relation = d.subgoals_[not_subgoals[ii]].items_[0];
							not_relation.replaceVariables(d.variables_, m);
							if (true_props_set.find(not_relation) != true_props_set.end() || statics_set_.find(not_relation) != statics_set_.end()) {
								check_not_and_distinct = false;
								break;
							}
						}
						if (check_not_and_distinct) {
							if (combined_candidates_set.find(m) == combined_candidates_set.end()) {
								combined_candidates.push_back(m);
								combined_candidates_set.insert(m);
							}
						}
						++idx[k];
					} else {						
						++k;
					}
				} else {
					++idx[k];						
				}
			}
			if (combined_candidates.size() == 0) {
				continue;
			}
			if (current_stratum_subgoals.size() == 0) {
				for (int ii = 0; ii < combined_candidates.size(); ++ii) {
					Proposition p = d.target_;
					p.replaceVariables(d.variables_, combined_candidates[ii]);
					if (true_props_set.find(p) == true_props_set.end()) {											
						if (head_relations.find(p.head_) == head_relations.end()) {
							vector<int> tmp;
							head_relations[p.head_] = tmp;
						}
						head_relations[p.head_].push_back(true_props.size());
						true_props.push_back(p);
						true_props_set.insert(p);
						current_stratum_props.push_back(p);										
					}
				}
				continue;
			}
			Derivation d2;
			d2.target_ = d.target_;
			d2.variables_ = d.variables_;
			for (int ii = 0; ii < current_stratum_subgoals.size(); ++ii) {
				d2.subgoals_.push_back(d.subgoals_[current_stratum_subgoals[ii]]);
			}			
			// undetermined distinct subgoals
			for (int ii = 0; ii < distinct_subgoals.size(); ++ii) {							
				Proposition &distinct = d.subgoals_[distinct_subgoals[ii]];																		
				for (int jj = 0; jj < variable_size; ++jj) {
					if ((d.variables_[jj] == distinct.items_[0].head_ || d.variables_[jj] == distinct.items_[1].head_) 
						&& combined_candidates[0][jj] == -1) {	// combined_candidates has 1 element at least
						d2.subgoals_.push_back(d.subgoals_[distinct_subgoals[ii]]);
						break;
					}
				}						
			}
			derivations.push_back(d2);
			der_var_candidates.push_back(combined_candidates);			
		}
		for (int j = 0; j < current_stratum_props.size(); ++j) { // size of current_stratum_props would be changed in the loop
			Proposition &p = current_stratum_props[j];
			for (int k = 0; k < derivations.size(); ++k) {
				Derivation &d = derivations[k];
				vector<int> distincts;
				vector<int> non_distincts;
				for (int ii = 0; ii < d.subgoals_.size(); ++ii) {
					Proposition &subgoal = d.subgoals_[ii];
					if (subgoal.head_ == r_distinct) {
						distincts.push_back(ii);
					} else {
						non_distincts.push_back(ii);
					}
				}
				for (int ii = 0; ii < non_distincts.size(); ++ii) {
					Proposition &subgoal = d.subgoals_[non_distincts[ii]];
					vector<int> variables;
					vector<int> values;
					if (subgoal.matches(p, variables, values)) {
						vector<vector<int> > combined_candidates;
						set<vector<int> > combined_candidates_set;
						for (int jj = 0; jj < der_var_candidates[k].size(); ++jj) {
							vector<int> m = der_var_candidates[k][jj];
							bool combined = true;
							for (int kk = 0; kk < d.variables_.size() && combined; ++kk) {
								for (int iii = 0; iii < variables.size(); ++iii) {
									if (d.variables_[kk] == variables[iii]) {
										if (m[kk] == -1) {
											m[kk] = values[iii];
										} else if (m[kk] != values[iii]) {
											combined = false;
										}
										break;
									}
								}
							}
							if (combined) {
								bool check_distinct = true;								
								for (int kk = 0; kk < distincts.size() && check_distinct; ++kk) {									
									Proposition &distinct = d.subgoals_[distincts[kk]];														
									int first = distinct.items_[0].head_;
									int second = distinct.items_[1].head_;
									for (int iii = 0; iii < d.variables_.size(); ++iii) {
										if (d.variables_[iii] == distinct.items_[0].head_) {
											first = m[iii];
										}
										if (d.variables_[iii] == distinct.items_[1].head_) {
											second = m[iii];
										}
									}
									if (first != -1 && second != -1 && first == second) {
										check_distinct = false;
									}									
								}
								if (check_distinct) {									
									if (non_distincts.size() == 1) {
										Proposition p = d.target_;
										p.replaceVariables(d.variables_, m);
										if (true_props_set.find(p) == true_props_set.end()) {											
											if (head_relations.find(p.head_) == head_relations.end()) {
												vector<int> tmp;
												head_relations[p.head_] = tmp;
											}
											head_relations[p.head_].push_back(true_props.size());
											true_props.push_back(p);
											true_props_set.insert(p);
											current_stratum_props.push_back(p);										
										}
									} else {
										if (combined_candidates_set.find(m) == combined_candidates_set.end()) {
											combined_candidates.push_back(m);
											combined_candidates_set.insert(m);
										}
									}
								}
							}
						}
						if (combined_candidates.size() == 0) {
							continue;
						}
						Derivation d2;
						d2.target_ = d.target_;
						d2.variables_ = d.variables_;
						for (int jj = 0; jj < non_distincts.size(); ++jj) {
							if (jj != ii) {
								d2.subgoals_.push_back(d.subgoals_[non_distincts[jj]]);
							}											
						}			
						// undetermined distinct subgoals
						for (int jj = 0; jj < distincts.size(); ++jj) {
							Proposition &distinct = d.subgoals_[distincts[jj]];
							for (int kk = 0; kk < d.variables_.size(); ++kk) {
								if ((d.variables_[kk] == distinct.items_[0].head_ || d.variables_[kk] == distinct.items_[1].head_) 
									&& combined_candidates[0][kk] == -1) {	// combined_candidates has 1 element at least
									d2.subgoals_.push_back(d.subgoals_[distincts[jj]]);
									break;
								}
							}						
						}
						derivations.push_back(d2);
						der_var_candidates.push_back(combined_candidates);			
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
