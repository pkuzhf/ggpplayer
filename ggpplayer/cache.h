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

typedef string State;
typedef Relation Role;
typedef Relations Roles;
typedef string Move;
typedef string Moves;// Ӧ����������ʽ�ɣ�
typedef Relations Goals;

class Data {
public:
	vector<int> goals_;				// ÿ����ҵĵ÷�..����˳������
	map<Role, Moves> legalActions_;	// ÿ����ҿ��ܵ��߷�
	map<Moves, State> nextState_;	// ÿ��move(��ÿ�����)->nextState
	int terminal_;					// �Ƿ����0��֪����-1������1û����
	Data();
};

struct Node {
	State key_;
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
	map<State, Node*> map_;			// ����ӳ��ı�
    vector<Node*> free_entries_;	// �洢���ý��ĵ�ַ
    Node *head_, *tail_;			// ͷβ�����й���
    Node *entries_;					// ˫�������еĽ��
	
public:
    Cache(int size);
    ~Cache();
    bool put(State key, Data data);
    Data *get(State key);
	bool containsKey(State key);
	bool isEmpty();
	void show();// debugʹ��

private:
    // ������
    void detach(Node* node);
    // ��������ͷ��
    void attach(Node* node);
};

#endif