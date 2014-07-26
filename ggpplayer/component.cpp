#include <vector>

#include "component.h"

Component::Component() {
	type_ = c_uninit;
	value_ = false;
	trues_ = 0;
}

Component::Component(int type) {
	type_ = type;
	value_ = false;
	trues_ = 0;
}