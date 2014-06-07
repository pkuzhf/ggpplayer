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
	void readLine(char *buffer);
	bool readFile(string filename);
	bool getRelations(Relations &relations);
	void getPropositions(Propositions &propositions);
	bool getState(Propositions &propositions);
	static Relation getRelation(const string &s);
	string file_content_;
private:	
	static bool fetch(const string &s, int &start, string &result);	
	Relations eliminateLogicalWords(Relation r);
	void sortDerivationItems(Relation &r);
};

#endif