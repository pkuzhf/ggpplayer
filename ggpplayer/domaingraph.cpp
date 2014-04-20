#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "domaingraph.h"
#include "tools.h"
#include "relation.h"
#include "reader.h"

using namespace std;

void DomainGraph::removeNodesByType(Relation r, RelationType type) {
	vector<Relation>::iterator i = r.items_.begin();
	while (i != r.items_.end()) {
		if (i->type_ == type) {			
			i = r.items_.erase(i);
		} else {
			removeNodesByType(*i, type);			
			++i;
		}
	}
}

void DomainGraph::buildGraph(Relations rs) {

	cout<<"buildGraphBySingleRelation"<<endl;

	for (int i = 0; i < rs.size(); ++i) {
		//removeNodesByType(rs[i], RelationType::r_distinct);

		buildGraphBySingleRelation(rs[i]);
	}
	if (node_num_.find("does[0]") != node_num_.end() && node_num_.find("legal[0]") != node_num_.end()) {
		edges_[node_num_["does[0]"]].insert(node_num_["legal[0]"]);
	}
	if (node_num_.find("does[1]") != node_num_.end() && node_num_.find("legal[1]") != node_num_.end()) {
		edges_[node_num_["does[1]"]].insert(node_num_["legal[1]"]);
	}

	cout<<"buildMaximalInstancesForNode"<<endl;

	for (int i = 0; i < nodes_.size(); ++i) {
		set<string> path;
		buildMaximalInstancesForNode(nodes_[i], path); 
	}

	cout<<"removeNodesByType"<<endl;

	for (int i = 0; i < rs.size(); ++i) {
		removeNodesByType(rs[i], RelationType::r_not);
	}

	cout<<"validating_instances"<<endl;

	set<string> true_instances;
	set<string> false_instances;
	for (int i = 0; i < nodes_.size(); ++i) {
		if (nodes_[i][nodes_[i].size() - 1] != ']') {			
			vector<string>::iterator j = node_instances_[nodes_[i]].begin();			
			while (j != node_instances_[nodes_[i]].end()) {
				set<string> validating_instances;
				if (!validateInstance(*j, true_instances, false_instances, validating_instances, rs)) {
					j = node_instances_[nodes_[i]].erase(j);
				} else {
					++j;
				}						
			}	
			if (node_instances_[nodes_[i]].size() == 0) {
				node_instances_.erase(nodes_[i]);
			}
		}
	}

	for (int i = 0; i < nodes_.size(); ++i) { // don't merge this loop to above loop, because these nodes are used in validateInstance.
		if (nodes_[i][nodes_[i].size() - 1] == ']') {
			node_instances_.erase(nodes_[i]);
		}
	}
}

void DomainGraph::buildMaximalInstancesForNode(string node, set<string> path) {
	static set<string> cache_set;
	if(cache_set.find(node) == cache_set.end()){
		cache_set.insert(node);
	} else{
		return;
	}
	
	cout<<node<<endl;
	path.insert(node);
	vector<string> instances;
	if (node[node.size() - 1] == ']') {  // ²ÎÊý
		for (set<int>::iterator i = edges_[node_num_[node]].begin(); i != edges_[node_num_[node]].end(); ++i) {
			if (path.find(nodes_[*i]) == path.end()) {
				buildMaximalInstancesForNode(nodes_[*i], path);
				instances.insert(instances.end(), node_instances_[nodes_[*i]].begin(), node_instances_[nodes_[*i]].end());
			} else{
				cache_set.erase(node);
			}
		}
		instances = Tools::removeVectorDuplicates(instances);
	} else {   
		int item_num = 0;
		for (int i = 0; node_num_.find(buildNode(node, i)) != node_num_.end(); ++i) {
			if (path.find(buildNode(node, i)) == path.end()) {
				buildMaximalInstancesForNode(buildNode(node, i), path);
			}
			item_num = i + 1;
		}		
		if (item_num == 0) {
			instances.push_back(node);
		} else {
			vector<int> item_instance_nums;
			vector<int> idx;
			for (int i = 0; i < item_num; ++i) {
				item_instance_nums.push_back(node_instances_[buildNode(node, i)].size());
				idx.push_back(0);
			}
			bool empty = false;
			for (int i = 0; i < item_instance_nums.size(); ++i) {
				if (item_instance_nums[i] == 0) {
					empty = true;
				}
			}
			if (!empty) {
				do {
					string instance = node;
					Relation r;
					r.content_ = node;
					for (int i = 0; i < idx.size(); ++i) {
						Relation item;
						Reader::getRelation(node_instances_[buildNode(node, i)][idx[i]], item, RelationType::r_other);
						//item.content_ = node_instances_[buildNode(node, i)][idx[i]];
						r.items_.push_back(item);						
					}
					instance = r.toString();
					instances.push_back(instance);
					{
						int i = 0;
						while (i < idx.size() && idx[i] == item_instance_nums[i] - 1) {
							idx[i] = 0;
							++i;
						}
						if (i == idx.size()) {
							break;
						} else {
							++idx[i];
						}
					}
				} while (true);
			}
		}		
	}
	
	for(int i = 0 ; i < instances.size(); ++i){
		node_instances_[node].push_back(instances[i]);
	}
	node_instances_[node] = Tools::removeVectorDuplicates(node_instances_[node]);
	
	//node_instances_[node] = instances;  
}

