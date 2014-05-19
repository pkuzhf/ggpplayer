#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <algorithm>

#include "prover.h"
#include "relation.h"
#include "domaingraph.h"
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
			r.content_ = "true";
			r.type_ = r_true;
			keyrelations_.push_back(r);
			keyrelation_num_[r] = keyrelation_num_.size();
		} else if(true_rs[i].type_ == r_input){
			inputs_.push_back(true_rs[i]);
			Relation r = true_rs[i];
			r.content_ = "legal";
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
			string temp = relations_[i].items_[0].content_;
			if(find(key_head_.begin(), key_head_.end(), temp) == key_head_.end()){
				key_head_.push_back(temp);
			}
		} else if(relations_[i].type_ == r_derivation && (relations_[i].items_[0].type_ == r_next)){
			string temp = relations_[i].items_[0].items_[0].content_;
			if(find(key_head_.begin(), key_head_.end(), temp) == key_head_.end()){
				key_head_.push_back(temp);
			}
		} else if(relations_[i].type_ == r_role){
			roles_.push_back(relations_[i]);
		}
	}

	dpg2_.buildGraph(nonstatic_derivations_); 
}

void Prover::getStaticRelation()
{
	vector<int> mark;
	for(int i = 0 ; i < dpg_.init_nodes_.size(); ++i){
		mark.push_back(0);
	}
	markNonStatic(dpg_.node_num_["does"], mark);
	markNonStatic(dpg_.node_num_["true"], mark);
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


Relations Prover::getInitStateByDPG() {
	Relations rtn;
	for(int i = 0 ; i < inits_.size(); ++i){
		Relation r = inits_[i];
		if(find(key_head_.begin(), key_head_.end(), r.items_[0].content_) != key_head_.end()){
			r.content_ = "true";
			r.type_ = r_true;
			rtn.push_back(r);
		}
	}
	return rtn;
}

string Prover::getInitState() {

	dg_.buildGraph(relations_);	

	set<Relation> temp_keyrelations;
	for (vector<string>::iterator i = dg_.node_instances_["next"].begin(); i != dg_.node_instances_.at("next").end() ; ++i) {
		Relation r;
		r = Reader::getRelation(*i);
		temp_keyrelations.insert(r.items_[0]);
	}
	for (int i = 0; i < relations_.size(); ++i){
		if(relations_[i].content_.compare("init") == 0) {
			temp_keyrelations.insert(relations_[i].items_[0]);
		}
	}
	for(set<Relation>::iterator i = temp_keyrelations.begin(); i != temp_keyrelations.end(); ++i){
		keyrelations_.push_back(*i);
	}
	for( int i = 0; i < keyrelations_.size(); ++i){
		keyrelation_num_[keyrelations_[i]] = i;
	}// get keyrelation_num_ map

	for (vector<string>::iterator i = dg_.node_instances_["does"].begin(); i != dg_.node_instances_.at("does").end() ; ++i) {
		Relation r;
		r = Reader::getRelation(*i);
		legalactions_.push_back(r);
	}
	for( int i = 0; i < legalactions_.size(); ++i){
		legalaction_num_[legalactions_[i]] = i;
	}// get legalactions_num map

	for (vector<string>::iterator i = dg_.node_instances_["role"].begin(); i != dg_.node_instances_.at("role").end() ; ++i) {
		Relation r;
		r = Reader::getRelation(*i);
		roles_.push_back(r.items_[0]);
	}
	for( int i = 0; i < roles_.size(); ++i){
		role_num_[roles_[i]] = i;
	}// get roles_num maap

	cout << "map and vector ok~"<<endl;

	string rtn;
	set<string> true_instance, false_instance, valideting_instance;
	for( int i = 0; i < relations_.size(); ++i){
		if( relations_[i].type_ == r_init ){
			true_instance.insert(relations_[i].items_[0].toString());
		}
	}
	for( int i = 0; i < keyrelations_.size(); ++i){
		if(validateInstance(keyrelations_[i].toString(),true_instance ,false_instance, valideting_instance)){
			rtn += '1';
		}else{
			rtn += '0';
		}
	}

	cout << "init_state ok"<<endl;

	return rtn;
}

string Prover::buildNode(string s, int i){
	char number[10];
	sprintf(number, "%d", i);
	return s + '[' + number + ']';
}

void Prover::findVarDomainInSingleInstance(Relation condition, map<string, set<string> > &var_values) {  
	if(dg_.node_instances_.find(condition.content_) != dg_.node_instances_.end()){
		for(int k = 0;k < dg_.node_instances_.at(condition.content_).size(); k++){
			Relation r;
			r = Reader::getRelation(dg_.node_instances_.at(condition.content_).at(k));
			map<string, string> var_value_temp;
			if(r.matches(condition, var_value_temp)){
				for(map<string, string>::iterator it = var_value_temp.begin(); it != var_value_temp.end(); it++){
					var_values[it->first].insert(it->second);
				}
			}
		}
	}
	else if(condition.isLogic()){  
		for(int i = 0; i < condition.items_.size(); i++){
			findVarDomainInSingleInstance(condition.items_[i], var_values);
		}
	}
	else {

	}
}

bool Prover::contain_var(Relation r, string var){
	if(r.content_ == var) {
		return true;
	}
	for(int i = 0; i < r.items_.size(); ++i){
		if(contain_var(r.items_[i], var)){
			return true;	
		}
	}
	return false;
}


bool Prover::validateInstance(string instance, set<string> &true_instances, set<string> &false_instances, set<string> &validating_instances){

	
	if (true_instances.find(instance) != true_instances.end()) {
		return true;
	}
	if (false_instances.find(instance) != false_instances.end()) {
		return false;
	}
	if (validating_instances.find(instance) != validating_instances.end()) {
		return false;
	} else {
		validating_instances.insert(instance);
	}
	bool result = false;
	Relation r;
	r = Reader::getRelation(instance);

	
	if (r.type_ == r_true) {
		if (validateInstance(r.items_[0].toString(), true_instances, false_instances, validating_instances)) {
			result = true;
		} else {
			result = false;
		}
	} 
	else if(r.type_==r_does){
		result =  false;
	}
	else if (r.isLogic()) {
		switch (r.type_) {
		case r_and:
			result = true;
			for (int i = 0; i < r.items_.size(); ++i) {
				if (!validateInstance(r.items_[i].toString(), true_instances, false_instances, validating_instances)) {
					result = false;
					break;
				}
			}
			break;
		case r_or:
			result = false;
			for (int i = 0; i < r.items_.size(); ++i) {
				if (validateInstance(r.items_[i].toString(), true_instances, false_instances, validating_instances)) {
					result = true;
					break;
				}
			}
			break;
		case r_not:
			if (validateInstance(r.items_[0].toString(), true_instances, false_instances, validating_instances)) {
				result = false;
			} else {
				result = true;
			}
			break;
		case r_distinct:
			if (r.items_[0].type_ == r_constant && r.items_[1].type_ == r_constant) {
				if (r.items_[0].content_ != r.items_[1].content_) {
					result = true;
				} else {
					result = false;
				}
			} else {
				//impossible
			}
			break;
		}
	} else {
		for (int i = 0; i < relations_.size(); ++i) {
			if (relations_[i].type_ == r_derivation) {
				map<string, string> var_value;
				if (relations_[i].items_[0].matches(r, var_value)) {
					bool condition_satisfy = true;		
					bool contain_variables = false;
					map<string, set<string> > var_values;
					for (int j = 1; j < relations_[i].items_.size(); ++j) {
						Relation condition = relations_[i].items_[j];
						condition.replaceVariables(var_value);
						if (condition.findVariables().size() != 0) {
							contain_variables = true;
							findVarDomainInSingleInstance(condition, var_values);
						}
					}
					if (contain_variables) {
						vector<string> vars;
						vector<vector<string> > values;
						vector<int> idx;
						bool none_value_var = false;
						for (map<string, set<string> >::iterator j = var_values.begin(); j != var_values.end(); ++j) {
							if (j->second.size() == 0) {
								none_value_var = true;
								break;
							}
							idx.push_back(0);
							vars.push_back(j->first);
							vector<string> v;
							v.insert(v.begin(), j->second.begin(), j->second.end());
							values.push_back(v);								
						}	
						condition_satisfy = false;
						if (none_value_var) {																
							break;
						}
						
						if(conditions_satisfied(relations_[i],var_value, vars, values, 1, true_instances, false_instances, validating_instances)){ // ÅÐ¶ÏÊÇ·ñÂú×ã
							condition_satisfy = true;
						}
					} else {
						condition_satisfy = true;
						for (int j = 1; j < relations_[i].items_.size(); ++j) {
							Relation condition = relations_[i].items_[j];
							condition.replaceVariables(var_value);
							if (!validateInstance(condition.toString(), true_instances, false_instances, validating_instances)) {
								condition_satisfy = false;
								break;
							}
						}
					}		
					
					if (condition_satisfy) {
						result = true;
						break;
					}
				}			
			} else {
				if (relations_[i].equals(r)) {				
					result = true;
					break;
				}
			}
		}
	}
	if (result) {
		true_instances.insert(instance);
		false_instances.clear();
	} else {
		false_instances.insert(instance);
	}
	validating_instances.erase(instance);
	return result;
}

bool Prover::conditions_satisfied(Relation relation, map<string, string> var_value, vector<string> vars, vector<vector<string> > values, int condition_count, set<string> &true_instances, set<string> &false_instances, set<string> &validating_instances){
	if(condition_count == relation.items_.size()){
		return true;
	}
	vector<int> var_idx;
	for(int i = 0; i < vars.size(); ++i){
		if(contain_var(relation.items_[condition_count], vars[i])){
			var_idx.push_back(i);
		}
	}
	vector<int> idx;
	for(int i = 0; i < var_idx.size(); ++i){
		idx.push_back(0);
	}

	do {
		map<string, string> var_value_enum;
		for (int k = 0; k < var_idx.size(); ++k) {
			var_value_enum[vars[var_idx[k]]] = values[var_idx[k]][idx[k]];
		}
		Relation r = relation.items_[condition_count];
		r.replaceVariables(var_value);
		r.replaceVariables(var_value_enum);
		if (validateInstance(r.toString(), true_instances, false_instances, validating_instances)) {
			vector<vector<string> > new_values = values;
			for(int k = 0; k < var_idx.size(); ++k){
				new_values[var_idx[k]].clear();
				new_values[var_idx[k]].push_back(values[var_idx[k]][idx[k]]);
			}
			if(conditions_satisfied(relation, var_value, vars, new_values, condition_count + 1, true_instances, false_instances, validating_instances)){
				return true;
			}
		}
		
		
		int k = 0;
		while (k < var_idx.size() && idx[k] == values[var_idx[k]].size() - 1) {
			idx[k] = 0;			
			++k;
		}
		if (k < var_idx.size()) {
			++idx[k];
		} else {
			break;
		}
	} while (true);



	return false;
}

int Prover::askRole(Relation &role){
	return role_num_[role];
}



bool Prover::askGoal(vector<int> &result, const string & state ){
	result.resize(roles_.size());
	set<string> true_instances, false_instances, validating_instances;
	for(int j = 0; j < state.size(); ++j){
		if(state[j] == '1'){
			true_instances.insert(keyrelations_[j].toString());
		}
	}
	for(int i = 0; i < dg_.node_instances_["goal"].size(); ++i){
		Relation r;
		string s = dg_.node_instances_.at("goal").at(i);
		if(validateInstance(s, true_instances, false_instances, validating_instances)){
			r = Reader::getRelation(s);
			result[role_num_.at(r.items_[0])] = atoi(r.items_[1].toString().c_str());
		}
	}
	return true;
}

string Prover::askLegalActions(const string &state){
	string rtn;
	set<string> true_instances, false_instances, validating_instances;
	for(int i = 0; i < state.size(); ++i){
		if(state[i] == '1'){
			true_instances.insert(keyrelations_[i].toString());
		}
	}
	for(int i = 0; i < legalactions_.size(); ++i){
		Relation r = legalactions_[i];
		r.content_ = "legal";
		r.type_ = r_legal;
		if(validateInstance(r.toString(), true_instances, false_instances, validating_instances)){
			rtn += '1';
		}else{
			rtn += '0';
		}
	}
	return rtn;
}

string Prover::askLegalActions(int role,const string &state ){
	string rtn = askLegalActions(state);
	for( int i = 0; i < rtn.size(); ++i){
		if(rtn[i] == '1' && role_num_[legalactions_[i].items_[0]] != role){
			rtn[i] = '0';
		}
	}
	return rtn;
}

string Prover::askNextState(const string &currentstate, const string &does){
	string rtn;
	set<string> true_instances, false_instances, validating_instances;
	for(int i = 0; i < currentstate.size(); ++i){
		if(currentstate[i] == '1'){
			true_instances.insert(keyrelations_[i].toString());
		}
	}
	for(int i = 0; i < does.size(); ++i){
		if(does[i] == '1'){
			true_instances.insert(legalactions_[i].toString());
		}
	}

	for(int i = 0; i < keyrelations_.size(); ++i){
		Relation r;
		r.content_ = "next";
		r.type_ = r_next;
		r.items_.push_back(keyrelations_[i]);

		if(validateInstance(r.toString(), true_instances, false_instances, validating_instances)){
			rtn += '1';
		}else {
			rtn += '0';
		}
	}
	return rtn;
}

void Prover::askNextStateByDPG(Relations &currentstate, Relations &does) {
	Relations true_relations;
	true_relations.insert(true_relations.end(), statics_.begin(), statics_.end());
	true_relations.insert(true_relations.end(), currentstate.begin(), currentstate.end());
	true_relations.insert(true_relations.end(), does.begin(), does.end());
	generateTrueProps(true_relations);
}

Relations Prover::generateTrueProps(Relations true_props) {	
	map<string, vector<int> > content_relations;
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
			Relation d = derivations_[dpg_.stra_deriv_[i][j]];
			vector<int> lower_stratum_subgoals;
			vector<int> current_stratum_subgoals;
			vector<int> not_subgoals;
			vector<int> distinct_subgoals;
			vector<vector<map<string, string> > > var_candidates;
			vector<int> idx;
			bool impossible = false;
			for (int k = 1; k < d.items_.size(); ++k) {								
				if (d.items_[k].type_ == r_not) {
					not_subgoals.push_back(k);
				} else if (d.items_[k].type_ == r_distinct) {
					distinct_subgoals.push_back(k);
				} else if (dpg_.node_stra_[dpg_.node_num_[d.items_[k].content_]] < i) { // lower stratum subgoals					
					//lower_stratum_subgoals.push_back(k);
					vector<map<string, string> > candidates;
					for (int ii = 0; ii < content_relations[d.items_[k].content_].size(); ++ii) { // scan all true props to generate var-value maps
						map<string, string> var_value;
						if (d.items_[k].matches(true_props[content_relations[d.items_[k].content_][ii]], var_value)) {
							bool duplicated = false;
							for (int jj = 0; jj < candidates.size(); ++jj) {
								bool equal = true;
								for (map<string, string>::iterator kk = var_value.begin(); kk != var_value.end(); ++kk) {
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
						map<string, string> m;
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
			vector<map<string, string> > maps;
			map<string, string> m;
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
				map<string, string> m = maps[maps.size() - 1];
				map<string, string> m2 = var_candidates[k][idx[k]];
				bool combined = true;
				for (map<string, string>::iterator ii = m2.begin(); ii != m2.end(); ++ii) {
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
								map<string, string> m;
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