#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "relation.h"
#include "time.h"
#include "prover.h"

using namespace std;


vector<string> Relation::int2string_ = vector<string>();
map<string, int> Relation::string2int_ = map<string, int>();


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