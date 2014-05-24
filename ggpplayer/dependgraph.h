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
	vector<vector<int> > init_edges_in_; // 入边的另一个顶点的num
	vector<vector<int> > init_edges_out_; // 出边的另一个顶点的num
	
	vector<vector<int> > topo_graph_;  // 拓扑拍好序的关系图，vector中每个元素是一层，层内是环形的依赖关系网
	vector<int> node_stra_;

	vector<Derivation> derivations_; // 推理规则集合
	vector<vector<int> > stra_deriv_; // 已经分层的规则，外层vector中第i个元素表示所有最大子语句层数为i的推理规则序号

	void buildGraph(vector<Derivation> derivations);
	int legal_level_;

private:
	void buildGraphBySingleRelation(Relation & r);
	void addNode(int node);
	void addEdge(int head, int tail);
	void topoSort();
	int findZeroIn();
	void deleteLoop();
	vector<bool> MARK;
	void getStraDeriv();
	int findMaxCondition(Relation & r);
	int findMaxSubRelation(Relation & r);
	vector<vector<int> > edges_in_; // 入边的另一个顶点的num
	vector<vector<int> > edges_out_; // 出边的另一个顶点的num
};

#endif