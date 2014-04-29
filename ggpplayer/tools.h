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

class Tools {
public:
	static set<string> stringIntersect(set<string> s1, set<string> s2);
	static vector<string> removeVectorDuplicates(vector<string> v);
	static Relations removeRelationVectorDuplicates(Relations v);
};

#endif