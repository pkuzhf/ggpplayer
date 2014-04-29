#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <queue>
#include <map>
#include <algorithm>
#include <string>

#include "dependgraph.h"
#include "tools.h"
#include "relation.h"
#include "reader.h"

using namespace std;


void DependGraph::buildGraph(Relations rs)
{
	//input
	addNode(relation_type_words[RelationType::r_role]);
	addNode(relation_type_words[RelationType::r_does]);
	addNode(relation_type_words[RelationType::r_true]);
	//output
	addNode(relation_type_words[RelationType::r_next]);
	addNode(relation_type_words[RelationType::r_goal]);
	addNode(relation_type_words[RelationType::r_legal]);
	addNode(relation_type_words[RelationType::r_terminal]);		

	for(int i = 0 ; i < rs.size(); ++i){
		buildGraphBySingleRelation(rs[i]);
		if (rs[i].type_ == RelationType::r_derivation) {
			derivations_.push_back(rs[i]);
		}
	}

	topoSort();

}

void DependGraph::topoSort()
{
	while(true){
		int zero_in_index = findZeroIn();
		if(zero_in_index == -1) break;
		if(zero_in_index == -2){
			deleteLoop();
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

void DependGraph::deleteLoop()
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

int DependGraph::findZeroIn()
{
	bool b = false;
	for(int i = 0 ;i < node_num_.size(); ++i){
		if(MARK[i] == false){
			b = true;
			if(edges_in_[i].size() == 0){
				cout<< nodes_[i][0]<<endl;
				return i;
			}
		}
	}
	 if(!b) return -1; // sort done
	 return -2;   // there are loops
}

void DependGraph::addNode(string node) {
	node_num_[node] = nodes_.size();
	nodes_.push_back(vector<string>());
	nodes_.at(nodes_.size() - 1).push_back(node);
	edges_in_.push_back(vector<int>());
	edges_out_.push_back(vector<int>());
	MARK.push_back(false);
}

void DependGraph::buildGraphBySingleRelation(Relation & r)
{	
	if (r.type_ == RelationType::r_function && node_num_.find(r.content_) != node_num_.end()) {
		addNode(r.content_);
	}
	for (int i = 0; i < r.items_.size(); ++i) {
		buildGraphBySingleRelation(r.items_[i]);			
	}
	//add edges
	
	if(r.type_ == RelationType::r_derivation){
		for(int i = 1; i < r.items_.size(); ++i){
			addEdge(r.items_[0], r.items_[i]);
		}
	}
}

void DependGraph::addEdge(Relation & head, Relation & tail)
{
	if(tail.isLogic()){
		for(int i = 0 ; i < tail.items_.size(); ++i){
			addEdge(head, tail.items_[i]);
		}
	} else if(node_num_.find(head.content_) != node_num_.end() && node_num_.find(tail.content_) != node_num_.end()){
		edges_in_[node_num_[head.content_]].push_back(node_num_[tail.content_]);
		edges_out_[node_num_[tail.content_]].push_back(node_num_[head.content_]);
	}
}