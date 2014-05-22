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
using namespace std;



Prover::Prover(Relations relations) : relations_(relations) {
	init();
}

void Prover::init() {	
	for (int i = 0; i < relations_.size(); ++i) {
		if (relations_[i].type_ == r_derivation) {
			derivations_.push_back(relations_[i]);
		}
	}
	
	dpg_.buildGraph(derivations_);  // build graph for the first time
	getStaticRelation();
	Relations true_rs;
	for(int i = 0 ; i < relations_.size(); ++i){
		if(relations_[i].type_ == r_role 
			|| relations_[i].type_ == r_init
			|| relations_[i].type_ == r_function ){
				true_rs.push_back(relations_[i]);
		}
	}
	true_rs = generateTrueProps(true_rs);
	for(int i = 0; i < true_rs.size(); ++i){
		if(true_rs[i].type_ == r_init){
			inits_.push_back(true_rs[i]);
		} else if(true_rs[i].type_ == r_base){
			bases_.push_back(true_rs[i]);
			Relation r = true_rs[i];
			r.content_ = r_true;
			r.type_ = r_true;
			keyrelations_.push_back(r);
			keyrelation_num_[r] = keyrelation_num_.size();
		} else if(true_rs[i].type_ == r_input){
			inputs_.push_back(true_rs[i]);
			Relation r = true_rs[i];
			r.content_ = r_legal;
			r.type_ = r_legal;
			legalactions_.push_back(r);
			legalaction_num_[r] = legalaction_num_.size();
		} else if(find(static_relation_.begin(), static_relation_.end(), dpg_.node_num_[true_rs[i].content_]) != static_relation_.end()){
			statics_.push_back(true_rs[i]);
		}
	}
	// add key_head which isn't in base sentence
	for(int i = 0 ; i < relations_.size(); ++i){
		if(relations_[i].type_ == r_base){
			int temp = relations_[i].items_[0].content_;
			if(find(key_head_.begin(), key_head_.end(), temp) == key_head_.end()){
				key_head_.push_back(temp);
			}
		} else if(relations_[i].type_ == r_derivation && (relations_[i].items_[0].type_ == r_next)){
			int temp = relations_[i].items_[0].items_[0].content_;
			if(find(key_head_.begin(), key_head_.end(), temp) == key_head_.end()){
				key_head_.push_back(temp);
			}
		} else if(relations_[i].type_ == r_role){
			roles_.push_back(relations_[i]);
		}
	}

	DependGraph dpg2;
	dpg2.buildGraph(nonstatic_derivations_); 
	dpg_ = dpg2;
}

