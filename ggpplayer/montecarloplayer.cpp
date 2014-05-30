#include <iostream>
#include <vector>
#include <time.h>
#include <string>
#include <cmath>
#include <stdlib.h>

#include "dependgraph.h"
#include "node.h"
#include "montecarloplayer.h"

using namespace std;


MonteCarloPlayer::MonteCarloPlayer(Relations rs, int rolenum):stateMachine_(rs)
{
	currentState_ = stateMachine_.trues_;
	is_terminal_ = false;
	for (int i = 0; i < stateMachine_.prover_.roles_.size(); ++i) {
		if (stateMachine_.prover_.roles_[i].items_[0].head_ == rolenum) {
			roleNum_ = i;
			break;
		}
	}
}

Proposition MonteCarloPlayer::stateMachineSelectMove(int timeout)
{
	stateMachine_.setState(currentState_);
	if(stateMachine_.getLegalMoves(roleNum_).size() == 1){
		return stateMachine_.getLegalMoves(roleNum_)[0];
	}
	int start = clock();
	int finishBy = start + timeout;
	Propositions legalmoves;

	Node root;  
	root.nodeState_ = currentState_;

	int count = 0;	
	legalmoves = stateMachine_.getLegalMoves(roleNum_);

	while (clock() < finishBy) {		
		stateMachine_.setState(currentState_);
		Node *node = &root;
		while (node->sons_.size() != 0) {
			int max = 0;
			double maxscore = 0;
			for (int i = 0; i < node->sons_.size(); i++) {
				vector<Node> &nodes = node->sons_[i];
				double totalScores = 0;
				for (int j = 0; j < nodes.size(); j++) {
					totalScores += nodes[j].getScore();
				}
				if (totalScores / nodes.size() > maxscore) {
					max = i;
					maxscore = totalScores / nodes.size();
				}
			}
			int tempRand = rand() % node->sons_[max].size();
			node = &node->sons_[max][tempRand];		
			if(node->sons_.size() == 0) {
				if (node->nodeState_.size() == 0){				
					stateMachine_.setState(node->parent_->nodeState_);				
					stateMachine_.goOneStep(stateMachine_.getLegalJointMoves(roleNum_, max)[tempRand]);						
					node->nodeState_ = stateMachine_.trues_;
					node->isTerminal_ = stateMachine_.is_terminal_;
				} else {
					stateMachine_.setState(node->nodeState_);
				}
			}	
		}

		int thePoint;
		if (!node->isTerminal_) {
			int move_size = stateMachine_.getLegalMoves(roleNum_).size();
			for (int i = 0; i < move_size; i++) {
				vector<vector<Proposition>> jointmoves = stateMachine_.getLegalJointMoves(roleNum_, i);
				vector<Node> nodes;				
				for (int j = 0; j < jointmoves.size(); j++) {					
					nodes.push_back(Node(node));
				}
				node->sons_.push_back(nodes);
			}
			int mymove = rand() %  node->sons_.size();
			vector<Node> &nodes = node->sons_[mymove];
			int tempRand = rand() % nodes.size();
			node = &nodes[tempRand];			
			stateMachine_.goOneStep(stateMachine_.getLegalJointMoves(roleNum_, mymove)[tempRand]);
			node->nodeState_ = stateMachine_.trues_;
			node->isTerminal_ = stateMachine_.is_terminal_;
			if (node->isTerminal_) {					
				thePoint = stateMachine_.getGoal(roleNum_);					
			} else {
				count++;
				thePoint = performDepthChargeFromMove();
			}
		} else {
			thePoint = stateMachine_.getGoal(roleNum_);
		}			

		node->totalAttemps_++;
		while (node->parent_ != NULL) {
			node->nPoints_ += thePoint;
			node->nAttemps_++;				
			node = node->parent_;
		}
	}	

	int max = 0;
	double maxscore = 0;
	for (int i = 0; i < root.sons_.size(); i++) {
		vector<Node> nodes = root.sons_[i];
		double totalScores = 0;
		for (int j = 0; j < nodes.size(); j++) {
			totalScores += nodes[j].getScore();
		}
		if (totalScores / nodes.size() > maxscore) {
			max = i;
			maxscore = totalScores / nodes.size();
		}					
	}

	Proposition selection = legalmoves[max];
	int stop = clock();


	//cout<< "UCT simu times: " <<(double)count / (stop - start) * CLOCKS_PER_SEC <<endl;
	//cout<< "UCT simu times: " <<count<<endl;
	return selection;
}

int MonteCarloPlayer::performDepthChargeFromMove()
{
	stateMachine_.randomGo();
	// wait to be changed
	return stateMachine_.getGoal(roleNum_);
}

void MonteCarloPlayer::goOneStep(Propositions moves)
{	
	stateMachine_.setState(currentState_);	
	stateMachine_.goOneStep(moves);
	currentState_ = stateMachine_.trues_;
	is_terminal_ = stateMachine_.is_terminal_;
}
