#ifndef RELATION_H
#define RELATION_H

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "tools.h"

using namespace std;

class Relation {
public:
	string content_;
	RelationType type_;
	Relations items_;

	bool matches(const Relation &r, map<string, string> &var_value) const;
	bool equals(const Relation &r) const;
	map<string, string> getVarValue(const Relation &r) const;
	set<string> findVariables();
	Relations findProposions();
	bool operator<(const Relation &r) const;
	string toString() const;
	bool replaceVariables(map<string, string> m);
	bool validate(State state);
	bool isLogic();
};

#endif