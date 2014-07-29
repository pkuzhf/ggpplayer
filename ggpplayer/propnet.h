#ifndef PROPNET_H
#define PROPNET_H

#include <vector>

#include "component.h"
#include "relation.h"

class Propnet {
public:
	vector<Component *> components_;
	vector<Component *> transitions_;
	vector<Component *> inputs_;
	Propositions ps_;
	map<int, vector<int> > map_head_ps_;
	map<string, int> map_string_p_;

	void init(Relations rs);
	void setState(Propositions state);
	void setMove(Propositions move);
	Propositions get(int head);
	Propositions getState();
	Propositions getLegals();
	Propositions getGoals();
	bool isTerminal();
	void printTrues();
};

#endif