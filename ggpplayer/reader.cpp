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
		Relation r;
		if (getRelation(substring, r, RelationType::r_other)) {
			relations.push_back(r);
		}
	}
	return true;
}

bool Reader::getRelation(const string &s, Relation &r, RelationType fathertype) {
	string head;
	int idx = 0;
	if (s[idx] == '(') {
		idx = 1;
	}
	if (fetch(s, idx, head)) {
		if (head[0] == '(') {
			return false;
		}
		r.content_ = head;
		r.type_ = getType(head);
		string substring;
		while (fetch(s, idx, substring)) {
			Relation r1;
			if (getRelation(substring, r1, r.type_)) {
				r.items_.push_back(r1);
			}
		}
		if (r.type_ == RelationType::r_function && (fathertype == RelationType::r_function 
													|| fathertype == RelationType::r_distinct
													|| fathertype == RelationType::r_role)) {
			r.type_ = RelationType::r_constant;
		}
	}
	return true;
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
	return RelationType::r_function;
}
