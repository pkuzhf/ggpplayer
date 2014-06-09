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

const int r_role = 0;
const int r_init = 1;
const int r_next = 2;
const int r_goal = 3;	
const int r_true = 4;
const int r_legal = 5;
const int r_terminal = 6;
const int r_base = 7;
const int r_input = 8;
const int r_does = 9;

const int r_derivation = 10;
	
	//logic
const int r_or = 11;
const int r_and = 12;
const int r_not = 13;
const int r_distinct = 14;

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

	static unordered_map<string, int> symbol2code;
	static vector<string> code2symbol; // code2symbol may contain some strings of proposition other than symbols
	static int symbol_table_size;

	static void addSymbol(string symbol);

	string s_;
	int head_;	
	Relations items_;

	bool operator<(const Relation &r) const;
	string toString() const;
	bool replaceVariables(vector<pair<int, int> > &m);		
	Proposition toProposition();
	Derivation toDerivation();
};

class Proposition {
public:

	static Proposition strToProp(string s);
	static string propsToStr(Propositions ps);
	int head_;
	bool is_variable_;
	vector<Proposition> items_;

	bool operator<(const Proposition &p) const;
	bool operator==(const Proposition &p) const;
	Relation toRelation();
	bool matches(Proposition &p, vector<int> &variables, vector<int> &values);
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

struct hash_Proposition {
	enum   
	{   //   parameters   for   hash   table   
		bucket_size   =   4,   //   0   <   bucket_size   
		min_buckets   =   8  //   min_buckets   =   2   ^^   N,   0   <   N   
	};   
	size_t operator()(const class Proposition & a) const {	
		int prime = 10000001;
		int sum = a.head_;
		for (int i = 0; i < a.items_.size(); ++i) {
			sum = (sum * (operator()(a.items_[i]) << i)) % prime;
		}
		return sum;
	}
	bool operator()(const class Proposition & a1, const class Proposition & a2) const {
		return  a1 < a2;
	}
};

//struct hash_Propositions {
//	enum   
//	{   //   parameters   for   hash   table   
//		bucket_size   =   4,   //   0   <   bucket_size   
//		min_buckets   =   8  //   min_buckets   =   2   ^^   N,   0   <   N   
//	};   
//	size_t operator()(const class vector<Proposition> & a) const {
//		int ret = 0;
//		int prime = 10000001;
//		hash_Proposition hash_p;
//		for (int i = 0; i < a.size(); ++i) {
//			ret = (ret + (hash_p.operator()(a[i]) << i)) % prime;
//		}
//		return ret;
//	}
//	bool operator()(const class vector<Proposition> & a1, const class vector<Proposition> & a2) const {
//		if (a1.size() < a2.size()) {
//			return true;
//		} else if (a1.size() > a2.size()) {
//			return false;
//		}
//		for (int i = 0; i < a1.size(); ++i) {
//			if (a1[i] < a2[i]) {
//				return true;
//			} else if (a2[i] < a1[i]) {
//				return false;
//			}
//		}
//		return  false;
//	}
//};

#endif