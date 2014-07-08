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
	unordered_map<Node *, vector<Node *>> map_state_path_;

	MonteCarloPlayer();
	MonteCarloPlayer(Relations rs, string role);
	Proposition stateMachineSelectMove(int timeout);
	double uct(clock_t time_limit, clock_t once_simu_limit, int max_simu_times);
	Node * selectLeafNode(vector<Node *> &path);
	Node * selectLeafNodeServer();
	void goOneStep(Propositions moves);
	void setState(Propositions state);
	void updateTree(Propositions state, string tree);
	Proposition getRandomMove();
	Proposition getBestMove();
	Node * newNode(Node * parent = NULL);
	pair<int, int> updateNode(Node * node, string s);
	void initNode(Node * node, Propositions & state, bool is_terminal);
	void initNode(Node * node, string & state, bool is_terminal);
	void deleteNodes();
	void getAncients(Node * node, unordered_set<Node *> &ancients);
	void updateValidNumber(Node * node, int number);
	void expandeNode(Node * node);
};

#endif