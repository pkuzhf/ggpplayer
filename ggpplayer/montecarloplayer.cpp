#include "MonteCarloPlayer.h"
#include <vector>
#include <set>
#include <map>
#include <string>
#include<cmath>
#include "dependgraph.h"
#include "node.h"
#include <time.h>

Relation MonteCarloPlayer::stateMachineSelectMove(int timeout)
{
	int start = clock();
	int finishBy = start + timeout - 1000;

	Node root;        

	int count = 0;
	
	while (clock() < finishBy) {

		Node node = root;
		while (node.sons_.size() != 0) {
			int max = 0;
			double maxscore = 0;
			for (int i = 0; i < node.sons_.size(); i++) {
				vector<Node> nodes = node.sons_[i];
				double totalScores = 0;
				for (int j = 0; j < nodes.size(); j++) {
					totalScores += nodes[j].getScore();
				}
				if (totalScores / nodes.size() > maxscore) {
					max = i;
					maxscore = totalScores / nodes.size();
				}
			}

			node = node.sons_[max][rand() * node.sons_[max].size()];		
			stateMachine_.goOneStep(node.moves_);				
		};

		int thePoint;
		if (!node.isTerminal_) {
			Relations moves = stateMachine_.getLegalMoves(stateMachine_.getRole());
			Relations currentState = stateMachine_.current_state_;
			for (int i = 0; i < moves.size(); i++) {
				vector<vector<Relation>> jointmoves = stateMachine_.getLegalJointMoves(getRole(), moves[i]);
				vector<Node> nodes;				
				for (int j = 0; j < jointmoves.size(); j++) {					
					stateMachine_.goOneStep(jointmoves[j]);
					nodes.push_back(Node(jointmoves[j], moves[i], &node, stateMachine_.isTerminal, currentState));
					stateMachine_.setState(currentState);
				}
				node.sons_.push_back(nodes);
			}

			vector<Node> nodes = node.sons_[rand() * node.sons_.size()];
			node = nodes[rand() * nodes.size()];
			stateMachine_.goOneStep(node.moves_);

			if (node.isTerminal_) {					
				thePoint = stateMachine_.getGoal(getRole());					
			} else {
				count++;
				thePoint = performDepthChargeFromMove();
			}
		} else {
			thePoint = stateMachine_.getGoal(getRole());
		}			

		node.totalAttemps_++;
		while (node.parent_ != &node) {
			node.nPoints_ += thePoint;
			node.nAttemps_++;				
			node = *node.parent_;
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

	Relation selection = root.sons_[max][0].mymove_;

	int stop = clock();


	cout<< "UCT simu times: " <<(double)count / (stop - start) * 1000<<endl;
	return selection;
}

int MonteCarloPlayer::performDepthChargeFromMove()
{
	Relations finalGoals = stateMachine_.randomGo();
	// wait to be changed
	return 0;
}