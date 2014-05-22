#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "relation.h"

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

bool Relation::matches(const Relation &r, map<int, int> &var_value) const {
	if (items_.size() != r.items_.size()) {
		return false;
	}
	if (type_ != r.type_ || content_ != r.content_) {
		if ((type_ == r_constant || type_ == r_function && items_.size() == 0) && 
			r.type_ == r_variable) {
			var_value[r.content_] = content_;
		} else if ((r.type_ == r_constant || r.type_ == r_function && r.items_.size() == 0) && 
			type_ == r_variable) {
			var_value[content_] = r.content_;
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

bool Relation::replaceVariables(map<int, int> m) {
	bool replace_all_vars = true;;
	if (type_ == r_variable) {
		if (m.find(content_) != m.end()) {
			content_ = m[content_];
			type_ = r_constant;
		} else {
			replace_all_vars = false;
		}
	}
	for (int i = 0; i < items_.size(); ++i) {
		if (!items_[i].replaceVariables(m)) {
			replace_all_vars = false;
		}
	}
	return replace_all_vars;
}

bool Relation::validate(State state) {
	switch (type_) {
	case r_distinct:
		return items_[0].content_ != items_[1].content_;
		break;
	case r_and:
		return items_[0].validate(state) && items_[1].validate(state);
		break;
	case r_or:
		return items_[0].validate(state) || items_[1].validate(state);
		break;
	case r_not:
		return !items_[0].validate(state);
		break;
	case r_true:
		return items_[0].validate(state);
		break;
	default:
		pair<State::iterator, State::iterator> p = state.equal_range(*this);
		for (State::iterator i = p.first; i != p.second; ++i) {
			if (i->equals(*this)) {
				return true;
			}
		}
		return false;
	}
}

bool Relation::isLogic() {
	if (type_ == r_and ||
		type_ == r_or ||
		type_ == r_not ||
		type_ == r_distinct) {
		return true;
	} else {
		return false;
	}
}