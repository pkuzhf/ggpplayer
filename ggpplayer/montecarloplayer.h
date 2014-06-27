#ifndef MONTECARLOPLAYER_H
#define MONTECARLOPLAYER_H

#include <vector>
#include <set>
#include <unordered_map>
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
	Node * root_;
	Propositions legal_moves_;
	int role_;
	unordered_map<string, Node *> map_state_node_;

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
	Node * newNode(Node * parent = NULL);
	void updateNode(Node * node, string s);
	void initNode(Node * node, Propositions & state, bool is_terminal);
	void deleteNodes();
	void updateParents(Node * node, long long points, long long attemps);
	void getAncients(Node * node, unordered_set<Node *> &ancients);
	void updateValidNumber(Node * node, int number);
};

#endif