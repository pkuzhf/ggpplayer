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
	int finish_by = start + timeout;
	Propositions legalmoves;

	int count = 0;	
	legalmoves = state_machine_.getLegalMoves(role_num_);

	while (clock() < finish_by) {		
		state_machine_.setState(current_state_);
		Node *node = &root_;
		while (node->sons_.size() != 0) {
			int max = 0;
			double maxscore = 0;
			for (int i = 0; i < node->sons_.size(); i++) {
				vector<Node> &nodes = node->sons_[i];
				double total_score = 0;
				for (int j = 0; j < nodes.size(); j++) {
					total_score += nodes[j].getScore();
				}
				if (total_score / nodes.size() > maxscore) {
					max = i;
					maxscore = total_score / nodes.size();
				}
			}
			int temp_rand = rand() % node->sons_[max].size();
			node = &node->sons_[max][temp_rand];		
			if(node->sons_.size() == 0) {
				if (node->state_.size() == 0){				
					state_machine_.setState(node->parent_->state_);				
					state_machine_.goOneStep(state_machine_.getLegalJointMoves(role_num_, max)[temp_rand]);						
					node->state_ = state_machine_.trues_;
					node->is_terminal_ = state_machine_.is_terminal_;
				} else {
					state_machine_.setState(node->state_);
				}
			}	
		}

		int point = -1;
		if (!node->is_terminal_) {
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
			int temp_rand = rand() % nodes.size();
			node = &nodes[temp_rand];			
			state_machine_.goOneStep(state_machine_.getLegalJointMoves(role_num_, mymove)[temp_rand]);
			node->state_ = state_machine_.trues_;
			node->is_terminal_ = state_machine_.is_terminal_;
			if (node->is_terminal_) {					
				point = state_machine_.getGoal(role_num_);					
			} else {
				count++;
				if (state_machine_.randomGo(finish_by)) {
					point = state_machine_.getGoal(role_num_);
				}				
			}
		} else {
			point = state_machine_.getGoal(role_num_);
		}			
		if (point != -1) {			
			do {
				node->points_ += point;
				node->attemps_++;				
				node = node->parent_;
			} while (node != NULL);
		}
	}	

	int max = 0;
	double maxscore = 0;
	for (int i = 0; i < root.sons_.size(); i++) {
		vector<Node> nodes = root.sons_[i];
		double total_score = 0;
		for (int j = 0; j < nodes.size(); j++) {
			total_score += nodes[j].getScore();
		}
		if (total_score / nodes.size() > maxscore) {
			max = i;
			maxscore = total_score / nodes.size();
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
