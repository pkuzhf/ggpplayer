#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <string>

#include "relation.h"
#include "time.h"
#include "prover.h"

using namespace std;


vector<string> Relation::int2string_ = vector<string>();
unordered_map<string, int> Relation::string2int_ = unordered_map<string, int>();


bool Relation::operator<(const Relation &r) const{
	if (content_ < r.content_) {
		return true;
	}else if(content_ > r.content_){
		return false;
	}
	for (int i = 0; i < items_.size() && i < r.items_.size() ; ++i){
		if(items_[i] < r.items_[i]) {
			return true;
		}else if(r.items_[i] < items_[i]){
			return false;
		}
	}
	if(items_.size() < r.items_.size()){
		return true;
	}
	return false;
}

bool Relation::matches(const Relation &r, vector<pair<int, int> > &var_value) const {
	if (items_.size() != r.items_.size()) {
		return false;
	}
	if (type_ != r.type_ || content_ != r.content_) {
		//if ((type_ == r_constant || type_ == r_function && items_.size() == 0) && r.type_ == r_variable) {
		if (type_ == r_constant && r.type_ == r_variable) {			
			var_value.push_back(pair<int, int>(r.content_, content_));			
		//} else if ((r.type_ == r_constant || r.type_ == r_function && r.items_.size() == 0) && type_ == r_variable) {
		} else if (r.type_ == r_constant && type_ == r_variable) {
			var_value.push_back(pair<int, int>(content_, r.content_));			
		} else {
			return false;
		}
	}
	for (int i = 0; i < items_.size(); ++i) {
		if (!items_[i].matches(r.items_[i], var_value)) {
			return false;
		}
	}
	return true;
}

bool Relation::equals(const Relation &r) const {
	if (items_.size() != r.items_.size() || type_ != r.type_ || content_ != r.content_) {
		return false;
	}
	for (int i = 0; i < items_.size(); ++i) {
		if (!items_[i].equals(r.items_[i])) {
			return false;
		}
	}
	return true;
}

string Relation::toString() const {
	string s = "" + Relation::int2string_[content_];
	for (int i = 0; i < items_.size(); ++i) {
		if (items_[i].items_.size() > 0) {
			s += " ("+ items_[i].toString() + ")";
		} else {
			s += " "+ items_[i].toString();
		}
	}
	return s;
}

bool Relation::replaceVariables(vector<pair<int, int> > &m) {
	int start = clock();
	bool replace_all_vars = true;;
	if (type_ == r_variable) {
		bool find = false;
		for (int i = 0; i < m.size(); ++i) {
			if (m[i].first == content_) {
				find = true;
				content_ = m[i].second;
				type_ = r_constant;
			}
		}
		if (!find) {
			replace_all_vars = false;
		}
	}
	for (int i = 0; i < items_.size(); ++i) {
		if (!items_[i].replaceVariables(m)) {
			replace_all_vars = false;
		}
	}
	Prover::time16 += clock() - start;
	return replace_all_vars;
}

Proposition Relation::toProposition() {
	Proposition ret;
	ret.items_[0] = content_;
	if (type_ == r_legal || type_ == r_input || type_ == r_does) {
		ret.items_[1] = items_[0].content_;
		ret.items_[2] = items_[1].content_;
		for (int i = 0; i < items_[1].items_.size(); ++i) {
			ret.items_[i + 3] = items_[1].items_[i].content_;
		}
		ret.item_num_ = items_[1].items_.size() + 3;	
	} else if (type_ == r_true || type_ == r_next || type_ == r_not || type_ == r_init || type_ == r_base) {
		ret.items_[1] = items_[0].content_;
		for (int i = 0; i < items_[0].items_.size(); ++i) {
			ret.items_[i + 2] = items_[0].items_[i].content_;
		}
		ret.item_num_ = items_[0].items_.size() + 2;
		if (type_ == r_not && items_[0].type_ == r_true) {
			for (int i = 0; i < items_[0].items_[0].items_.size(); ++i) {
				ret.items_[ret.item_num_] = items_[0].items_[0].items_[i].content_;
				++ret.item_num_;
			}
		}		
	} else {
		for (int i = 0; i < items_.size(); ++i) {		
			ret.items_[i + 1] = items_[i].content_;
		}
		ret.item_num_ = items_.size() + 1;
	}	
	return ret;
}

Derivation Relation::toDerivation() {
	Derivation ret;
	ret.target_ = items_[0].toProposition();
	for (int i = 1; i < items_.size(); ++i) {
		ret.subgoals_.push_back(items_[i].toProposition());
	}
	return ret;
}

bool Proposition::operator<(const Proposition &p) const{
	if (item_num_ < p.item_num_) {
		return true;
	}else if (item_num_ > p.item_num_) {
		return false;
	}
	for (int i = 0; i < item_num_; ++i) {
		if(items_[i] < p.items_[i]) {
			return true;
		}else if (items_[i] > p.items_[i]) {
			return false;
		}
	}
	return false;
}

