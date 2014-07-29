#include <vector>

#include "component.h"

Component::Component(int type) {
	init(type);
}

void Component::init(int type) {
	type_ = type;
	trues_ = 0;
	if (type == c_not) {
		value_ = true;
		last_value_ = true;
	} else {
		value_ = false;
		last_value_ = false;
	}
}

Component * Component::getNotOutput() {
	for (int i = 0; i < outputs_.size(); ++i) {
		if (outputs_[i]->type_ == c_not) {
			return outputs_[i];
		}
	}
	return NULL;
}

Component * Component::getTransitionInput() {
	for (int i = 0; i < inputs_.size(); ++i) {
		if (inputs_[i]->type_ == c_transition) {
			return inputs_[i];
		}
	}
	return NULL;
}

void Component::add() {
	++trues_;
	if (type_ == c_and) {
		if (trues_ == inputs_.size()) {
			value_ = true;
		}
	} else if (type_ == c_or) {
		value_ = true;
	} else if (type_ == c_not) {
		value_ = false;
	} else if (type_ == c_transition) {
		value_ = true;
	}
	if (type_ != c_transition) {
		propagate();
	}
}

void Component::minus() {
	--trues_;
	if (type_ == c_and) {
		value_ = false;
	} else if (type_ == c_or) {
		if (trues_ == 0) {
			value_ = false;
		}
	} else if (type_ == c_not) {
		value_ = true;
	} else if (type_ == c_transition) {
		value_ = false;
	}
	if (type_ != c_transition) {
		propagate();
	}
}

void Component::propagate() {
	if (value_ && !last_value_) {
		for (int i = 0; i < outputs_.size(); ++i) {
			outputs_[i]->add();
		}
	} else if (!value_ && last_value_) {
		for (int i = 0; i < outputs_.size(); ++i) {
			outputs_[i]->minus();
		}
	}
	last_value_ = value_;
}

void Component::addOutput(Component * c) {
	outputs_.push_back(c);
	c->inputs_.push_back(this);
	if (c->type_ == c_and && c->value_) {
		c->value_ = false;
		c->propagate();
	}
	if (type_ != c_transition && value_) {
		c->add();
	}
}