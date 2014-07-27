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
#include "combination.h"
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
					uniqCombinations(not_combinations);
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
					uniqCombinations(combinations);
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
			clock_t s8 = clock();
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
						uniqCombinations(not_combinations);
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
					uniqCombinations(combinations);
					multiple_combinations.push_back(combinations);					
				} else {
					current_stratum_subgoals.push_back(k);
				}
			}
			//time[6] += clock() - s6;
			//clock_t s7 = clock();
			if (impossible) {
				time[8] += clock() - s8;
				continue;
			}
			if (multiple_combinations.size() == 0) {
				multiple_combinations.push_back(vector<vector<int> >(1, vector<int>())); // in case there are only 'not' subgoals
			}
			//for (int k = der_multiple_combinations_[der_id].size(); k < multiple_combinations.size(); ++k) {
			//	uniqCombinations(multiple_combinations[k]);
			//}
			//for (int k = der_multiple_not_combinations_[der_id].size(); k < multiple_not_combinations.size(); ++k) {
			//	uniqCombinations(multiple_not_combinations[k]);
			//}						
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
			time[8] += clock() - s8;
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
