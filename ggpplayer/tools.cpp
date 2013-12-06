#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "tools.h"

using namespace std;

set<string> Tools::stringIntersect(set<string> s1, set<string> s2) {
	set<string> result;
	set<string>::iterator i = s1.begin();
	set<string>::iterator j = s2.begin();
	while (i != s1.end() && j != s2.end()) {
		if (*i == *j) {
			result.insert(*i);
			++i;
			++j;
		} else if (*i < *j) {
			++i;
		} else {
			++j;
		}
	}
	return result;
}

vector<string> Tools::removeVectorDuplicates(vector<string> v) {
	set<string> s;
	vector<string> res;
	for (int i = 0; i < v.size(); ++i) {
		s.insert(v[i]);
	}
	for (set<string>::iterator i = s.begin(); i != s.end(); ++i) {
		res.push_back(*i);
	}
	return res;
}
/*
Relations Tools::removeRelationVectorDuplicates(Relations v) {
	set<Relation> s;
	Relations res;
	for (int i = 0; i < v.size(); ++i) {
		s.insert(v[i]);
	}
	for (set<Relation>::iterator i = s.begin(); i != s.end(); ++i) {
		res.push_back(*i);
	}
	return res;
}
*/