void Prover::getStaticRelation()
{
	vector<int> mark;
	for(int i = 0 ; i < dpg_.init_nodes_.size(); ++i){
		mark.push_back(0);
	}
	markNonStatic(dpg_.node_num_[r_does], mark);
	markNonStatic(dpg_.node_num_[r_true], mark);
	for(int i = 0 ; i < mark.size(); ++i){
		if(mark[i] == 0){
			static_relation_.push_back(i);
		}
	}
	for(int i = 0 ; i < derivations_.size(); ++i){
		if(find(static_relation_.begin(), static_relation_.end(), dpg_.node_num_[derivations_[i].items_[0].content_]) == static_relation_.end()){
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


string Prover::buildNode(string s, int i){
	char number[10];
	sprintf(number, "%d", i);
	return s + '[' + number + ']';
}










int Prover::askRole(Relation &role){
	return role_num_[role];
}








Relations Prover::generateTrueProps(Relations true_props) {	
	map<int, vector<int> > content_relations;
	set<string> true_props_string;
	for (int i = 0; i < true_props.size(); ++i) {
		if (content_relations.find(true_props[i].content_) == content_relations.end()) {
			vector<int> rs;
			content_relations[true_props[i].content_] = rs;
		}
		content_relations[true_props[i].content_].push_back(i);
		true_props_string.insert(true_props[i].toString());
	}
	for (int i = 0; i < dpg_.stra_deriv_.size(); ++i) {
		Relations derivations;
		for (int j = 0; j < dpg_.stra_deriv_[i].size(); ++j) {
			Relation d = dpg_.derivations_[dpg_.stra_deriv_[i][j]];
			vector<int> lower_stratum_subgoals;
			vector<int> current_stratum_subgoals;
			vector<int> not_subgoals;
			vector<int> distinct_subgoals;
			vector<vector<map<int, int> > > var_candidates;
			vector<int> idx;
			bool impossible = false;
			for (int k = 1; k < d.items_.size(); ++k) {								
				if (d.items_[k].type_ == r_not) {
					not_subgoals.push_back(k);
				} else if (d.items_[k].type_ == r_distinct) {
					distinct_subgoals.push_back(k);
				} else if (dpg_.node_stra_[dpg_.node_num_[d.items_[k].content_]] < i) { // lower stratum subgoals					
					//lower_stratum_subgoals.push_back(k);
					vector<map<int, int> > candidates;
					for (int ii = 0; ii < content_relations[d.items_[k].content_].size(); ++ii) { // scan all true props to generate var-value maps
						map<int, int> var_value;
						if (d.items_[k].matches(true_props[content_relations[d.items_[k].content_][ii]], var_value)) {
							bool duplicated = false;
							for (int jj = 0; jj < candidates.size(); ++jj) {
								bool equal = true;
								for (map<int, int>::iterator kk = var_value.begin(); kk != var_value.end(); ++kk) {
									if (candidates[jj][kk->first] != kk->second) {
										equal = false;
									}
								}
								if (equal) {
									duplicated = true;
									break;
								}
							}
							if (!duplicated) {
								candidates.push_back(var_value);
							}
						}
					}
					if (candidates.size() == 0) { // size of candidates should be greater than 0
						impossible = true;
						break;
					}
					idx.push_back(0);
					var_candidates.push_back(candidates);
				} else {
					current_stratum_subgoals.push_back(k);
				}
			}
			if (impossible) {
				continue;
			}
			int k = 0;
			if (var_candidates.size() == 0) {
				bool satisfied = true;
				for (int ii = 0; ii < not_subgoals.size() && satisfied; ++ii) {
					Relation not_relation = d.items_[not_subgoals[ii]].items_[0];					
					for (int jj = 0; jj < content_relations[not_relation.content_].size(); ++jj) {
						Relation true_prop = true_props[content_relations[not_relation.content_][jj]];
						map<int, int> m;
						if (true_prop.matches(not_relation, m)) {
							satisfied = false;
							break;
						}
					}
				}
				if (satisfied) {
					Relation d2;
					d2.type_ = d.type_;
					d2.content_ = d.content_;
					d2.items_.push_back(d.items_[0]);
					derivations.push_back(d2);
				}
				k = -1; // avoid the while loop below
			}			
			vector<map<int, int> > maps;
			map<int, int> m;
			maps.push_back(m);
			while (true) {								
				while (k >= 0 && idx[k] == var_candidates[k].size()) {
					idx[k] = 0;
					maps.pop_back();
					--k;
					if (k >= 0) {
						++idx[k];
					}
				}
				if (k < 0) {
					break;
				}
				map<int, int> m = maps[maps.size() - 1];
				map<int, int> m2 = var_candidates[k][idx[k]];
				bool combined = true;
				for (map<int, int>::iterator ii = m2.begin(); ii != m2.end(); ++ii) {
					if (m.find(ii->first) == m.end()) {
						m[ii->first] = ii->second;
					} else if (m[ii->first] != ii->second) {
						combined = false;
						break;
					}
				}
				if (combined) {											
					if (k == var_candidates.size() - 1) {
						bool check_not_and_distinct = true;
						vector<int> undetermined_distincts;
						for (int ii = 0; ii < distinct_subgoals.size(); ++ii) {
							Relation distinct = d.items_[distinct_subgoals[ii]];
							distinct.replaceVariables(m);							
							if (distinct.items_[0].type_ == r_variable || distinct.items_[1].type_ == r_variable) {
								undetermined_distincts.push_back(ii);
							} else if (distinct.items_[0].content_ == distinct.items_[1].content_) {
								check_not_and_distinct = false;
							}							
						}
						for (int ii = 0; ii < not_subgoals.size(); ++ii) {
							Relation not_relation = d.items_[not_subgoals[ii]].items_[0];
							not_relation.replaceVariables(m);							
							for (int jj = 0; jj < content_relations[not_relation.content_].size(); ++jj) {								
								map<int, int> m;
								if (true_props[content_relations[not_relation.content_][jj]].matches(not_relation, m)) {
									check_not_and_distinct = false;
									break;
								}
							}
						}
						if (check_not_and_distinct) {
							Relation d2;
							d2.type_ = d.type_;
							d2.content_ = d.content_;
							d2.items_.push_back(d.items_[0]);
							for (int ii = 0; ii < current_stratum_subgoals.size(); ++ii) {
								d2.items_.push_back(d.items_[current_stratum_subgoals[ii]]);
							}
							for (int ii = 0; ii < undetermined_distincts.size(); ++ii) {
								d2.items_.push_back(d.items_[distinct_subgoals[undetermined_distincts[ii]]]);
							}
							d2.replaceVariables(m);
							derivations.push_back(d2);
						}
						++idx[k];
					} else {
						maps.push_back(m);
						++k;
					}
				} else {
					++idx[k];						
				}				
			}			
		}

		while (true) {
			int old_true_props_num = true_props.size();
			for (Relations::iterator j = derivations.begin(); j != derivations.end(); ) {
				if (j->items_.size() == 1) {					
					Relation prop = j->items_[0];
					if (true_props_string.find(prop.toString()) == true_props_string.end()) {
						true_props.push_back(prop);
						true_props_string.insert(prop.toString());
						if (content_relations.find(prop.content_) == content_relations.end()) {
							vector<int> rs;
							content_relations[prop.content_] = rs;
						}
						content_relations[prop.content_].push_back(true_props.size() - 1);
					}
					j = derivations.erase(j);
				} else {
					++j;
				}
			}
			if (old_true_props_num == true_props.size()) {
				break;
			}
			for (int j = 0; j < derivations.size(); ++j) {				
				for (Relations::iterator k = derivations[j].items_.begin(); k != derivations[j].items_.end(); ) {
					Relation p = *k;
					if (content_relations.find(p.content_) != content_relations.end()) {
						vector<int> props = content_relations[p.content_];
						bool find = false;
						for (int ii = props.size() - 1; ii >= 0; --ii) {
							if (props[ii] < old_true_props_num) {
								break;
							}
							bool equal = true;
							for (int jj = 0; jj < p.items_.size(); ++jj) {
								if (p.items_[jj].content_ != true_props[props[ii]].items_[jj].content_) {
									equal = false;
									break;
								}
							}
							if (equal) {
								find = true;
								break;
							}
						}
						if (find) {
							k = derivations[j].items_.erase(k);
							continue;
						}
					}
					++k;
				}
			}
		}		
	}
	return true_props;
}