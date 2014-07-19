#ifndef PROPNET_H
#define PROPNET_H

#include <vector>

#include "component.h"
#include "relation.h"

class Propnet {
public:
	vector<Component *> components_;
	
	void init(Relations ps);
};

#endif