void DomainGraph::buildGraphBySingleRelation(Relation r) {	

	bool is_a_node = true;
	if (r.type_ == RelationType::r_or ||
		r.type_ == RelationType::r_and ||
		r.type_ == RelationType::r_not ||
		r.type_ == RelationType::r_distinct ||
		r.type_ == RelationType::r_derivation ||
		r.type_ == RelationType::r_variable ||
		node_num_.find(r.content_) != node_num_.end()) {
		is_a_node = false;
	}
	if (is_a_node) {
		addNode(r.content_);		
	}
	for (int i = 0; i < r.items_.size(); ++i) {
		if (is_a_node) {		
			addNode(buildNode(r.content_, i));			
		}
		buildGraphBySingleRelation(r.items_[i]);			
	}


	//add edges
	if (node_num_.find(r.content_) != node_num_.end()) {
		for (int i = 0; i < r.items_.size(); ++i) {
			if (node_num_.find(r.items_[i].content_) != node_num_.end()) {				
				edges_[node_num_[buildNode(r.content_, i)]].insert(node_num_[r.items_[i].content_]);
			}
		}
	}
	if (r.type_ == RelationType::r_derivation) { 
		set<string> head_vars = r.items_[0].findVariables();
		map<string, set<int>> headvar_nodes;
		for (set<string>::iterator i = head_vars.begin(); i != head_vars.end(); ++i) {
			set<int> s;
			headvar_nodes[*i] = s;
		}
		findVariableNode(r.items_[0], headvar_nodes);

		set<string> body_vars;
		map<string, set<int>> bodyvar_nodes;
		for (int i = 1; i < r.items_.size(); ++i) {
			set<string> vars = r.items_[i].findVariables();
			body_vars.insert(vars.begin(), vars.end());
		}
		for (set<string>::iterator i = body_vars.begin(); i != body_vars.end(); ++i) {
			set<int> s;
			bodyvar_nodes[*i] = s;
		}
		for (int i = 1; i < r.items_.size(); ++i) {
			findVariableNode(r.items_[i], bodyvar_nodes);
		}

		for (set<string>::iterator i = head_vars.begin(); i != head_vars.end(); ++i) {
			if (body_vars.find(*i) != body_vars.end()) {
				set<int> headnodes = headvar_nodes[*i];
				set<int> bodynodes = bodyvar_nodes[*i];
				for (set<int>::iterator j = headnodes.begin(); j != headnodes.end(); ++j) {
					for (set<int>::iterator k = bodynodes.begin(); k != bodynodes.end(); ++k) {
						if (*k != *j) {
							edges_[*j].insert(*k);
						}
					}
				}
			}
		}
	}
}

void DomainGraph::addNode(string node) {
	node_num_[node] = nodes_.size();
	nodes_.push_back(node);
	set<int> s;
	edges_.push_back(s);
}

void DomainGraph::findVariableNode(Relation r, map<string, set<int>> &m) {
	if (node_num_.find(r.content_) != node_num_.end()) {
		for (int i = 0; i < r.items_.size(); ++i) {
			if (r.items_[i].type_ == RelationType::r_variable) {
				m[r.items_[i].content_].insert(node_num_[buildNode(r.content_, i)]);
			}
		}
	}
	for (int i = 0; i < r.items_.size(); ++i) {
		findVariableNode(r.items_[i], m);
	}
}

