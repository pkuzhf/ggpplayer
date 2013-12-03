#ifndef PROVER_H
#define PROVER_H

#include <vector>
#include <set>
#include <map>
#include <string>
#include "domaingraph.h"
#include "tools.h"

using namespace std;

class Prover {
public:
	Prover(Relations relations);
	bool getInitState(State &initstate);
	bool getNextState(const State &currentstate, State &nextstate);
	Relations generateNewRelations(const State &currentstate, Relation & deviration);

	bool askRole(Relations &rs);
	bool askGoal(Relations &rs,const State & state);
	bool askTerminal(const State & state);
	bool askLegalActions(Relations &rs,Relation role,const State state );
	bool askLegalActions(Relations &rs,const State state );
	bool askNextState(State &nextstate,const State state,const Relations does);
	

private:
	Relations relations_;
	Relations derivations_;
	DomainGraph dg_;
	bool validateInstance(string instance, set<string> &true_instances, set<string> &false_instances, set<string> &validating_instances);
	void findVarDomainInSingleInstance(Relation r, map<string, set<string>> &var_values);
	string buildNode(string s, int i);

};

#endif