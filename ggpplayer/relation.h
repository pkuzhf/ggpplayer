#ifndef RELATION_H
#define RELATION_H

#define MAX_ITEM_NUM 50
#define MAX_SUBGOAL_NUM 50
#define MAX_VARIABLE_NUM 50

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <hash_set>
#include <map>
#include<hash_map>
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

typedef vector<Relation> Relations;
typedef multiset<Relation> State;

class Proposition;
class Derivation;

class Relation {
public:
	string s_;
	int content_;
	RelationType type_;
	Relations items_;
	
	static hash_map<string, int> string2int_;
	static vector<string> int2string_;
	
	bool matches(const Relation &r, vector<pair<int, int> > &var_value) const;
	bool equals(const Relation &r) const;
	Relations findProposions();
	bool operator<(const Relation &r) const;
	string toString() const;
	bool replaceVariables(vector<pair<int, int> > &m);		
	Proposition toProposition();
	Derivation toDerivation();
};

class Proposition {
public:
	short int items_[MAX_ITEM_NUM];
	short int item_num_;
	Relation toRelation();
	bool match(Proposition p, int *variables, int *values, int &len);
	void remove
	//void replaceVariables(int *variables, int *values, int len);
private:
	void shiftBy(int n);
};

class Derivation {
public:
	Proposition target_;
	Proposition subgoals_[MAX_SUBGOAL_NUM];
	short int subgoal_num_;
	short int variables_[MAX_VARIABLE_NUM];
	short int variable_num_;
	Relation toRelation();
	void prepareVariables();
};

#endif