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

void fun() {

}

void Prover::init() {	
	for (int i = 0; i < relations_.size(); ++i) {
		if (relations_[i].type_ == r_derivation) {
			Derivation d = relations_[i].toDerivation();
			d.prepareVariables();
			derivations_.push_back(d);			
		}
	}
	
	dpg_.buildGraph(derivations_);  // build graph for the first time
	getStaticRelation();
	vector<Proposition> true_rs;
	for(int i = 0 ; i < relations_.size(); ++i){
		if(relations_[i].type_ == r_role 
			|| relations_[i].type_ == r_init
			|| relations_[i].type_ == r_function ){
				true_rs.push_back(relations_[i].toProposition());
		}
	}
	true_rs = generateTrueProps(true_rs, 0, dpg_.stra_deriv_.size() - 1);
	for(int i = 0; i < true_rs.size(); ++i){
		if(true_rs[i].type() == r_init){
			inits_.push_back(true_rs[i]);
		} else if(true_rs[i].type() == r_base){
			bases_.push_back(true_rs[i]);			
		} else if(true_rs[i].type() == r_input){
			inputs_.push_back(true_rs[i]);			
		} else if(find(static_heads_.begin(), static_heads_.end(), true_rs[i].head()) != static_heads_.end()){
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
			roles_.push_back(relations_[i].toProposition());
		}
	}
	for (int i = 0; i < nonstatic_derivations_.size(); ++i) {
		vector<vector<int> > tmp;
		non_der_var_values_.push_back(tmp);
		Derivation &d = nonstatic_derivations_[i];		
		vector<vector<vector<int> > > var_candidates;
		vector<int> idx;
		bool impossible = false;		
		for (int j = 0; j < d.subgoals_.size(); ++j) {
			if (find(static_heads_.begin(), static_heads_.end(), d.subgoals_[j].head()) == static_heads_.end()) {
				continue;
			}
			vector<vector<int> > candidates;
			Proposition &subgoal = d.subgoals_[j];
			vector<int> var_positions = subgoal.getVarPos();			
			for (int ii = 0; ii < statics_.size(); ++ii) {				
				vector<int> values;
				if (subgoal.headMatches(statics_[ii])) {
					for (int jj = 0; jj < var_positions.size(); ++jj) {
						values.push_back(statics_[ii].items_[jj]);
					}					
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
			for (int ii = 0; ii < candidates.size(); ++ii) {
				vector<int> &c = candidates[ii];
				vector<int> c2;
				for (int jj = 0; jj < d.variables_.size(); ++jj) {
					int val = -1;
					for (int kk = 0; kk < var_positions.size(); ++kk) {
						if (subgoal.items_[var_positions[kk]] == d.variables_[jj]) {
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
					vector<int> m(size);
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
		for (vector<Proposition>::iterator j = d.subgoals_.begin(); j != d.subgoals_.end(); ) {
			if (find(static_heads_.begin(), static_heads_.end(), j->head()) != static_heads_.end()) {
				j = d.subgoals_.erase(j);
			} else {
				++j;
			}
		}
	}

	for (int i = 0; i < nonstatic_derivations_.size(); ++i) {
		cout << nonstatic_derivations_[i].toRelation().toString() << endl;
		cout << non_der_var_values_[i].size() << endl;
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
			static_heads_.push_back(dpg_.init_nodes_[i]);
		}
	}
	for(int i = 0 ; i < derivations_.size(); ++i){
		if(find(static_heads_.begin(), static_heads_.end(), derivations_[i].target_.head()) == static_heads_.end()){
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



vector<Proposition> Prover::generateTrueProps(vector<Proposition> true_props, int start_stra, int end_stra) {	
int start = clock();
	map<int, vector<int> > content_relations;
	set<string> true_props_string;
int time1start = clock();
	for (int i = 0; i < true_props.size(); ++i) {
		if (content_relations.find(true_props[i].head()) == content_relations.end()) {
			vector<int> rs;
			content_relations[true_props[i].head()] = rs;
		}
		content_relations[true_props[i].head()].push_back(i);
		true_props_string.insert(true_props[i].toRelation().toString());
	}
time1 += clock() - time1start;
	for (int i = start_stra; i <= end_stra; ++i) {
		vector<Derivation> derivations;
		vector<vector<vector<int> > > der_var_candidates;
		vector<Proposition> current_stratum_props;
		for (int j = 0; j < dpg_.stra_deriv_[i].size(); ++j) {
			Derivation d = dpg_.derivations_[dpg_.stra_deriv_[i][j]];
			vector<int> lower_stratum_subgoals;
			vector<int> current_stratum_subgoals;
			vector<int> not_subgoals;
			vector<int> distinct_subgoals;
			vector<vector<vector<int> > > var_candidates;
			vector<int> idx;
			if (non_der_var_values_.size() > 0 && non_der_var_values_[dpg_.stra_deriv_[i][j]].size() > 0) {
				var_candidates.push_back(non_der_var_values_[dpg_.stra_deriv_[i][j]]);
				idx.push_back(0);
			}
			bool impossible = false;
			int time2s = clock();
			for (int k = 0; k < d.subgoals_.size(); ++k) {								
				if (d.subgoals_[k].head() == r_not) {
					not_subgoals.push_back(k);
				} else if (d.subgoals_[k].head() == r_distinct) {
					distinct_subgoals.push_back(k);
				} else if (dpg_.node_stra_[dpg_.node_num_[d.subgoals_[k].head()]] < i) { // lower stratum subgoals					
					lower_stratum_subgoals.push_back(k);
					vector<vector<int> > candidates;
					Proposition &subgoal = d.subgoals_[k];
					vector<int> var_positions = subgoal.getVarPos();
					vector<int> &true_rs = content_relations[d.subgoals_[k].head()];
					for (int ii = 0; ii < true_rs.size(); ++ii) { // scan all true props to generate var-value maps
						int time17s = clock();						
						vector<int> values;
						if (subgoal.headMatches(true_props[true_rs[ii]])) {
							for (int jj = 0; jj < var_positions.size(); ++jj) {
								values.push_back(statics_[ii].items_[jj]);
							}
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
					idx.push_back(0);
					vector<vector<int> > candidates2;
					for (int ii = 0; ii < candidates.size(); ++ii) {
						vector<int> &c = candidates[ii];
						vector<int> c2;
						for (int jj = 0; jj < d.variables_.size(); ++jj) {
							int val = -1;
							for (int kk = 0; kk < var_positions.size(); ++kk) {
								if (subgoal.items_[var_positions[kk]] == d.variables_[jj]) {
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
			if (lower_stratum_subgoals.size() == 0) {
				bool satisfied = true;
				for (int ii = 0; ii < not_subgoals.size() && satisfied; ++ii) {
					Proposition not_relation = d.subgoals_[not_subgoals[ii]];
					not_relation.removeHead();					
					if (statics_set_.find(not_relation) != statics_set_.end()) {
						satisfied = false;
						break;
					}
					if (content_relations.find(not_relation.head()) != content_relations.end()) {
						for (int jj = 0; jj < content_relations[not_relation.head()].size(); ++jj) {
							Proposition true_prop = true_props[content_relations[not_relation.head()][jj]];
							vector<int> variables;
							vector<int> values;
							if (true_prop.matches(not_relation, variables, values)) {
								satisfied = false;
								break;
							}
						}
					}
				}
				if (satisfied) {
					current_stratum_props.push_back(d.target_);
				}
				continue; // avoid the while loop below				
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
						vector<int> m(variable_size);
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
							int first;
							int second;
							for (int jj = 0; jj < variable_size && (first == -1 || second == -1); ++jj) {
								if (d.variables_[jj] == distinct.items_[1]) {
									first = m[jj];
								}
								if (d.variables_[jj] == distinct.items_[2]) {
									second = m[jj];
								}
							}
							if (first != -1 && second != -1 && first != second) {
								check_not_and_distinct = false;
							}							
						}
						int time12s = clock();
						for (int ii = 0; ii < not_subgoals.size() && check_not_and_distinct; ++ii) {
							int time14s = clock();
							Proposition not_relation = d.subgoals_[not_subgoals[ii]]; // 改成引用 对比tmp和m
							not_relation.removeHead();
							time15 += clock() - time14s;
							vector<int> var_positions = not_relation.getVarPos();
							for (int jj = 0; jj < var_positions.size(); ++jj) {
								for (int kk = 0; kk < variable_size; ++kk) {
									if (d.variables_[kk] == not_relation.items_[var_positions[jj]]) {
										not_relation.items_[var_positions[jj]] = m[jj];
									}
								}
							}
							if (statics_set_.find(not_relation) != statics_set_.end()) {
								check_not_and_distinct = false;
								break;
							}
							time14 += clock() - time14s;
							int time13s = clock();
							vector<int> &not_rs = content_relations[not_relation.head()];
							int size = not_rs.size();
							for (int jj = 0; jj < size; ++jj) {																																
								if (true_props[not_rs[jj]].equals(not_relation)) {									
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
							combined_candidates.push_back(m);
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
					current_stratum_props.push_back(p);
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
					if ((d.variables_[jj] == distinct.items_[1] || d.variables_[jj] == distinct.items_[2]) 
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
					if (subgoal.head() == r_distinct) {
						distincts.push_back(ii);
					} else {
						non_distincts.push_back(ii);
					}
				}
				for (int ii = 0; ii < non_distincts.size(); ++ii) {
					Proposition &subgoal = d.subgoals_[non_distincts[ii]];
					vector<int> variables;
					vector<int> values;
					if (subgoal.headMatches(p) && subgoal.matches(p, variables, values)) {
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
								for (int kk = 0; kk < distincts.size(); ++kk) {

								}
								if (non_distincts.size() == 1) {

								} else {
								}
							}
						}
					}
				}
			}
		}
		while (true) {
			int old_true_props_num = true_props.size();
			vector<Derivation>::iterator j = derivations.begin();
			vector<vector<vector<int> > >::iterator k = der_var_candidates.begin();
			for (; j != derivations.end(); ) {
				if (j->subgoals_.size() == 0) {
					for (int ii = 0; ii < k->size(); ++ii) {
						Proposition prop = j->target_;
						prop.replaceVariables(j->variables_, k->at(ii)); 
						if (true_props_string.find(prop.toRelation().toString()) == true_props_string.end()) {
							true_props.push_back(prop);
							true_props_string.insert(prop.toRelation().toString());
							if (content_relations.find(prop.head()) == content_relations.end()) {
								vector<int> rs;
								content_relations[prop.head()] = rs;
							}
							content_relations[prop.head()].push_back(true_props.size() - 1);
						}
					}					
					j = derivations.erase(j);
					k = der_var_candidates.erase(k);
				} else {
					++j;
					++k;
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