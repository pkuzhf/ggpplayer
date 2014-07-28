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

Component * Component::getNotOutput() {
	for (int i = 0; i < outputs_.size(); ++i) {
		if (outputs_[i]->type_ == c_not) {
			return outputs_[i];
		}
	}
	return NULL;
}