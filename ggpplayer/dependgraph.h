#ifndef DEPENDGRAPH_H
#define DEPENDGRAPH_H

#include <vector>
#include <set>
#include <map>
#include <string>

#include "relation.h"

using namespace std;

class DependGraph {
public:
	vector<int> init_nodes_;
	vector<vector<int> > nodes_;      
	map<int, int> node_num_; // string to num

	vector<vector<int> > init_edges_out_; // ���ߵ���һ�������num
	
	vector<vector<int> > topo_graph_;  // �����ĺ���Ĺ�ϵͼ��vector��ÿ��Ԫ����һ�㣬�����ǻ��ε�������ϵ��
	vector<int> node_stra_;

	vector<Derivation> derivations_; // ������򼯺�
	vector<vector<int> > stra_deriv_; // �Ѿ��ֲ�Ĺ������vector�е�i��Ԫ�ر�ʾ���������������Ϊi������������
	int legal_level_;
	
	void buildGraph(vector<Derivation> derivations);
private:
	void buildGraphBySingleRelation(Relation & r);
	void addNode(int node, vector<vector<int> > &edges_in_, vector<vector<int> >& edges_out_, vector<bool> & MARK);
	void addEdge(int head, int tail, vector<vector<int> >& edges_in_, vector<vector<int> >& edges_out_);
	void topoSort(vector<vector<int> > &edges_in_, vector<vector<int> >& edges_out_, vector<bool> & MARK);
	int findZeroIn(vector<vector<int> > &edges_in_, vector<vector<int> >& edges_out_, vector<bool> & MARK);
	void deleteLoop(vector<vector<int> > &edges_in_, vector<vector<int> >& edges_out_, vector<bool> & MARK);
	void getStraDeriv();
	int findMaxCondition(Relation & r);
	int findMaxSubRelation(Relation & r);
};

#endif