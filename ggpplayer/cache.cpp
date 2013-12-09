#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "cache.h"
#include "prover.h"
#include "tools.h"

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

bool Cache::put(State key, Data data) {
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

Data *Cache::get(State key) {
	map<State, Node*>::iterator i = map_.find(key);
	if (i != map_.end()) {
		Node *node = i->second;
		detach(node);
		attach(node);// 被读取的结点放到链表的头部
		return &(node->data_);
	} else {
		return NULL;
	}
}

bool Cache::containsKey(State key) {
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
		cout << "node " << ++n << " : ";
		// TODO：输出结点内存储的信息
	}
}