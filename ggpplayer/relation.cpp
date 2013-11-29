#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "relation.h"
#include "tools.h"

using namespace std;


bool Relation::matches(const Relation &r, map<string, string> &var_value) const {
	if (items_.size() != r.items_.size()) {
		return false;
	}
	if (type_ != r.type_ || content_ != r.content_) {
		if ((type_ == RelationType::r_constant || type_ == RelationType::r_function && items_.size() == 0) && 
			r.type_ == RelationType::r_variable) {
			var_value[r.content_] = content_;
		} else if ((r.type_ == RelationType::r_constant || r.type_ == RelationType::r_function && r.items_.size() == 0) && 
			type_ == RelationType::r_variable) {
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

map<string, string> Relation::getVarValue(const Relation &r) const {
//this and r should be matched...
	map<string, string> p;
	if (type_ == RelationType::r_variable && r.type_ != RelationType::r_variable) {
		p[content_] = r.content_;
	} else if (r.type_ == RelationType::r_variable && type_ != RelationType::r_variable) {
		p[r.content_] = content_;
	}
	for (int i = 0; i < items_.size(); ++i) {
		map<string, string> p1 = items_[i].getVarValue(r.items_[i]);
		for (map<string, string>::iterator j = p1.begin(); j != p1.end(); ++j) {
			if (p.find(j->first) == p.end()) {
				p[j->first] = j->second;
			} else {
				p.erase(j->first);
			}
		}
	}
	return p;
}

set<string> Relation::findVariables() {
	set<string> vs;
	if (type_ == RelationType::r_variable) {
		vs.insert(content_);
	}
	for (int i = 0; i < items_.size(); ++i) {
		set<string> vs1 = items_[i].findVariables();
		for (set<string>::iterator j = vs1.begin(); j != vs1.end(); ++j) {
			if (vs.find(*j) == vs.end()) {
				vs.insert(*j);
			}
		}
	}
	return vs;
}

Relations Relation::findProposions() {
	Relations rs;
	if (type_ == RelationType::r_function) {
		rs.push_back(*this);
	} else {
		for (int i = 0; i < items_.size(); i++) {
			Relations pros = items_[i].findProposions();
			rs.insert(rs.end(), pros.begin(), pros.end());
		}
	}
	return rs;
}

bool Relation::operator<(const Relation &r) const{
	return content_ < r.content_;
}

string Relation::toString() const {
	string s = content_;
	for (int i = 0; i < items_.size(); ++i) {
		if (items_[i].items_.size() > 0) {
			s += " ("+ items_[i].toString() + ")";
		} else {
			s += " "+ items_[i].toString();
		}
	}
if (content_ == "does") {
	int a = 5;
}
	return s;
}

bool Relation::replaceVariables(map<string, string> m) {
	bool replace_all_vars = true;;
	if (type_ == RelationType::r_variable) {
		if (m.find(content_) != m.end()) {
			content_ = m[content_];
			type_ = RelationType::r_constant;
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
	case RelationType::r_distinct:
		return items_[0].content_ != items_[1].content_;
		break;
	case RelationType::r_and:
		return items_[0].validate(state) && items_[1].validate(state);
		break;
	case RelationType::r_or:
		return items_[0].validate(state) || items_[1].validate(state);
		break;
	case RelationType::r_not:
		return !items_[0].validate(state);
		break;
	case RelationType::r_true:
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
	if (type_ == RelationType::r_and ||
		type_ == RelationType::r_or ||
		type_ == RelationType::r_not ||
		type_ == RelationType::r_distinct) {
		return true;
	} else {
		return false;
	}
}