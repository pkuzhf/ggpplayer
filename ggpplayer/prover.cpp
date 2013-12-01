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

using namespace std;



Prover::Prover(Relations relations) : relations_(relations) {
	for (int i = 0; i < relations.size(); ++i) {
		if (relations[i].type_ == RelationType::r_derivation) {
			derivations_.push_back(relations[i]);
		}
	}
}


bool Prover::getInitState(State &initstate) {

	dg_.buildGraph(relations_);

	//Reader reader;
	//Relation r;
	//reader.getRelation(dg.node_instances_["true"][0], r, RelationType::r_other);

	for (int i = 0; i < relations_.size(); ++i) {
		if (relations_[i].type_ == RelationType::r_init) {
			initstate.insert(relations_[i].items_[0]);
		} else if (relations_[i].type_ == RelationType::r_role) {
			initstate.insert(relations_[i]);
		} else if (relations_[i].type_ == RelationType::r_function) {
			initstate.insert(relations_[i]);
		}
	}
	return true;
}

bool Prover::getNextState(const State &currentstate, State &nextstate) {
	nextstate = currentstate;
	bool statechanged = false;
	for (int i = 0; i < derivations_.size(); ++i) {
		Relations rs = generateNewRelations(currentstate, derivations_[i]);


		//vector<vector<Relation>> relative_relations;
		//for (int j = 1; j < derivations_[i].items_.size(); ++j) {
		//	vector<Relation> rs;
		//	for (State::iterator k = currentstate.begin(); k != currentstate.end(); ++k) {
		//		if (derivations_[i].items_[j].match(*k)) {
		//			rs.push_back(*k);
		//		}
		//	}
		//	relative_relations.push_back(rs);
		//}
		//int a = 0; a++;
	}
	return true;
}

Relations Prover::generateNewRelations(const State &currentstate, Relation &derivation) {
	Relations rs;
	set<string> vs = derivation.findVariables();
	map<string, set<string>> var_values;
	for (set<string>::iterator i = vs.begin(); i != vs.end(); ++i) {
		set<string> s;
		var_values[*i] = s;
	}

	Relations functions;
	for (int i = 1; i < derivation.items_.size(); ++i) {
		Relations pros = derivation.items_[i].findProposions();
		functions.insert(functions.end(), pros.begin(), pros.end());
	}
	for (int i = 0; i < functions.size(); ++i) {		
		pair<State::iterator, State::iterator> p = currentstate.equal_range(functions[i]);
		for (State::iterator j = p.first; j != p.second; ++j) {
			map<string, string> tmp;
			if (j->matches(functions[i], tmp)) {
				map<string, string> var_value = j->getVarValue(functions[i]);
				for (map<string, string>::iterator k = var_value.begin(); k != var_value.end(); ++k) {				
					var_values[k->first].insert(k->second);
				}
			}
		}		
	}

	vector<string> vars;
	vector<vector<string>> values;
	for (map<string, set<string>>::iterator i = var_values.begin(); i != var_values.end(); ++i) {
		vars.push_back(i->first);
		vector<string> v;
		for (set<string>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
			v.push_back(*j);
		}
		values.push_back(v);
	}

	vector<int> idx;
	for (int i = 0; i < values.size(); ++i) {
		idx.push_back(0);
		if (values[i].size() == 0) {
			return rs;
		}
	}

	do {
		Relation d = derivation;
		map<string, string> m;
		for (int i = 0; i < vars.size(); ++i) {
			m[vars[i]] = values[i][idx[i]];
		}
		if (d.replaceVariables(m)) {
			bool alltrue = true;
			for (int i = 1; i < d.items_.size(); ++i) {
				if (!d.items_[i].validate(currentstate)) {
					alltrue = false;
					break;
				}
			}
			if (alltrue) {
				rs.push_back(d.items_[0]);
			}
		} else {
			// impossible... should be bug...
		}
		int i = 0; 
		while (i < idx.size() && idx[i] == values[i].size() - 1) {
			idx[i] = 0;
			++i;
		}
		if (i < idx.size()) {
			++idx[i];
		} else {
			break;
		}
	} while (true);

	return rs;
}

Relations Prover::getLegalActions(State &state, string &role) {
	Relations rs;
	return rs;
}

string Prover::buildNode(string s, int i){
	char number[10];
	sprintf(number, "%d", i);
	return s + '[' + number + ']';
}

