#ifndef TOOLS_H
#define TOOLS_H

#include <vector>
#include <set>
#include <map>
#include <string>

class Relation;

using namespace std;

typedef vector<Relation> Relations;
typedef multiset<Relation> State;

enum RelationType {
	r_role = 0,
	r_init = 1,
	r_next = 2,
	r_goal = 3,
	r_action = 4,
	r_true = 5,
	r_legal = 6,
	r_terminal = 7,
	r_base = 8,
	r_input = 9,
	r_does = 10,

	r_derivation = 11,
	
	//logic
	r_or = 12,
	r_and = 13,
	r_not = 14,
	r_distinct = 15,

	r_constant = 100,
	r_variable = 101,

	r_function = 102,

	r_other = 200
};


const int relation_type_num = 16;

const string relation_type_words[relation_type_num] = {
	"role",
	"init",
	"next",
	"goal",
	"action",
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

class Tools {
public:
	static set<string> stringIntersect(set<string> s1, set<string> s2);
	static vector<string> removeVectorDuplicates(vector<string> v);
};

#endif