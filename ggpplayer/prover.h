#ifndef PROVER_H
#define PROVER_H

#include <vector>
#include <set>
#include <map>
#include <string>

#include "tools.h"

using namespace std;

class Prover {
public:
	Prover(Relations relations);
	bool getInitState(State &initstate);
	bool getNextState(const State &currentstate, State &nextstate);
	Relations generateNewRelations(const State &currentstate, Relation & deviration);
	Relations getLegalActions(State &state, string &role);
private:
	Relations relations_;
	Relations derivations_;

	
};

#endif