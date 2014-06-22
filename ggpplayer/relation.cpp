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

using namespace std;

unordered_map<string, int> Relation::symbol2code = unordered_map<string, int>();
vector<string> Relation::code2symbol = vector<string>(); // Relation::code2symbol may contain some strings of proposition other than symbols
int Relation::symbol_table_size = 0;

void Relation::addSymbol(string symbol) {
	Relation::symbol2code[symbol] = Relation::code2symbol.size();
	Relation::code2symbol.push_back(symbol);
	++Relation::symbol_table_size;
}

void Relation::initSymbolTable() {
	symbol2code.clear();
	code2symbol.clear();
	symbol_table_size = 0;
	for(int i = 0 ; i < relation_type_num; ++i){		
		Relation::addSymbol(relation_type_words[i]);
	}
}

bool Relation::operator<(const Relation &r) const{
	if (head_ < r.head_) {
		return true;
	} else if (head_ > r.head_) {
		return false;
	}
	if (items_.size() < r.items_.size()){
		return true;
	} else {		
		return false;
	}
	for (int i = 0; i < items_.size() && i < r.items_.size() ; ++i) {
		if (items_[i] < r.items_[i]) {
			return true;
		}else if (r.items_[i] < items_[i]) {
			return false;
		}
	}
	return false;
}

string Relation::toString() const {
	string s = "" + Relation::code2symbol[head_];
	for (int i = 0; i < items_.size(); ++i) {
		if (items_[i].items_.size() > 0) {
			s += " ("+ items_[i].toString() + ")";
		} else {
			s += " "+ items_[i].toString();
		}
	}
	return s;
}

Proposition Relation::toProposition() {
	Proposition ret;
	ret.head_ = head_;
	if (Relation::code2symbol[head_][0] == '?') {
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
	if (s[start] == '(') {
		i = start + 1;
		while (i < size && s[i] == ' ') ++i;
		start = i;
		while (i < size && s[i] != ' ') ++i;
		ret.head_ = Relation::symbol2code[s.substr(start, i - start)];
		while (true) {
			while (i < size && s[i] == ' ') ++i;
			if (i == size || s[i] == ')') break;
			int start = i;
			if (s[start] == '(') {
				int bracket_count = 0;
				do {
					if (s[i] == '(') ++bracket_count;
					if (s[i] == ')') --bracket_count;				
					++i;
				} while (i < size && bracket_count > 0);
			} else {
				while (i < size && s[i] != ' ') ++i;
			}
			ret.items_.push_back(strToProp(s.substr(start, i - start)));
		}
	} else {
		while (i < size && s[i] != ' ') ++i;
		ret.head_ = Relation::symbol2code[s.substr(start, i - start)];
	}
	if (Relation::code2symbol[ret.head_][0] == '?') {
		ret.is_variable_ = true;
	} else {
		ret.is_variable_ = false;
	}
	return ret;
}

string Proposition::propsToStr(Propositions ps) {
	string ret = "";
	for (int i = 0; i < ps.size(); ++i) {
		ret += ps[i].toString() + " ";
	}
	return ret;
}


bool Proposition::operator==(const Proposition &p) const {
	if (head_ != p.head_ || items_.size() != p.items_.size()) {
		return false;
	}
	for (int i = 0; i < items_.size(); ++i) {
		if (!(items_[i] == p.items_[i])) {
			return false;
		}
	}
	return true;
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
	ret.head_ = head_;	
	for (int i = 0; i < items_.size(); ++i) {
		ret.items_.push_back(items_[i].toRelation());
	}
	return ret;
}

bool Proposition::matches(Proposition &p, vector<int> &variables, vector<int> &values) {	
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
		for (int i = 0; i < variables.size(); ++i) {
			if (variables[i] == variable) {
				if (values[i] == value) {
					return true;
				} else {
					return false;					
				}
			}
		}
		variables.push_back(variable);
		values.push_back(value);		
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
		if (value < Relation::symbol_table_size) {
			head_ = value;
		} else {
			Proposition p = strToProp(Relation::code2symbol[value]);
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
		if (Relation::symbol2code.find(s) == Relation::symbol2code.end()) {
			Relation::symbol2code[s] = Relation::code2symbol.size();
			Relation::code2symbol.push_back(s);
		}	
		return Relation::symbol2code[s];
	}	
}

string Proposition::toString() {	
	if (items_.size() == 0) {
		return Relation::code2symbol[head_];
	} else {
		string ret = "( " + Relation::code2symbol[head_];
		for (int i = 0; i < items_.size(); ++i) {
			ret += " " + items_[i].toString();
		}
		ret += " )";
		return ret;
	}
}

Relation Derivation::toRelation() {
	Relation ret;	
	ret.head_ = r_derivation;
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