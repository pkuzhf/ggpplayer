#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "cache.h"
#include "prover.h"
#include "relation.h"

using namespace std;

Data::Data() {
	terminal_ = 0;
}

Cache::Cache(int size) {
	entries_ = new Node[size];
	for (int i = 0; i < size; ++i) {				// 存储可用结点的地址
		free_entries_.push_back(entries_ + i);
	}
	head_ = new Node;
	tail_ = new Node;
	head_->prev_ = NULL;
	head_->next_ = tail_;
	tail_->prev_ = head_;
	tail_->next_ = NULL;
}

Cache::~Cache() {
	delete head_;
	delete tail_;
	delete[] entries_;
}

bool Cache::put(string key, Data data) {
	Node *node;
	if (free_entries_.empty()) {// 可用结点为空，即cache已满,删除
		node = tail_->prev_;// 替换链表的最后一个结点
		detach(node);
		map_.erase(node->key_);
	} else {
		node = free_entries_.back();
		free_entries_.pop_back();
	}
	node->key_ = key;
	node->data_ = data;
	map_[key] = node;
	attach(node);
	return true;
}

Data *Cache::get(string key) {
	map<string, Node*>::iterator i = map_.find(key);
	if (i != map_.end()) {
		Node *node = i->second;
		detach(node);
		attach(node);// 被读取的结点放到链表的头部
		return &(node->data_);
	} else {
		return NULL;
	}
}

bool Cache::containsKey(string key) {
	return map_[key] != NULL;
}

bool Cache::isEmpty() {
	return map_.empty();
}


void Cache::detach(Node* node) {
    node->prev_->next_ = node->next_;
    node->next_->prev_ = node->prev_;
}

void Cache::attach(Node* node) {
    node->prev_ = head_;
    node->next_ = head_->next_;
    head_->next_ = node;
    node->next_->prev_ = node;
}

void Cache::show() {
	int n = 0;
	for (Node *i = head_->next_; i != tail_; i = i->next_) {
		cout << "[node " << ++n << " ]:";
		cout << "	key: " << i->key_ << endl;
		cout << "	legalActions:(role(int), moves(string))" << endl;
		for(map<int, string>::const_iterator iter = i->data_.legalActions_.begin(); iter != i->data_.legalActions_.end(); ++iter)
			cout << "		(" << iter->first << ", " << iter->second << ")" << endl;
		cout << "	nextState:(moves(string), state(string))" << endl;
		for(map<string, string>::const_iterator iter = i->data_.nextState_.begin(); iter != i->data_.nextState_.end(); ++iter)
			cout << "		(" << iter->first << ", " << iter->second << ")" << endl;
		cout << "	terminal:";
		if (i->data_.terminal_ == 0)
			cout << "Can't know" << endl;
		else if (i->data_.terminal_ == 1)
			cout << "terminal" << endl;
		else
			cout << "continue" << endl;
	}
}