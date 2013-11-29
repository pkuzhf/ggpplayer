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
	static bool getRelation(const string &s, Relation &r, RelationType fathertype);
private:
	string file_content_;
	static bool fetch(const string &s, int &start, string &result);
	static RelationType getType(const string &s);
};

#endif