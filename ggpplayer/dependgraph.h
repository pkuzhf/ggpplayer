#ifndef DEPENDGRAPH_H
#define DEPENDGRAPH_H

#include <vector>
#include <set>
#include <map>
#include <string>

#include "tools.h"

using namespace std;

class DependGraph {
public:
	vector<vector<string>> nodes_;      // num to strings
	map<string, int> node_num_; // string to num

	vector<vector<int>> edges_in_; // ��ߵ���һ�������num
	vector<vector<int>> edges_out_; // ���ߵ���һ�������num
	
	vector<vector<int>> topo_graph_;  // �����ĺ���Ĺ�ϵͼ��vector��ÿ��Ԫ����һ�㣬�����ǻ��ε�������ϵ��
	vector<int> node_stra_;

	Relations derivations_; // ������򼯺�
	vector<vector<int>> stra_deriv_; // �Ѿ��ֲ�Ĺ������vector�е�i��Ԫ�ر�ʾ���������������Ϊi������������

	void buildGraph(Relations derivations);


private:
	void buildGraphBySingleRelation(Relation & r);
	void addNode(string node);
	void addEdge(string head, string tail);
	void topoSort();
	int findZeroIn();
	void deleteLoop();
	vector<bool> MARK;
	void getStraDeriv();
	int findMaxCondition(Relation & r);
	int findMaxSubRelation(Relation & r);
};

#endif