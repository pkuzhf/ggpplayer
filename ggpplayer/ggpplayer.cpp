#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "reader.h"
#include "relation.h"
#include "prover.h"
#include "domaingraph.h"
#include "tools.h"

using namespace std;

int main() {
	Reader r;
	r.scanGDLFile("C:\\Dropbox\\GGP\\ggpplayer\\ggpplayer\\gdl\\tic_tac_toe.txt");
	Relations rs;
	r.getRelations(rs);
	State initstate;
	Prover p(rs);
	p.getInitState(initstate);
	State nextstate;
	p.getNextState(initstate, nextstate);
	return 0;
}
