#define NEW
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
	true_rs = generateTrueProps(true_rs, 0, dpg_.stra_deriv_.size() - 1);
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
		} else if(find(static_relation_.begin(), static_relation_.end(), true_rs[i].content_) != static_relation_.end()){
			statics_.push_back(true_rs[i]);			
			statics_set_.insert(true_rs[i]);
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
#ifdef NEW
	for (int i = 0; i < nonstatic_derivations_.size(); ++i) {
		vector<vector<pair<int, int> > > tmp;
		non_der_var_values_.push_back(tmp);
		Relation &d = nonstatic_derivations_[i];		
		vector<vector<vector<pair<int, int> > > > var_candidates;
		vector<int> idx;
		bool impossible = false;		
		for (int j = 1; j < d.items_.size(); ++j) {
			if (find(static_relation_.begin(), static_relation_.end(), d.items_[j].content_) == static_relation_.end()) {
				continue;
			}
			vector<vector<pair<int, int> > > candidates;			
			for (int ii = 0; ii < statics_.size(); ++ii) {				
				vector<pair<int, int> > var_value;
				if (d.items_[j].matches(statics_[ii], var_value)) {
					bool duplicated = false;
					for (int jj = 0; jj < candidates.size(); ++jj) {
						bool equal = true;
						int size = var_value.size();
						vector<pair<int, int> > &c = candidates[jj];
						for (int kk = 0; kk < size && equal; ++kk) {
							if (c[kk].second != var_value[kk].second) {
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
			if (candidates.size() == 0) {
				impossible = true;
				break;
			}
			idx.push_back(0);
			var_candidates.push_back(candidates);			
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
			vector<pair<int, int> > &c1 = var_candidates[k][idx[k]];
			for (int ii = 0; ii < c1.size() && combined; ++ii) {
				for (int jj = 0; jj < k && combined; ++jj) {
					vector<pair<int, int> > &c2 = var_candidates[jj][idx[jj]];
					int size = c2.size();
					for (int kk = 0; kk < size && combined; ++kk) {
						if (c2[kk].first == c1[ii].first
							&& c2[kk].second != c1[ii].second) {
								combined = false;
						}
					}
				}
			}
			if (combined) {											
				vector<pair<int, int> > m;
				for (int ii = 0; ii <= k; ++ii) {
					vector<pair<int, int> > &c = var_candidates[ii][idx[ii]];
					int size = c.size();
					for (int jj = 0; jj < size; ++jj) {
						bool find = false;
						for (int kk = 0; kk < m.size(); ++kk) {
							if (c[jj] == m[kk]) {
								find = true;
								break;
							}							
						}	
						if (!find) {
							m.push_back(c[jj]);
						}
					}
				}
				if (k == var_candidates.size() - 1) {
					non_der_var_values_[i].push_back(m);
					++idx[k];
				} else {						
					++k;
				}
			} else {
				++idx[k];						
			}			
		}						
		for (vector<Relation>::iterator j = d.items_.begin() + 1; j != d.items_.end(); ) {
			if (find(static_relation_.begin(), static_relation_.end(), j->content_) != static_relation_.end()) {
				j = d.items_.erase(j);
			} else {
				++j;
			}
		}
	}

	for (int i = 0; i < nonstatic_derivations_.size(); ++i) {
		cout << nonstatic_derivations_[i].toString() << endl;
		cout << non_der_var_values_[i].size() << endl;
	}
#endif

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
			static_relation_.push_back(dpg_.init_nodes_[i]);
		}
	}
	for(int i = 0 ; i < derivations_.size(); ++i){
		if(find(static_relation_.begin(), static_relation_.end(), derivations_[i].items_[0].content_) == static_relation_.end()){
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

int Prover::askRole(Relation &role){
	return role_num_[role];
}



Relations Prover::generateTrueProps(Relations true_props, int start_stra, int end_stra) {	
int start = clock();
	map<int, vector<int> > content_relations;
	set<string> true_props_string;
int time1start = clock();
	for (int i = 0; i < true_props.size(); ++i) {
		if (content_relations.find(true_props[i].content_) == content_relations.end()) {
			vector<int> rs;
			content_relations[true_props[i].content_] = rs;
		}
		content_relations[true_props[i].content_].push_back(i);
		true_props_string.insert(true_props[i].toString());
	}
time1 += clock() - time1start;
	for (int i = start_stra; i <= end_stra; ++i) {
		Relations derivations;
		for (int j = 0; j < dpg_.stra_deriv_[i].size(); ++j) {
			Relation d = dpg_.derivations_[dpg_.stra_deriv_[i][j]];
			vector<int> lower_stratum_subgoals;
			vector<int> current_stratum_subgoals;
			vector<int> not_subgoals;
			vector<int> distinct_subgoals;
			vector<vector<vector<pair<int, int> > > > var_candidates;
			vector<int> idx;
			if (non_der_var_values_.size() > 0 && non_der_var_values_[dpg_.stra_deriv_[i][j]].size() > 0) {
				var_candidates.push_back(non_der_var_values_[dpg_.stra_deriv_[i][j]]);
				idx.push_back(0);
			}
			bool impossible = false;
			int time2s = clock();
			for (int k = 1; k < d.items_.size(); ++k) {								
				if (d.items_[k].type_ == r_not) {
					not_subgoals.push_back(k);
				} else if (d.items_[k].type_ == r_distinct) {
					distinct_subgoals.push_back(k);
				} else if (dpg_.node_stra_[dpg_.node_num_[d.items_[k].content_]] < i) { // lower stratum subgoals					
					//lower_stratum_subgoals.push_back(k);
					vector<vector<pair<int, int> > > candidates;
					vector<int> &true_rs = content_relations[d.items_[k].content_];
					for (int ii = 0; ii < true_rs.size(); ++ii) { // scan all true props to generate var-value maps
						int time17s = clock();
						vector<pair<int, int> > var_value;
						if (d.items_[k].matches(true_props[true_rs[ii]], var_value)) {
							bool duplicated = false;
							for (int jj = 0; jj < candidates.size(); ++jj) {
								bool equal = true;
								int size = var_value.size();
								vector<pair<int, int> > &c = candidates[jj];
								for (int kk = 0; kk < size && equal; ++kk) {
									if (c[kk].second != var_value[kk].second) {
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
						time17 += clock() - time17s;
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
			time2 += clock() - time2s;
			if (impossible) {
				continue;
			}
			int k = 0;
			
			if (var_candidates.size() == 0) {
				bool satisfied = true;
				for (int ii = 0; ii < not_subgoals.size() && satisfied; ++ii) {
					Relation &not_relation = d.items_[not_subgoals[ii]].items_[0];
					if (statics_set_.find(not_relation) != statics_set_.end()) {
						satisfied = false;
						break;
					}
					if (content_relations.find(not_relation.content_) != content_relations.end()) {
						for (int jj = 0; jj < content_relations[not_relation.content_].size(); ++jj) {
							Relation true_prop = true_props[content_relations[not_relation.content_][jj]];
							vector<pair<int, int> > m;
							if (true_prop.matches(not_relation, m)) {
								satisfied = false;
								break;
							}
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
			int time3s = clock();
			
			int y = 1;
			for (int x = 0; x < var_candidates.size(); ++x) {
				y *= var_candidates[x].size();
			}
			time18 += y;
			time19++;
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
				vector<pair<int, int> > &c1 = var_candidates[k][idx[k]];				
				for (int ii = 0; ii < c1.size() && combined; ++ii) {
					for (int jj = 0; jj < k && combined; ++jj) {
						vector<pair<int, int> > &c2 = var_candidates[jj][idx[jj]];
						int size = c2.size();
						for (int kk = 0; kk < size && combined; ++kk) {
							time21++;
							if (c2[kk].first == c1[ii].first
								&& c2[kk].second != c1[ii].second) {
									combined = false;
							}
						}
					}
				}
				time9 += clock() - time9s;
				time5 += clock() - time5s;
				int time6s = clock();
				if (combined) {		
					time20++;
					int time8s = clock();
					vector<pair<int, int> > m;
					for (int ii = 0; ii <= k; ++ii) {
						vector<pair<int, int> > &c = var_candidates[ii][idx[ii]];
						int size = c.size();
						for (int jj = 0; jj < size; ++jj) {
							m.push_back(c[jj]);
						}
					}
					time8 += clock() - time8s;					
					if (k == var_candidates.size() - 1) {
						int time11s = clock();
						bool check_not_and_distinct = true;
						vector<int> undetermined_distincts;						
						for (int ii = 0; ii < distinct_subgoals.size() && check_not_and_distinct; ++ii) {							
							Relation distinct = d.items_[distinct_subgoals[ii]];														
							distinct.replaceVariables(m);													
							if (distinct.items_[0].type_ == r_variable || distinct.items_[1].type_ == r_variable) {
								undetermined_distincts.push_back(ii);
							} else if (distinct.items_[0].content_ == distinct.items_[1].content_) {
								check_not_and_distinct = false;
							}							
						}						
						int time12s = clock();
						for (int ii = 0; ii < not_subgoals.size() && check_not_and_distinct; ++ii) {
							int time14s = clock();
							Relation not_relation = d.items_[not_subgoals[ii]].items_[0]; // 改成引用 对比tmp和m
							time15 += clock() - time14s;
							not_relation.replaceVariables(m);
							if (statics_set_.find(not_relation) != statics_set_.end()) {
								check_not_and_distinct = false;
								break;
							}
							time14 += clock() - time14s;
							int time13s = clock();
							vector<int> &not_rs = content_relations[not_relation.content_];
							int size = not_rs.size();
							for (int jj = 0; jj < size; ++jj) {																								
								vector<pair<int, int> > tmp;
								if (true_props[not_rs[jj]].matches(not_relation, tmp)) {									
									check_not_and_distinct = false;									
									break;
								}																
							}
							time13 += clock() - time13s;
						}
						time12 += clock() - time12s;
						time11 += clock() - time11s;
						int time10s = clock();
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
			time3 += clock() - time3s;
		}
		int time4s = clock();
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
		time4 += clock() - time4s;
	}
generate_time += clock() - start;
	return true_props;
}