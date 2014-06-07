#ifndef MONTECARLOPLAYER
#define MONTECARLOPLAYER

#include <vector>
#include <set>
#include <map>
#include <string>
#include<cmath>
#include "dependgraph.h"
#include "relation.h"
#include "node.h"
#include "statemachine.h"

class MonteCarloPlayer
{
public:
	StateMachine state_machine_;
	Propositions current_state_;
	bool is_terminal_;
	Node root_;
	int role_num_;
	MonteCarloPlayer(Relations rs, int rolenum);	
	Proposition stateMachineSelectMove(int timeout);
	void goOneStep(Propositions moves);
	void updateTree(Propositions state, string tree);
};

#endif