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

void MonteCarloPlayer::updateTree(Propositions state, string tree) {

}

MonteCarloPlayer::MonteCarloPlayer(Relations rs, int rolenum):state_machine_(rs)
{
	current_state_ = state_machine_.trues_;
	is_terminal_ = false;
	for (int i = 0; i < state_machine_.prover_.roles_.size(); ++i) {
		if (state_machine_.prover_.roles_[i].items_[0].head_ == rolenum) {
			role_num_ = i;
			break;
		}
	}
}

Proposition MonteCarloPlayer::stateMachineSelectMove(int timeout)
{
	//state_machine_.setState(current_state_);
	if(state_machine_.getLegalMoves(role_num_).size() == 1){
		return state_machine_.getLegalMoves(role_num_)[0];
	}
	int start = clock();
	int finishBy = start + timeout;
	Propositions legalmoves;

	Node root;
	root.nodeState_ = current_state_;

	int count = 0;	
	legalmoves = state_machine_.getLegalMoves(role_num_);

	while (clock() < finishBy) {		
		state_machine_.setState(current_state_);
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
					state_machine_.setState(node->parent_->nodeState_);				
					state_machine_.goOneStep(state_machine_.getLegalJointMoves(role_num_, max)[tempRand]);						
					node->nodeState_ = state_machine_.trues_;
					node->isTerminal_ = state_machine_.is_terminal_;
				} else {
					state_machine_.setState(node->nodeState_);
				}
			}	
		}

		int thePoint = -1;
		if (!node->isTerminal_) {
			int move_size = state_machine_.getLegalMoves(role_num_).size();
			for (int i = 0; i < move_size; i++) {
				vector<vector<Proposition>> jointmoves = state_machine_.getLegalJointMoves(role_num_, i);
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
			state_machine_.goOneStep(state_machine_.getLegalJointMoves(role_num_, mymove)[tempRand]);
			node->nodeState_ = state_machine_.trues_;
			node->isTerminal_ = state_machine_.is_terminal_;
			if (node->isTerminal_) {					
				thePoint = state_machine_.getGoal(role_num_);					
			} else {
				count++;
				if (state_machine_.randomGo(finishBy)) {
					thePoint = state_machine_.getGoal(role_num_);
				}				
			}
		} else {
			thePoint = state_machine_.getGoal(role_num_);
		}			
		if (thePoint != -1) {
			node->totalAttemps_++;
			while (node->parent_ != NULL) {
				node->nPoints_ += thePoint;
				node->nAttemps_++;				
				node = node->parent_;
			}
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

void MonteCarloPlayer::goOneStep(Propositions moves)
{	
	state_machine_.setState(current_state_);	
	state_machine_.goOneStep(moves);
	current_state_ = state_machine_.trues_;
	is_terminal_ = state_machine_.is_terminal_;
}
