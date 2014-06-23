#ifndef MONTECARLOPLAYER_H
#define MONTECARLOPLAYER_H

#include <vector>
#include <set>
#include <map>
#include <string>
#include <cmath>
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
	Propositions legal_moves_;
	int role_num_;
	map<string, Node *> state_node_;

	MonteCarloPlayer();
	MonteCarloPlayer(Relations rs, string role);	
	Proposition stateMachineSelectMove(int timeout);
	double uct(int time_limit, int once_simu_limit, int max_simu_times);
	Node * selectLeafNode();
	void goOneStep(Propositions moves);
	void setState(Propositions state);
	void updateTree(Propositions state, string tree);
	Proposition getRandomMove();
	Proposition getBestMove();
};

#endif