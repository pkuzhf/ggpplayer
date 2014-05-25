#ifndef READER_H
#define READER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "relation.h"

using namespace std;


class Reader {
public:
	bool scanGDLFile(string filename);
	bool getRelations(Relations &relations);
	bool getMoves(Propositions &propositions);
	static Relation getRelation(const string &s, RelationType fathertype = r_function);
	string file_content_;
private:	
	static bool fetch(const string &s, int &start, string &result);
	static RelationType getType(const string &s);
	Relations eliminateLogicalWords(Relation r);
	void sortDerivationItems(Relation &r);
};

#endif