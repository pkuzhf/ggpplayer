#ifndef COMPONENT_H
#define COMPONENT_H

#include <vector>

#include "component.h"

using namespace std;

const int c_uninit = -1;
const int c_and = 0;
const int c_or = 1;
const int c_not = 2;
const int c_transition = 3;

class Component {
public:
	int type_;
	bool value_;
	bool last_value_;
	int trues_;
	vector<Component *> inputs_;
	vector<Component *> outputs_;
	
	Component(int type);
	void init(int type);
	Component * getNotOutput();
	Component * getTransitionInput();
	void add();
	void minus();
	void propagate();
	void addOutput(Component * c);
};

#endif