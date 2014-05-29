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
	init();
}

void Prover::getSubgoalSequence(vector<vector<vector<pair<int, int> > > > & var_candidates)
{ 
	if(var_candidates.size() <= 1) return;
	vector<vector<int> > subgoals;
	for(int i = 0 ; i < var_candidates.size(); ++i){
		vector<int> tt;
		subgoals.push_back(tt);
	}
	for(int i = 0 ; i < var_candidates.size(); ++i){
		for(int j = 0 ; j < var_candidates[i].size(); ++j){
			for(int k = 0 ; k  <var_candidates[i][j].size(); ++k){
				if(find(subgoals[i].begin(), subgoals[i].end(), var_candidates[i][j][k].first) == subgoals[i].end()){
					subgoals[i].push_back(var_candidates[i][j][k].first);
				}
			}
		}
	}
	vector<int> rtn;
	int maxVar = -1, tempindex = 0;
	for(int i = 0 ; i  < subgoals.size(); ++i){
		if(subgoals[i].size() > maxVar){
			maxVar = subgoals[i].size(); 
			tempindex = i;
		}
	}
	rtn.push_back(tempindex);  // get the first and largest subgoal
	set<int> varibles;
	vector<int> degrees;
	for(int j = 0 ; j < subgoals.size(); ++j){
		degrees.push_back(0);
	}
	for(int i = 0 ; i < subgoals[tempindex].size(); ++i){
		varibles.insert(subgoals[tempindex][i]);
		for(int j = 0 ; j < subgoals.size(); ++j){
			if(j == tempindex) continue;
			if(find(subgoals[j].begin(), subgoals[j].end(), subgoals[tempindex][i]) != subgoals[j].end()){
				degrees[j] ++;
			}
		}
	}
	subgoals[tempindex].clear();
	while(rtn.size() < subgoals.size()){
		int maxdegree = -11;
		int index = 0;
		for(int i = 0 ; i < subgoals.size(); ++i){
			//if(find)
			if(find(rtn.begin(), rtn.end(), i) == rtn.end() && degrees[i] > maxdegree){
				maxdegree = degrees[i];
				index = i;
			}
		}
		rtn.push_back(index);
		for(int i = 0 ; i < subgoals[index].size(); ++i){
			if(varibles.find(subgoals[index][i]) == varibles.end()){
				varibles.insert(subgoals[index][i]);
				for(int j = 0; j <  subgoals.size(); ++j){
					if(find(subgoals[j].begin(), subgoals[j].end(), subgoals[index][i]) != subgoals[j].end()){
						degrees[j]++;
					}
				}
			}
		}
		subgoals[index].clear();
	}
	int index = 0;
	vector<vector<vector<pair<int, int> > > > tempV = var_candidates;
	for(int i = 0 ; i < rtn.size(); ++i){
		var_candidates[i] = tempV[rtn[i]];
	}
}

void Prover::init() {	
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
int start = clock();
	map<int, vector<int> > head_relations;
	set<Proposition> true_props_set;
int time1start = clock();
	for (int i = 0; i < true_props.size(); ++i) {
		if (head_relations.find(true_props[i].head_) == head_relations.end()) {
			vector<int> rs;
			head_relations[true_props[i].head_] = rs;
		}
		head_relations[true_props[i].head_].push_back(i);
		true_props_set.insert(true_props[i]);
	}
time1 += clock() - time1start;
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
			int time2s = clock();
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
						int time17s = clock();
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
						time17 += clock() - time17s;
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
			time2 += clock() - time2s;
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
			int time3s = clock();
			
			int y = 1;
			for (int x = 0; x < var_candidates.size(); ++x) {
				y *= var_candidates[x].size();
			}
			time18 += y;
			time19++;

			// right?
			//getSubgoalSequence(var_candidates);
			int k = 0;
			vector<vector<int> > combined_candidates;
			set<vector<int> > combined_candidates_set;
			int variable_size = d.variables_.size();
			while (true) {
				int time5s = clock();
				int time7s = clock();
				while (k >= 0 && idx[k] == var_candidates[k].size()) {
					idx[k] = 0;
					--k;
					if (k >= 0) {
						++idx[k];
					}
				}
				time7 += clock() - time7s;
				if (k < 0) {
					time5 += clock() - time5s;
					break;
				}
				bool combined = true;
				int time9s = clock();
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
				time9 += clock() - time9s;
				time5 += clock() - time5s;
				int time6s = clock();
				if (combined) {		
					time20++;									
					if (k == var_candidates.size() - 1) {
						int time8s = clock();						
						vector<int> m(variable_size, -1);
						for (int ii = 0; ii <= k; ++ii) {
							vector<int> &c = var_candidates[ii][idx[ii]];					
							for (int jj = 0; jj < variable_size; ++jj) {
								m[jj] = c[jj] == -1 ? m[jj] : c[jj];
							}										
						}
						time8 += clock() - time8s;	
						int time11s = clock();
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
						int time12s = clock();
						for (int ii = 0; ii < not_subgoals.size() && check_not_and_distinct; ++ii) {
							int time14s = clock();
							Proposition not_relation = d.subgoals_[not_subgoals[ii]].items_[0];
							time15 += clock() - time14s;
							not_relation.replaceVariables(d.variables_, m);
							if (true_props_set.find(not_relation) != true_props_set.end() || statics_set_.find(not_relation) != statics_set_.end()) {
								check_not_and_distinct = false;
								break;
							}
							time14 += clock() - time14s;
						}
						time12 += clock() - time12s;
						time11 += clock() - time11s;
						int time10s = clock();
						if (check_not_and_distinct) {
							if (combined_candidates_set.find(m) == combined_candidates_set.end()) {
								combined_candidates.push_back(m);
								combined_candidates_set.insert(m);
							}
						}
						time10 += clock() - time10s;
						++idx[k];
					} else {						
						++k;
					}
				} else {
					++idx[k];						
				}
				time6 += clock() - time6s;
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
			d2.prepareVariables();
			derivations.push_back(d2);
			der_var_candidates.push_back(combined_candidates);			
			time3 += clock() - time3s;
		}
		int time4s = clock();
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
						d2.prepareVariables();
						derivations.push_back(d2);
						der_var_candidates.push_back(combined_candidates);			
					}
				}
			}
		}		
		time4 += clock() - time4s;
	}
generate_time += clock() - start;
	vector<string> output;
	for (int i = 0; i < true_props.size(); ++i) {
		output.push_back(true_props[i].toRelation().toString());
	}
	cout << "prover" << endl;
	return true_props;
}