#ifndef RELATION_H
#define RELATION_H

#define MAX_ITEM_NUM 20

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <string>

using namespace std;

enum RelationType {
	r_role = 0,
	r_init = 1,
	r_next = 2,
	r_goal = 3,	
	r_true = 4,
	r_legal = 5,
	r_terminal = 6,
	r_base = 7,
	r_input = 8,
	r_does = 9,

	r_derivation = 10,
	
	//logic
	r_or = 11,
	r_and = 12,
	r_not = 13,
	r_distinct = 14,

	r_constant = 100,
	r_variable = 101,	

	r_function = 103,

	r_other = 200
};


const int relation_type_num = 15;

const string relation_type_words[relation_type_num] = {
	"role",
	"init",
	"next",
	"goal",	
	"true",
	"legal",
	"terminal",
	"base",
	"input",
	"does",
	"<=",
	"or",
	"and",
	"not",
	"distinct"
};

class Relation;
class Proposition;
class Derivation;

typedef vector<Relation> Relations;
typedef vector<Proposition> Propositions;
typedef vector<Derivation> Derivations;

class Relation {
public:
	string s_;
	int content_;
	RelationType type_;
	Relations items_;
	
	static unordered_map<string, int> string2int_;
	static vector<string> int2string_;
	static int symbol_table_size_;

	static void addSymbol(string symbol);

	bool matches(const Relation &r, vector<pair<int, int> > &var_value) const;
	bool equals(const Relation &r) const;	
	bool operator<(const Relation &r) const;
	string toString() const;
	bool replaceVariables(vector<pair<int, int> > &m);		
	Proposition toProposition();
	Derivation toDerivation();
};

class Proposition {
public:

	static Proposition strToProp(string s);

	int head_;
	bool is_variable_;
	vector<Proposition> items_;

	bool operator<(const Proposition &r) const;
	Relation toRelation();
	bool matches(Proposition p, vector<int> &variables, vector<int> &values);
	bool equals(Proposition p);		
	void getVariables(vector<int> &variables);
	void replaceVariables(vector<int> &variables, vector<int> &values);
	int getPropCode();
	string toString();
};

class Derivation {
public:
	Proposition target_;
	Propositions subgoals_;
	vector<int> variables_;
	Relation toRelation();
	void prepareVariables();
};

#endif