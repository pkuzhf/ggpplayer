#ifndef CACHE_H
#define CACHE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "prover.h"
#include "relation.h"

using namespace std;


class Data {
public:
	map<int, string> legalActions_;	// 每个玩家可能的走法
	map<string, string> nextState_;	// 每组move(对每个玩家)->nextState
	int terminal_;					// 是否结束0不知道，1结束，-1没结束
	Data();
};

struct Node {
	string key_;
	Data data_;
	Node *prev_, *next_;
};


/*
 * cache：map+双向链表
 * 总的存储个数是固定的
 * 采用LRU算法进行替换
 */
class Cache{
private:						
	map<string, Node*> map_;		// 整个映射的表
    vector<Node*> free_entries_;	// 存储可用结点的地址
    Node *head_, *tail_;			// 头尾结点进行管理
    Node *entries_;					// 双向链表中的结点
	
public:
    Cache(int size);
    ~Cache();
    bool put(string key, Data data);
    Data *get(string key);
	bool containsKey(string key);
	bool isEmpty();
	void show();// debug使用

private:
    // 分离结点
    void detach(Node* node);
    // 将结点插入头部
    void attach(Node* node);
};

#endif