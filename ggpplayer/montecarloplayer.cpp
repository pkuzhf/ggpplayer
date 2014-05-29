#include <iostream>
#include <vector>
#include <time.h>
#include <string>
#include <cmath>
#include <stdlib.h>

#include "dependgraph.h"
#include "node.h"
#include "MonteCarloPlayer.h"

using namespace std;


MonteCarloPlayer::MonteCarloPlayer(Relations rs, int rolenum):stateMachine_(rs)
{
	roleNum_ = rolenum;
	currentState_ = stateMachine_.trues_;
}

Proposition MonteCarloPlayer::stateMachineSelectMove(int timeout)
{
	int start = clock();
	int finishBy = start + timeout;

	Node root;        

	int count = 0;
	
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

			node = &node->sons_[max][rand() % node->sons_[max].size()];		
			stateMachine_.goOneStep(node->moves_);				
		};

		int thePoint;
		if (!node->isTerminal_) {
			Propositions moves = stateMachine_.getLegalMoves(roleNum_);
			Propositions currentState = stateMachine_.trues_;
			for (int i = 0; i < moves.size(); i++) {
				vector<vector<Proposition>> jointmoves = stateMachine_.getLegalJointMoves(roleNum_, moves[i]);
				vector<Node> nodes;				
				for (int j = 0; j < jointmoves.size(); j++) {					
					stateMachine_.goOneStep(jointmoves[j]);
					nodes.push_back(Node(jointmoves[j], moves[i], node, stateMachine_.is_terminal_, stateMachine_.trues_));
					stateMachine_.setState(currentState);
				}
				node->sons_.push_back(nodes);
			}

			vector<Node> &nodes = node->sons_[rand() %  node->sons_.size()];
			node = &nodes[rand() % nodes.size()];
			stateMachine_.goOneStep(node->moves_);

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

	Proposition selection = root.sons_[max][0].mymove_;

	int stop = clock();


	cout<< "UCT simu times: " <<(double)count / (stop - start) * 1000<<endl;
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
	stateMachine_.goOneStep(moves);
	currentState_ = stateMachine_.trues_;
}