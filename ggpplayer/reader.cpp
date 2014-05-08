#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "reader.h"
#include "tools.h"
#include "relation.h"

using namespace std;

bool Reader::scanGDLFile(string filename) {
	ifstream f(filename);
	if (!f) {
		return false;
	}
	while (!f.eof()) {
		char buffer[1000];
		f.getline(buffer, 1000);
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
		if (r.type_ == RelationType::r_derivation) {
			Relations rs = eliminateLogicalWords(r);
			relations.insert(relations.end(), rs.begin(), rs.end());
		} else {
			relations.push_back(r);
		}
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
	r.content_ = head;
	r.type_ = getType(head);
	if (r.type_ == RelationType::r_other) {
		r.type_ = default_type;
	}

	string substring;	
	switch(r.type_) {
	case RelationType::r_role:
		fetch(s, idx, substring);
		r.items_.push_back(getRelation(substring, RelationType::r_constant));		
		break;
	case RelationType::r_init:			
	case RelationType::r_next:
	case RelationType::r_true:
	case RelationType::r_base:
	case RelationType::r_not:
		fetch(s, idx, substring);
		r.items_.push_back(getRelation(substring, RelationType::r_function));		
		break;
	case RelationType::r_legal:
	case RelationType::r_input:
	case RelationType::r_does:
		fetch(s, idx, substring);
		r.items_.push_back(getRelation(substring, RelationType::r_constant));		
		fetch(s, idx, substring);
		r.items_.push_back(getRelation(substring, RelationType::r_function));					
		break;
	case RelationType::r_goal:
	case RelationType::r_distinct:
		fetch(s, idx, substring);
		r.items_.push_back(getRelation(substring, RelationType::r_constant));		
		fetch(s, idx, substring);
		r.items_.push_back(getRelation(substring, RelationType::r_constant));
		break;		
	case RelationType::r_and:
	case RelationType::r_or:
		while (fetch(s, idx, substring)) {				
			r.items_.push_back(getRelation(substring, RelationType::r_function));
		}
		break;
	case RelationType::r_derivation:
		fetch(s, idx, substring);
		r.items_.push_back(getRelation(substring, RelationType::r_function));
		while (fetch(s, idx, substring)) {				
			r.items_.push_back(getRelation(substring, RelationType::r_function));
		}
		break;
	case RelationType::r_function:
		while (fetch(s, idx, substring)) {				
			r.items_.push_back(getRelation(substring, RelationType::r_constant));
		}
		break;
	case RelationType::r_terminal:
	case RelationType::r_constant:
	case RelationType::r_variable:
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
		while (i < s.size() && s[i] != ' ' && s[i] != '(' && s[i] != ')') {
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
		return RelationType::r_variable;
	}
	return RelationType::r_other;
}

Relations Reader::eliminateLogicalWords(Relation r) {
	Relations ret;
	Relations items;
	for (int i = 1; i < r.items_.size(); ++i) {
		if (r.items_[i].type_ == RelationType::r_and) {
			for (int j = 0; j < r.items_[i].items_.size(); ++j) {
				items.push_back(r.items_[i].items_[j]);
			}
		} else {
			items.push_back(r.items_[i]);
		}
	}
}

Relations 