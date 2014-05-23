#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <string.h>

#include "reader.h"
#include "relation.h"

using namespace std;

bool Reader::scanGDLFile(string filename) {
	ifstream f(filename.c_str());
	if (!f) {
		return false;
	}
	while (!f.eof()) {
		char buffer[100000];
		f.getline(buffer, 100000);
		int i = 0;
		while (i < strlen(buffer) && buffer[i] != ';'){
			file_content_ += buffer[i];
			++i;
		}
		file_content_ += " ";		
	}
	f.close();
	return true;
}

bool Reader::getRelations(Relations &relations) {
	int idx = 0;
	string substring;
	while (fetch(file_content_, idx, substring)) {
		if (substring[0] != '(') {
			return false;
		}
		Relation r = getRelation(substring);
		if (r.type_ == r_derivation) {
			Relations rs = eliminateLogicalWords(r);
			for (int i = 0; i < rs.size(); ++i) {
				sortDerivationItems(rs[i]);
			}
			relations.insert(relations.end(), rs.begin(), rs.end());
		} else {
			relations.push_back(r);
		}
	}
	return true;
}

bool Reader::getMoves(Relations &relations) {
	int idx = 0;
	string substring;
	while (fetch(file_content_, idx, substring)) {
		Relation r = getRelation(substring);
		relations.push_back(r);		
	}
	return true;
}

Relation Reader::getRelation(const string &s, RelationType default_type) {	
	int idx = 0;
	if (s[idx] == '(') {
		idx = 1;
	}
	Relation r;
	string head;
	fetch(s, idx, head);
	r.type_ = getType(head);
	if(Relation::string2int_.find(head) == Relation::string2int_.end()){
		Relation::string2int_[head] = Relation::int2string_.size();
		Relation::int2string_.push_back(head);
	}
	r.content_ = Relation::string2int_[head];
	if (r.type_ == r_other) {
		r.type_ = default_type;
	}

	string substring;	
	switch(r.type_) {
	case r_role:
		fetch(s, idx, substring);
		r.items_.push_back(getRelation(substring, r_constant));		
		break;
	case r_init:			
	case r_next:
	case r_true:
	case r_base:
	case r_not:
		fetch(s, idx, substring);
		r.items_.push_back(getRelation(substring, r_function));		
		break;
	case r_legal:
	case r_input:
	case r_does:
		fetch(s, idx, substring);
		r.items_.push_back(getRelation(substring, r_constant));		
		fetch(s, idx, substring);
		r.items_.push_back(getRelation(substring, r_function));					
		break;
	case r_goal:
	case r_distinct:
		fetch(s, idx, substring);
		r.items_.push_back(getRelation(substring, r_constant));		
		fetch(s, idx, substring);
		r.items_.push_back(getRelation(substring, r_constant));
		break;		
	case r_and:
	case r_or:
		while (fetch(s, idx, substring)) {				
			r.items_.push_back(getRelation(substring, r_function));
		}
		break;
	case r_derivation:
		fetch(s, idx, substring);
		r.items_.push_back(getRelation(substring, r_function));
		while (fetch(s, idx, substring)) {				
			r.items_.push_back(getRelation(substring, r_function));
		}
		break;
	case r_function:
		while (fetch(s, idx, substring)) {				
			r.items_.push_back(getRelation(substring, r_constant));
		}
		break;
	case r_terminal:
	case r_constant:
	case r_variable:
		break;	
	};
	return r;
}

bool Reader::fetch(const string &s, int &start, string &result) {
	while (start < s.size() && s[start] <= ' ') { // s maybe contain ascii 9...
		++start;
	}
	if (start >= s.size() || s[start] == ')') {
		return false;
	}
	if (s[start] == '(') {
		int count = 0;
		int i = start;
		while (i < s.size()) {
			if (s[i] == '(') {
				++count;
			} else if (s[i] == ')') {
				--count;
			}
			if (count == 0) {
				result = s.substr(start, i - start + 1);
				start = i + 1;
				return true;
			} else {
				++i;
			}
		}
		return false;
	} else {
		int i = start;
		while (i < s.size() && s[i] != ' ' && s[i] != '(' && s[i] != ')' && s[i] != '\r' && s[i] != '\n') {
			++i;
		}
		result = s.substr(start, i - start);
		start = i;
		return true;
	}
}

RelationType Reader::getType(const string &s) {
	for (int i = 0; i < relation_type_num; ++i) {
		if (s == relation_type_words[i]) {
			return (RelationType)i;
		}
	}
	if (s[0] == '?') {
		return r_variable;
	}
	return r_other;
}

Relations Reader::eliminateLogicalWords(Relation r) {	
	Relations ret;
	Relations items;
	bool changed = false;
	for (int i = 0; i < r.items_.size(); ++i) {
		if (r.items_[i].type_ == r_not) {
			if (r.items_[i].items_[0].type_ == r_and
				|| r.items_[i].items_[0].type_ == r_or) {
				changed = true;
				Relation item = r.items_[i].items_[0];
				if (item.type_ == r_and) {
					item.type_ = r_or;
					item.content_ = Relation::string2int_[relation_type_words[r_or]];
				} else {
					item.type_ = r_and;
					item.content_ = Relation::string2int_[relation_type_words[r_and]];
				}				
				for (int j = 0; j < item.items_.size(); ++j) {
					if (item.items_[j].type_ == r_not) {
						item.items_[j] = item.items_[j].items_[0];
					} else {
						Relation not_relation;
						not_relation.type_ = r_not;
						not_relation.content_ = Relation::string2int_[relation_type_words[r_not]];
						not_relation.items_.push_back(item.items_[j]);
						item.items_[j] = not_relation;
					}
				}
				r.items_[i] = item;
			}
		}
	}
	for (int i = 0; i < r.items_.size(); ++i) {
		if (r.items_[i].type_ == r_and) {
			changed = true;
			for (int j = 0; j < r.items_[i].items_.size(); ++j) {
				items.push_back(r.items_[i].items_[j]);
			}
		} else {
			items.push_back(r.items_[i]);
		}
	}
	r.items_ = items;
	for (int i = 0; i < r.items_.size(); ++i) {
		if (r.items_[i].type_ == r_or) {
			changed = true;
			for (int j = 0; j < r.items_[i].items_.size(); ++j) {
				ret.push_back(r);
				ret[j].items_[i] = r.items_[i].items_[j];				
			}
			break;
		}
	}
	if (ret.size() == 0) {
		ret.push_back(r);
	}
	if (changed) {
		Relations tmp = ret;
		ret.clear();
		for (int i = 0; i < tmp.size(); ++i) {
			Relations result = eliminateLogicalWords(tmp[i]);
			ret.insert(ret.end(), result.begin(), result.end());
		}
	}
	return ret;
}

inline int getOrder(RelationType rt) {
	if (rt == r_not) {
		return 3;
	} else if (rt == r_distinct) {
		return 2;
	} else {
		return 1;
	}
}

void Reader::sortDerivationItems(Relation & r) {
	for (int i = 1; i < r.items_.size(); ++i) {
		bool changed = false;
		for (int j = i + 1; j < r.items_.size(); ++j) {
			if (getOrder(r.items_[j - 1].type_) > getOrder(r.items_[j].type_)) {
				Relation tmp = r.items_[j - 1];
				r.items_[j - 1] = r.items_[j];
				r.items_[j] = tmp;
				changed = true;
			}
		}
		if (!changed) {
			break;
		}
	}
}
