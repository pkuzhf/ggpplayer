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
	map<int, string> legalActions_;	// ÿ����ҿ��ܵ��߷�
	map<string, string> nextState_;	// ÿ��move(��ÿ�����)->nextState
	int terminal_;					// �Ƿ����0��֪����1������-1û����
	Data();
};

struct Node {
	string key_;
	Data data_;
	Node *prev_, *next_;
};


/*
 * cache��map+˫������
 * �ܵĴ洢�����ǹ̶���
 * ����LRU�㷨�����滻
 */
class Cache{
private:						
	map<string, Node*> map_;		// ����ӳ��ı�
    vector<Node*> free_entries_;	// �洢���ý��ĵ�ַ
    Node *head_, *tail_;			// ͷβ�����й���
    Node *entries_;					// ˫�������еĽ��
	
public:
    Cache(int size);
    ~Cache();
    bool put(string key, Data data);
    Data *get(string key);
	bool containsKey(string key);
	bool isEmpty();
	void show();// debugʹ��

private:
    // ������
    void detach(Node* node);
    // ��������ͷ��
    void attach(Node* node);
};

#endif