void Prover::findVarDomainInSingleInstance(Relation condition, map<string, set<string>> &var_values) {  // bug : 
	if(dg_.node_instances_.find(condition.content_)!=dg_.node_instances_.end()){
		for(int k=0;k<dg_.node_instances_.at(condition.content_).size();k++){
			Relation r;
			Reader::getRelation(dg_.node_instances_.at(condition.content_).at(k),r,RelationType::r_other);
			map<string, string> var_value_temp;
			if(r.matches(condition,var_value_temp)){
				for(map<string, string>::iterator it=var_value_temp.begin();it!=var_value_temp.end();it++){
					var_values[it->first].insert(it->second);
				}
			}
		}
	}
	else if(condition.isLogic()){  
		for(int i=0;i<condition.items_.size();i++){
			findVarDomainInSingleInstance(condition.items_[i],var_values);
		}
	}
	else {
		// bug 
		int t=-1;
	}
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
	Reader::getRelation(instance, r, RelationType::r_other);

	//
	if(instance.compare("open")==0){
		int tt =1;
	}
	//
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
							/*if(condition.type_!=RelationType::r_distinct){
							for(int k=0;k<dg_.node_instances_.at(condition.content_).size();k++){
							Relation r;
							Reader::getRelation(dg_.node_instances_.at(condition.content_).at(k),r,RelationType::r_other);
							map<string, string> var_value_temp;
							if(r.matches(condition,var_value_temp)){
							for(map<string, string>::iterator it=var_value_temp.begin();it!=var_value_temp.end();it++){
							var_values[it->first].insert(it->second);
							}
							}
							}
							}
							*/

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
						do {
							map<string, string> var_value_enum;
							for (int j = 0; j < vars.size(); ++j) {
								var_value_enum[vars[j]] = values[j][idx[j]];
							}
							condition_satisfy = true;
							for (int j = 1; j < relations_[i].items_.size(); ++j) {
								Relation condition = relations_[i].items_[j];
								condition.replaceVariables(var_value);
								condition.replaceVariables(var_value_enum);
								if (!validateInstance(condition.toString(), true_instances, false_instances, validating_instances)) {
									condition_satisfy = false;
									break;
								}
							}
							if (condition_satisfy) {
								break;
							}
							int k = 0;
							while (k < idx.size() && idx[k] == values[k].size() - 1) {
								idx[k] = 0;			
								++k;
							}
							if (k < idx.size()) {
								++idx[k];
							} else {
								break;
							}
						} while (true);
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
	//
	if(instance.compare("goal xplayer 50")==0){
		int tt =1;
	}
	//

	if (result) {
		true_instances.insert(instance);
		false_instances.clear();
	} else {
		false_instances.insert(instance);
	}
	validating_instances.erase(instance);
	return result;
}

bool Prover::askRole(Relations &rs){
	vector<string>::iterator i;
	for(i=dg_.node_instances_.at("role").begin();i!=dg_.node_instances_.at("role").end();i++){
		string content = "(" ;
		content+= i->data() ;
		content+=  ")";
		Relation r;
		Reader::getRelation(content , r, RelationType::r_other);
		rs.push_back(r);
	}
	return true;
}
bool Prover::askTerminal(const State & state){
	vector<string>::iterator i;
	for(i=dg_.node_instances_.at("terminal").begin();i!=dg_.node_instances_.at("terminal").end();i++){
		string s=i->data();
		set<string> true_instances,false_instances,validating_instances;
		State::iterator i;
		for(i=state.begin();i!=state.end();i++){
			true_instances.insert(i->toString());
		}
		if(validateInstance(s,true_instances,false_instances,validating_instances)){
			return true;	
		}
	}
	return false;
}


bool Prover::askGoal(Relations &rs,const State & state ){
	vector<string>::iterator i;
	for(i=dg_.node_instances_.at("goal").begin();i!=dg_.node_instances_.at("goal").end();i++){
		Relation r;
		string s=i->data();
		set<string> true_instances,false_instances,validating_instances;
		State::iterator j;
		for(j=state.begin();j!=state.end();j++){
			true_instances.insert(j->toString());
		}
		if(validateInstance(s,true_instances,false_instances,validating_instances)){
			Reader::getRelation(s , r, RelationType::r_other);
			rs.push_back(r);	
		}
	}
	return true;
}

bool Prover::askLegalActions(Relations &rs,const State state ){
	vector<string>::iterator i;
	for(i=dg_.node_instances_.at("legal").begin();i!=dg_.node_instances_.at("legal").end();i++){
		Relation r;
		string s=i->data();
		set<string> true_instances,false_instances,validating_instances;
		State::iterator j;
		for(j=state.begin();j!=state.end();j++){
			true_instances.insert(j->toString());
		}
		if(validateInstance(s,true_instances,false_instances,validating_instances)){
			Reader::getRelation(s , r, RelationType::r_other);
			rs.push_back(r);	
		}
	}
	return true;
}

bool Prover::askLegalActions(Relations &rs,Relation role,const State state ){
	Prover::askLegalActions(rs, state );
	Relations::iterator i = rs.begin() ; 
	while(i!=rs.end()){
		if(i->items_[0].content_!=role.items_[0].content_){
			i=rs.erase(i);
		}
		else {
			i++;
		}
	}
	return true;
}

bool Prover::askNextState(State &nextstate,const State state,const Relations does){
	vector<string>::iterator i;
	for(i=dg_.node_instances_.at("next").begin();i!=dg_.node_instances_.at("next").end();i++){
		Relation r;
		string s=i->data();
		set<string> true_instances,false_instances,validating_instances;
		for(State::iterator j=state.begin();j!=state.end();j++){
			true_instances.insert(j->toString());
		}
		for(int j=0;j<does.size();j++){
			true_instances.insert(does[j].toString());
		}
		if(validateInstance(s,true_instances,false_instances,validating_instances)){
			s=s.substr(s.find(' ')+1);
			Reader::getRelation(s , r, RelationType::r_other);
	//		r=r.items_[0];
			nextstate.insert(r);
		}
	}
	return true;
}