#ifndef MONTECARLOPLAYER_H
#define MONTECARLOPLAYER_H

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
	Propositions legal_moves_;
	int role_num_;
	map<Propositions, Node *> state_node_;

	MonteCarloPlayer(Relations rs, int rolenum);	
	Proposition stateMachineSelectMove(int timeout);
	double uct(int finish_by);
	int getBestMoveOfNode(Node * node);
	Node * selectExpandingNode();
	void goOneStep(Propositions moves);
	void updateTree(Propositions state, string tree);
	Proposition getRandomMove();
	Proposition getBestMove();
};

#endif