string DomainGraph::buildNode(string s, int i) {
	char number[10];
	sprintf(number, "%d", i);
	return s + '[' + number + ']';
}

bool DomainGraph::validateInstance(string instance, set<string> &true_instances, set<string> &false_instances, set<string> &validating_instances, Relations &rs) {
	cout<<instance<<endl;
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


	if (r.type_ == RelationType::r_true) {
		if (validateInstance(r.items_[0].toString(), true_instances, false_instances, validating_instances, rs)) {
			result = true;
		} else {
			result = false;
		}
	} else if(r.type_== RelationType::r_init){
		result = false;
	} else if (r.type_ == RelationType::r_does) {
		r.content_ = "legal";
		r.type_ = RelationType::r_legal;
		if (validateInstance(r.toString(), true_instances, false_instances, validating_instances, rs)) {
			result = true;
		} else {
			result = false;
		}
	} else if (r.isLogic()) {
		switch (r.type_) {
		case RelationType::r_and:
			result = true;
			for (int i = 0; i < r.items_.size(); ++i) {
				if (!validateInstance(r.items_[i].toString(), true_instances, false_instances, validating_instances, rs)) {
					result = false;
					break;
				}
			}
			break;
		case RelationType::r_or:
			result = false;
			for (int i = 0; i < r.items_.size(); ++i) {
				if (validateInstance(r.items_[i].toString(), true_instances, false_instances, validating_instances, rs)) {
					result = true;
					break;
				}
			}
			break;
		case RelationType::r_not:
			/*
			if (validateInstance(r.items_[0].toString(), true_instances, false_instances, validating_instances, rs)) {
				result = false;
			} else {
				result = true;
			}
			*/
			result = true;
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
		for (int i = 0; i < rs.size(); ++i) {
			if (rs[i].type_ == RelationType::r_derivation) {
				map<string, string> var_value;
				if (rs[i].items_[0].matches(r, var_value) ||
					(rs[i].items_[0].type_ == RelationType::r_next && rs[i].items_[0].items_[0].matches(r, var_value))) {
					bool condition_satisfy = true;		
					bool contain_variables = false;
					map<string, set<string>> var_values;
					for (int j = 1; j < rs[i].items_.size(); ++j) {
						Relation condition = rs[i].items_[j];
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
						do {
							map<string, string> var_value_enum;
							for (int j = 0; j < vars.size(); ++j) {
								var_value_enum[vars[j]] = values[j][idx[j]];
							}
							condition_satisfy = true;
							for (int j = 1; j < rs[i].items_.size(); ++j) {
								Relation condition = rs[i].items_[j];
								condition.replaceVariables(var_value);
								condition.replaceVariables(var_value_enum);
								if (!validateInstance(condition.toString(), true_instances, false_instances, validating_instances, rs)) {
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
						for (int j = 1; j < rs[i].items_.size(); ++j) {
							Relation condition = rs[i].items_[j];
							condition.replaceVariables(var_value);
							if (!validateInstance(condition.toString(), true_instances, false_instances, validating_instances, rs)) {
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
			} else if (rs[i].type_ == RelationType::r_init) {
				if (rs[i].items_[0].equals(r)) {
					result = true;
					break;
				}
			} else {
				if (rs[i].equals(r)) {				
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

void DomainGraph::findVarDomainInSingleInstance(Relation r, map<string, set<string>> &var_values) {
	for (int i = 0; i < r.items_.size(); ++i) {
		if (r.items_[i].type_ == RelationType::r_variable) {
			if (node_instances_.find(buildNode(r.content_, i)) == node_instances_.end()) {
				continue;
			}
			vector<string> curvalues = node_instances_[buildNode(r.content_, i)];
			set<string> newvalues;
			if (var_values.find(r.items_[i].content_) == var_values.end()) {
				newvalues.insert(curvalues.begin(), curvalues.end());
			} else {
				set<string> prevalues = var_values[r.items_[i].content_];
				for (int i = 0; i < curvalues.size(); ++i) {
					if (prevalues.find(curvalues[i]) != prevalues.end()) {
						newvalues.insert(curvalues[i]);
					}
				}
			}
			var_values[r.items_[i].content_] = newvalues;
		}
	}
	for (int i = 0; i < r.items_.size(); ++i) {
		findVarDomainInSingleInstance(r.items_[i], var_values);
	}
}