#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <queue>
#include <map>
#include <algorithm>
#include <string>

#include "dependgraph.h"
#include "relation.h"
#include "reader.h"

using namespace std;


void DependGraph::buildGraph(vector<Derivation> derivations)
{
	vector<vector<int> > edges_in_; // 入边的另一个顶点的num
	vector<vector<int> > edges_out_; // 出边的另一个顶点的num
	vector<bool> MARK;

	addNode(r_legal, edges_in_, edges_out_, MARK);
	addNode(r_does, edges_in_, edges_out_, MARK);
	addNode(r_goal, edges_in_, edges_out_, MARK);
	addNode(r_terminal, edges_in_, edges_out_, MARK);
	addNode(r_next, edges_in_, edges_out_, MARK);
	addNode(r_base, edges_in_, edges_out_, MARK);
	addNode(r_input, edges_in_, edges_out_, MARK);
	addNode(r_init, edges_in_, edges_out_, MARK);

	derivations_ = derivations;
	for(int i = 0 ; i < derivations_.size(); ++i){
		if (node_num_.find(derivations_[i].target_.head_) == node_num_.end()) {
			addNode(derivations_[i].target_.head_, edges_in_, edges_out_, MARK);
		}
		for(int j = 0; j < derivations_[i].subgoals_.size(); ++j){			
			Proposition r = derivations_[i].subgoals_[j];
			if (r.head_ == r_distinct) {
				continue;
			}
			if (r.head_ == r_not) {
				r = r.items_[0];			
			}
			if (node_num_.find(r.head_) == node_num_.end()) {
				addNode(r.head_, edges_in_, edges_out_, MARK);
			}
			addEdge(derivations_[i].target_.head_, r.head_, edges_in_, edges_out_);
		}
	}	
	if(node_num_.find(r_legal) != node_num_.end() && node_num_.find(r_does) != node_num_.end())
		addEdge(r_does, r_legal, edges_in_, edges_out_);
	if(node_num_.find(r_terminal) != node_num_.end() && node_num_.find(r_does) != node_num_.end())
		addEdge(r_does, r_terminal, edges_in_, edges_out_);
	if(node_num_.find(r_goal) != node_num_.end() && node_num_.find(r_does) != node_num_.end())
		addEdge(r_does, r_goal, edges_in_, edges_out_);
	if(node_num_.find(r_legal) != node_num_.end() && node_num_.find(r_terminal) != node_num_.end())
		addEdge(r_terminal, r_legal, edges_in_, edges_out_);
	for(int i = 0 ; i < nodes_.size(); ++i){
		init_nodes_.push_back(nodes_[i][0]);
	}
	init_edges_out_ = edges_out_;
	topoSort(edges_in_, edges_out_, MARK);
	
	node_stra_.resize(node_num_.size());
	for (int i = 0; i < topo_graph_.size(); ++i) {
		for (int j = 0; j < topo_graph_[i].size(); ++j) {
			node_stra_[topo_graph_[i][j]] = i;
		}
	}
	legal_level_ = -1;
	if(node_num_.find(r_legal) != node_num_.end())
		legal_level_ = node_stra_[node_num_[r_legal]];
	if(node_num_.find(r_terminal) != node_num_.end())
		legal_level_ = max(legal_level_, node_stra_[node_num_[r_terminal]]);
	if(node_num_.find(r_goal) != node_num_.end())
		legal_level_ = max(legal_level_, node_stra_[node_num_[r_goal]]);
	getStraDeriv();
}

void DependGraph::getStraDeriv()
{
	for(int i = 0 ; i < topo_graph_.size(); ++i){
		stra_deriv_.push_back(vector<int>());
	}

	for(int i = 0 ; i < derivations_.size(); ++i){
		stra_deriv_[node_stra_[node_num_[derivations_[i].target_.head_]]].push_back(i);
	}
}

void DependGraph::topoSort(vector<vector<int> > & edges_in_, vector<vector<int> >& edges_out_, vector<bool> & MARK)
{
	while(true){
		int zero_in_index = findZeroIn(edges_in_, edges_out_, MARK);
		if(zero_in_index == -1) break;
		if(zero_in_index == -2){
			deleteLoop(edges_in_, edges_out_, MARK);
			continue;
		}
		MARK[zero_in_index] = true;
		for(int i = 0 ; i < edges_out_[zero_in_index].size(); ++i){
			vector<int>::iterator iter = find(edges_in_[edges_out_[zero_in_index][i]].begin(),edges_in_[edges_out_[zero_in_index][i]].end(), zero_in_index);
			edges_in_[edges_out_[zero_in_index][i]].erase(iter);
		}
		edges_out_[zero_in_index].resize(0);
		topo_graph_.push_back(vector<int>());
		for(int i = 0 ; i < nodes_[zero_in_index].size(); ++i){
			(topo_graph_.end() - 1)->push_back(node_num_[nodes_[zero_in_index][i]]);
		}
	}
}

