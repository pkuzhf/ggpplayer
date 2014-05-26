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
	StateMachine stateMachine_;
	Propositions currentState_;
	Node TreeRoot_;
	int roleNum_;
	int performDepthChargeFromMove();
	Proposition stateMachineSelectMove(int timeout);
};

#endif