#ifndef COMPONENT_H
#define COMPONENT_H

#include <vector>

const int c_and = 0;
const int c_or = 1;
const int c_not = 2;
const int c_transition = 3;

class Component {
public:
	int type_;
	vector<Component *> inputs_;
	vector<Component *> outputs_;
	int value_;
	int trues_;

	Component(int type, );
};

#endif