void DependGraph::deleteLoop(vector<vector<int> > & edges_in_, vector<vector<int> >& edges_out_, vector<bool> & MARK)
{
	int index = 0; 
	for(int i = 0 ; i < node_num_.size(); ++i){
		if(!MARK[i] && edges_out_[i].size() > 0){
			index = i;
			break;
		}
	}
	vector<int> dis;
	for(int i = 0 ; i < node_num_.size(); ++i){
		dis.push_back(index);
	}
	queue<int> q;
	q.push(index);
	while(!q.empty()){
		int now = q.front();
		q.pop();
		vector<int>::iterator iter = find(edges_out_[now].begin(), edges_out_[now].end(), index);
		if(iter != edges_out_[now].end()){ //环路已经找到
			vector<int> tempVector;
			while(now != index){
				tempVector.push_back(now);
				MARK[now] = true;
				now = dis[now];
			}
			tempVector.push_back(index);
			for(int i = 0 ; i < tempVector.size(); ++i){
				if(tempVector[i] == index) continue;
				for(int j = 0; j < edges_out_[tempVector[i] ].size(); ++j){
					if(find(tempVector.begin(), tempVector.end(), edges_out_[tempVector[i]][j]) == tempVector.end()){
						vector<int>::iterator iter = find(edges_in_[edges_out_[tempVector[i] ][j]].begin(), edges_in_[edges_out_[tempVector[i] ][j]].end(), tempVector[i] );
						*iter = index;
						edges_out_[index].push_back(edges_out_[tempVector[i] ][j]);
					}						
				}
				for(int j = 0; j < edges_in_[tempVector[i] ].size(); ++j){
					if(find(tempVector.begin(), tempVector.end(), edges_in_[tempVector[i] ][j]) == tempVector.end()){
						vector<int>::iterator iter = find(edges_out_[edges_in_[tempVector[i] ][j]].begin(), edges_out_[edges_in_[tempVector[i] ][j]].end(), tempVector[i] );
						*iter = index;
					
						edges_in_[index].push_back(edges_in_[tempVector[i] ][j]);
					}
				}
				edges_in_[tempVector[i] ].resize(0);
				edges_out_[tempVector[i] ].resize(0);
				for(int j = 0; j < nodes_[tempVector[i] ].size(); ++j){
					nodes_[index].push_back(nodes_[tempVector[i] ][j]);
				}
				for(vector<int>::iterator j = edges_in_[index].begin() ; j != edges_in_[index].end(); ++j){
					if(*j == tempVector[i]){
						j = edges_in_[index].erase(j);
					}
					if(j == edges_in_[index].end())
						break;
					
				}
				for(vector<int>::iterator j = edges_out_[index].begin() ; j != edges_out_[index].end(); ++j){
					if(*j == tempVector[i]){
						 j = edges_out_[index].erase(j);
					}
					if(j == edges_out_[index].end())
						break;
				}
			}
			
			return;
		} else {
			for(int i = 0; i < edges_out_[now].size(); ++i){
				dis[edges_out_[now][i]] = now;
				q.push(edges_out_[now][i]);
			}
		}
	}
}

int DependGraph::findZeroIn(vector<vector<int> > & edges_in_, vector<vector<int> >& edges_out_, vector<bool> & MARK)
{
	bool b = false;
	for(int i = 0 ;i < node_num_.size(); ++i){
		if(MARK[i] == false){
			b = true;
			if(edges_in_[i].size() == 0){
				//cerr<< nodes_[i][0]<<endl;
				return i;
			}
		}
	}
	 if(!b) return -1; // sort done
	 return -2;   // there are loops
}

void DependGraph::addNode(int node, vector<vector<int> > & edges_in_, vector<vector<int> >& edges_out_, vector<bool> & MARK) {
	node_num_[node] = nodes_.size();
	nodes_.push_back(vector<int>());
	nodes_.at(nodes_.size() - 1).push_back(node);
	edges_in_.push_back(vector<int>());
	edges_out_.push_back(vector<int>());
	MARK.push_back(false);
}

void DependGraph::addEdge(int head, int tail,vector<vector<int> > & edges_in_, vector<vector<int> >& edges_out_)
{
	if (head == tail) 
		return;        // delete single loop

	if (find(edges_in_[node_num_[head]].begin(), edges_in_[node_num_[head]].end(), node_num_[tail]) == edges_in_[node_num_[head]].end() ){
		edges_in_[node_num_[head]].push_back(node_num_[tail]);
		edges_out_[node_num_[tail]].push_back(node_num_[head]);
	}
}