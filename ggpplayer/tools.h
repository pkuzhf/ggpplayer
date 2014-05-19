#ifndef TOOLS_H
#define TOOLS_H

#include <vector>
#include <set>
#include <map>
#include <string>

class Relation;

using namespace std;



class Tools {
public:
	static set<string> stringIntersect(set<string> s1, set<string> s2);
	static vector<string> removeVectorDuplicates(vector<string> v);
	static Relations removeRelationVectorDuplicates(Relations v);
};

#endif