Relation Proposition::toRelation() {
	Relation ret;
	ret.content_ = items_[0];
	if (ret.content_ < relation_type_num) {
		ret.type_ = (RelationType)ret.content_;
	} else {
		ret.type_ = r_function;
	}
	if (ret.type_ == r_legal || ret.type_ == r_input || ret.type_ == r_does) {
		Relation role;
		role.content_ = items_[1];
		if (Relation::int2string_[role.content_][0] == '?') {
			role.type_ = r_variable;
		} else {
			role.type_ = r_constant;
		}		
		Relation prop;
		prop.content_ = items_[2];
		if (prop.content_ < relation_type_num) {
			prop.type_ = (RelationType)items_[1];
		} else {
			prop.type_ = r_function;
		}
		for (short int i = 3; i < item_num_; ++i) {
			Relation item;
			item.content_ = items_[i];
			if (Relation::int2string_[item.content_][0] == '?') {
				item.type_ = r_variable;
			} else {
				item.type_ = r_constant;
			}
			prop.items_.push_back(item);
		}
		ret.items_.push_back(role);
		ret.items_.push_back(prop);
	} else if (ret.type_ == r_true || ret.type_ == r_next || ret.type_ == r_init || ret.type_ == r_base) {
		Relation prop;
		prop.content_ = items_[1];
		if (prop.content_ == r_true) {
			prop.type_ = r_true;
			Relation func;
			func.content_ = items_[2];
			func.type_ = r_function;
			for (short int i = 3; i < item_num_; ++i) {
				Relation item;
				item.content_ = items_[i];
				if (Relation::int2string_[item.content_][0] == '?') {
					item.type_ = r_variable;
				} else {
					item.type_ = r_constant;
				}
				func.items_.push_back(item);
			}
			prop.items_.push_back(func);
		} else {
			prop.type_ = r_function;
			for (short int i = 2; i < item_num_; ++i) {
				Relation item;
				item.content_ = items_[i];
				if (Relation::int2string_[item.content_][0] == '?') {
					item.type_ = r_variable;
				} else {
					item.type_ = r_constant;
				}
				prop.items_.push_back(item);
			}
		}
		ret.items_.push_back(prop);
	} else {
		for (short int i = 1; i < item_num_; ++i) {
			Relation item;
			item.content_ = items_[i];
			if (Relation::int2string_[item.content_][0] == '?') {
				item.type_ = r_variable;
			} else {
				item.type_ = r_constant;
			}
			ret.items_.push_back(item);
		}
	}
	return ret;
}

bool Proposition::matches(Proposition p, vector<int> &variables, vector<int> &values) {
	if (item_num_ != p.item_num_) {
		return false;
	}
	for (int i = 0; i < item_num_; ++i) {
		if (items_[i] != p.items_[i]) {
			int variable;
			int value;
			if (isVariable(i) && !p.isVariable(i)) {
				variable = items_[i];
				value = p.items_[i];
			} else if (!isVariable(i) && p.isVariable(i)) {
				value = items_[i];
				variable = p.items_[i];
			} else {
				return false;
			}
			bool find = false;
			for (int j = 0; j < values.size(); ++j) {
				if (variables[j] == variable) {
					if (values[j] != value) {
						return false;
					}
					find = true;
					break;
				}
			}
			if (!find) {
				variables.push_back(variable);
				values.push_back(value);
			}
		}
	}
	return true;
}

bool Proposition::equals(Proposition p) {
	if (item_num_ != p.item_num_) {
		return false;
	}
	for (int i = 0; i < item_num_; ++i) {
		if (items_[i] != p.items_[i]) {
			return false;
		}
	}
	return true;
}

bool Proposition::headMatches(Proposition p) {
	int h = head();
	if (h != p.head()) {
		return false;
	}
	if (h == r_true || h == r_next || h == r_not || h == r_init || h == r_base) {
		return items_[1] == p.items_[1];
	}
	if (h == r_legal || h == r_input || h == r_does) {
		return items_[2] == p.items_[2];
	}
	return true; //r_function
}

void Proposition::removeHead() {
	--item_num_;
	for (int i = 0; i < item_num_; ++i) {
		items_[i] = items_[i + 1];
	}	
}

inline short int Proposition::head() {
	return items_[0];
}

void Proposition::setHead(int head) {
	items_[0] = head;
}
	
void Proposition::setRole(int role) {
	items_[1] = role;
}

vector<int> Proposition::getVarPos() {
	vector<int> ret;
	for (int i = 0; i < item_num_; ++i) {
		if (Relation::int2string_[items_[i]][0] == '?') {
			ret.push_back(i);
		}
	}
	return ret;
}

void Proposition::replaceVariables(vector<int> &variables, vector<int> &values) {
	int size = variables.size();
	for (int i = 0; i < item_num_; ++i) {
		for (int j = 0; j < size; ++j) {
			if (items_[i] == variables[j]) {
				items_[i] = values[j];
				break;
			}
		}
	}
}

inline bool Proposition::isVariable(int idx) {
	return Relation::int2string_[items_[idx]][0] == '?';
}

Relation Derivation::toRelation() {
	Relation ret;
	ret.type_ = r_derivation;
	ret.content_ = r_derivation;
	ret.items_.push_back(target_.toRelation());
	for (int i = 0; i < subgoals_.size(); ++i) {
		ret.items_.push_back(subgoals_[i].toRelation());
	}
	return ret;
}

void Derivation::prepareVariables() {
	set<int> variables;
	for (int i = 0; i < target_.item_num_; ++i) {
		if (Relation::int2string_[target_.items_[i]][0] == '?' && variables.find(target_.items_[i]) == variables.end()) {
			variables_.push_back(target_.items_[i]);
			variables.insert(target_.items_[i]);
		}
	}
	for (int i = 0; i < subgoals_.size(); ++i) {
		for (int j = 0; j < subgoals_[i].item_num_; ++j) {
			if (Relation::int2string_[subgoals_[i].items_[j]][0] == '?' && variables.find(subgoals_[i].items_[j]) == variables.end()) {
				variables_.push_back(subgoals_[i].items_[j]);
				variables.insert(subgoals_[i].items_[j]);
			}
		}
	}
}