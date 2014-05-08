#ifndef READER_H
#define READER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "tools.h"

using namespace std;


class Reader {
public:
	bool scanGDLFile(string filename);
	bool getRelations(Relations &relations);
	static Relation getRelation(const string &s, RelationType fathertype = RelationType::r_function);
private:
	string file_content_;
	static bool fetch(const string &s, int &start, string &result);
	static RelationType getType(const string &s);
	Relations eliminateLogicalWords(Relation r);
};

#endif