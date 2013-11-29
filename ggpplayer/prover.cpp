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

using namespace std;



Prover::Prover(Relations relations) : relations_(relations) {
	for (int i = 0; i < relations.size(); ++i) {
		if (relations[i].type_ == RelationType::r_derivation) {
			derivations_.push_back(relations[i]);
		}
	}
}


bool Prover::getInitState(State &initstate) {
	DomainGraph dg;
	dg.buildGraph(relations_);
	
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
