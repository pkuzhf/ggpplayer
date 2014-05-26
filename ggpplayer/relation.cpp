#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <string>
#include <algorithm>

#include "relation.h"
#include "time.h"
#include "prover.h"

using namespace std;


vector<string> Relation::int2string_ = vector<string>();
unordered_map<string, int> Relation::string2int_ = unordered_map<string, int>();
int Relation::symbol_table_size_ = 0; // int2string may contain some strings of proposition other than symbols

void Relation::addSymbol(string symbol) {
	string2int_[symbol] = int2string_.size();
	int2string_.push_back(symbol);
	++symbol_table_size_;
}

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
	ret.head_ = content_;
	if (type_ == r_variable) {
		ret.is_variable_ = true;
	} else {
		ret.is_variable_ = false;
	}
	for (int i = 0; i < items_.size(); ++i) {
		ret.items_.push_back(items_[i].toProposition());
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

Proposition Proposition::strToProp(string s) {	
	Proposition ret;
	int i = 0; 
	int size = s.size();
	while (i < size && s[i] == ' ') ++i;
	int start = i;
	while (i < size && s[i] != ' ') ++i;
	ret.head_ = Relation::string2int_[s.substr(start, i - start)];
	while (i < size) {
		while (i < size && s[i] == ' ') ++i;
		if (i == size) break;
		int start = i;
		if (s[i] == '(') {
			int bracket_count = 0;
			do {
				if (s[i] == '(') ++bracket_count;
				if (s[i] == ')') --bracket_count;				
				++i;
			} while (i < size && bracket_count > 0);
			ret.items_.push_back(strToProp(s.substr(start + 1, i - start - 2)));
		} else {
			while (i < size && s[i] != ' ') ++i;
			ret.items_.push_back(strToProp(s.substr(start, i - start)));
		}		
	}
	return ret;
}

bool Proposition::operator<(const Proposition &p) const {
	if (head_ < p.head_) {
		return true;
	} else if (head_ > p.head_) {
		return false;
	}
	if (items_.size() < p.items_.size()) {
		return true;
	} else if (items_.size() > p.items_.size()) {
		return false;
	}
	for (int i = 0; i < items_.size(); ++i) {
		if (items_[i] < p.items_[i]) {
			return true;
		} else if (p.items_[i] < items_[i]) {
			return false;
		}
	}
	return false;
}

Relation Proposition::toRelation() {
	Relation ret;
	ret.content_ = head_;
	if (ret.content_ < relation_type_num) {
		ret.type_ = (RelationType)ret.content_;
	} else if (items_.size() > 0) {
		ret.type_ = r_function;
	} else if (is_variable_) {
		ret.type_ = r_variable;
	} else {
		ret.type_ = r_constant;
	}
	for (int i = 0; i < items_.size(); ++i) {
		ret.items_.push_back(items_[i].toRelation());
	}
	return ret;
}

bool Proposition::matches(Proposition p, vector<int> &variables, vector<int> &values) {	
	if (is_variable_ || p.is_variable_) {
		int variable;
		int value;
		if (is_variable_) {
			variable = head_;
			value = p.getPropCode();
		} else if (p.is_variable_) {
			variable = p.head_;
			value = getPropCode();
		}
		if (find(variables.begin(), variables.end(), variable) == variables.end()) {
			variables.push_back(variable);
			values.push_back(value);
		}
		return true;
	} else {
		if (head_ != p.head_ || items_.size() != p.items_.size()) {
			return false;
		} else {
			for (int i = 0; i < items_.size(); ++i) {
				if (!items_[i].matches(p.items_[i], variables, values)) {
					return false;
				}
			}
			return true;
		}
	}
}

bool Proposition::equals(Proposition p) {
	if (head_ != p.head_ || items_.size() != p.items_.size()) {
		return false;
	} else {
		for (int i = 0; i < items_.size(); ++i) {
			if (!items_[i].equals(p.items_[i])) {
				return false;
			}
		}
		return true;
	}
}

void Proposition::getVariables(vector<int> &variables) {
	for (int i = 0; i < items_.size(); ++i) {
		if (items_[i].is_variable_) {
			if (find(variables.begin(), variables.end(), items_[i].head_) == variables.end()) {
				variables.push_back(items_[i].head_);
			}
		} else {
			items_[i].getVariables(variables);
		}
	}
}

void Proposition::replaceVariables(vector<int> &variables, vector<int> &values) {
	if (is_variable_) {
		int value;
		for (int i = 0; i < variables.size(); ++i) {
			if (variables[i] == head_) {
				value = values[i];
				break;
			}
		}
		if (head_ < Relation::symbol_table_size_) {
			head_ = value;
		} else {
			Proposition p = strToProp(Relation::int2string_[value]);
			head_ = p.head_;
			for (int i = 0; i < p.items_.size(); ++i) {
				items_.push_back(p.items_[i]);
			}
		}
		is_variable_ = false;
	} else {
		for (int i = 0; i < items_.size(); ++i) {
			items_[i].replaceVariables(variables, values);
		}
	}	
}

int Proposition::getPropCode() {
	if (items_.size() == 0) {
		return head_;
	} else {
		string s = toString();
		if (Relation::string2int_.find(s) == Relation::string2int_.end()) {
			Relation::string2int_[s] = Relation::int2string_.size();
			Relation::int2string_.push_back(s);
		}	
		return Relation::string2int_[s];
	}	
}

string Proposition::toString() {	
	if (items_.size() == 0) {
		return Relation::int2string_[head_];
	} else {
		string ret = "( " + Relation::int2string_[head_];
		for (int i = 0; i < items_.size(); ++i) {
			ret += " " + items_[i].toString();
		}
		ret += " )";
		return ret;
	}
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
	variables_.clear();
	target_.getVariables(variables_);
	for (int i = 0; i < subgoals_.size(); ++i) {
		subgoals_[i].getVariables(variables_);
	}
}