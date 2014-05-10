#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "prover.h"
#include "tools.h"
#include "relation.h"
#include "domaingraph.h"
#include "reader.h"
#include "dependgraph.h"
using namespace std;



Prover::Prover(Relations relations) : relations_(relations) {}


string Prover::getInitState() {

	//dg_.buildGraph(relations_);
	dpg_.buildGraph(relations_);

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
		if( relations_[i].type_ == RelationType::r_init ){
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

void Prover::findVarDomainInSingleInstance(Relation condition, map<string, set<string>> &var_values) {  
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

	
	if (r.type_ == RelationType::r_true) {
		if (validateInstance(r.items_[0].toString(), true_instances, false_instances, validating_instances)) {
			result = true;
		} else {
			result = false;
		}
	} 
	else if(r.type_==RelationType::r_does){
		result =  false;
	}
	else if (r.isLogic()) {
		switch (r.type_) {
		case RelationType::r_and:
			result = true;
			for (int i = 0; i < r.items_.size(); ++i) {
				if (!validateInstance(r.items_[i].toString(), true_instances, false_instances, validating_instances)) {
					result = false;
					break;
				}
			}
			break;
		case RelationType::r_or:
			result = false;
			for (int i = 0; i < r.items_.size(); ++i) {
				if (validateInstance(r.items_[i].toString(), true_instances, false_instances, validating_instances)) {
					result = true;
					break;
				}
			}
			break;
		case RelationType::r_not:
			if (validateInstance(r.items_[0].toString(), true_instances, false_instances, validating_instances)) {
				result = false;
			} else {
				result = true;
			}
			break;
		case RelationType::r_distinct:
			if (r.items_[0].type_ == RelationType::r_constant && r.items_[1].type_ == RelationType::r_constant) {
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
			if (relations_[i].type_ == RelationType::r_derivation) {
				map<string, string> var_value;
				if (relations_[i].items_[0].matches(r, var_value)) {
					bool condition_satisfy = true;		
					bool contain_variables = false;
					map<string, set<string>> var_values;
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
						vector<vector<string>> values;
						vector<int> idx;
						bool none_value_var = false;
						for (map<string, set<string>>::iterator j = var_values.begin(); j != var_values.end(); ++j) {
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

bool Prover::conditions_satisfied(Relation relation, map<string, string> var_value, vector<string> vars, vector<vector<string>> values, int condition_count, set<string> &true_instances, set<string> &false_instances, set<string> &validating_instances){
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
			vector<vector<string>> new_values = values;
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

bool Prover::askTerminal(const string & state){
	set<string> true_instances, false_instances, validating_instances;
	for(int i = 0; i < state.size(); ++i){
		if(state[i] == '1'){
			true_instances.insert(keyrelations_[i].toString());
		}
	}
	for(vector<string>::iterator i = dg_.node_instances_.at("terminal").begin(); i != dg_.node_instances_.at("terminal").end(); i++){
		string s = i->data();
		if(validateInstance(s, true_instances, false_instances, validating_instances)){
			return true;	
		}
	}
	return false;
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
		r.type_ = RelationType::r_legal;
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
		r.type_ = RelationType::r_next;
		r.items_.push_back(keyrelations_[i]);

		if(validateInstance(r.toString(), true_instances, false_instances, validating_instances)){
			rtn += '1';
		}else {
			rtn += '0';
		}
	}
	return rtn;
}

string Prover::askNextStateByDG(const string &currentstate, const string &does) {

	return "";
}