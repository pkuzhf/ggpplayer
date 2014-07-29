#include <vector>
#include <algorithm>

#include "propnet.h"
#include "component.h"
#include "relation.h" 
#include "prover.h"
#include "combination.h"

using namespace std;

void Propnet::init(Relations rs) {
	for (int i = 0; i < rs.size(); ++i) {
		rs[i].s_ = rs[i].toString();
	}

	Prover p(rs);
	Propositions bases = p.bases_;
	Propositions inputs = p.inputs_;

	Relations rs_remove_not = rs;
	for (int i = 0; i < rs_remove_not.size(); ++i) {
		if (rs_remove_not[i].head_ == r_derivation) {
			for (vector<Relation>::iterator j = rs_remove_not[i].items_.begin() + 1; j != rs_remove_not[i].items_.end(); ) {
				if (j->head_ == r_not) {
					j = rs_remove_not[i].items_.erase(j);
				} else {
					++j;
				}
			}
		}
	}
	
	Prover p2(rs_remove_not);
	for (int i = 0; i < bases.size(); ++i) {
		bases[i].head_ = r_true;
	}
	for (int i = 0; i < inputs.size(); ++i) {
		inputs[i].head_ = r_does;
	}
	ps_.insert(ps_.end(), bases.begin(), bases.end());
	ps_.insert(ps_.end(), inputs.begin(), inputs.end());
	p2.generateTrueProps(ps_, 0, p2.dpg_.stra_deriv_.size() - 1);
	ps_.insert(ps_.end(), p2.statics_.begin(), p2.statics_.end());
	for (int i = 0; i < ps_.size(); ++i) {
		Component * c = new Component(c_or);
		components_.push_back(c);
		if (ps_[i].head_ == r_does) {
			inputs_.push_back(c);
		}
	}

	
	for (int i = 0; i < ps_.size(); ++i) {
		map_head_ps_[ps_[i].head_].push_back(i);
		map_string_p_[ps_[i].toString()] = i;
	}

	for (int i = 0; i < p.statics_.size(); ++i) {
		Component *c = components_[map_string_p_[p.statics_[i].toString()]];
		c->value_ = true;
		c->last_value_ = true;
	}

	vector<string> scans;
	for (int i = 0; i < ps_.size(); ++i) {
		scans.push_back(ps_[i].toString());
	}
	
	for (int i = 0; i < map_head_ps_[r_next].size(); ++i) {
		Proposition next_p = ps_[map_head_ps_[r_next][i]];
		Proposition true_p = next_p;
		true_p.head_ = r_true;
		Component *n = components_[map_string_p_[next_p.toString()]];
		Component *t = components_[map_string_p_[true_p.toString()]];
		Component *transition = new Component(c_transition);
		components_.push_back(transition);
		transitions_.push_back(transition);
		n->addOutput(transition);
		transition->addOutput(t);
	}

	Propositions inits = p.inits_;
	for (int i = 0; i < inits.size(); ++i) {
		Proposition p = inits[i];
		p.head_ = r_next;
		if (map_string_p_.find(p.toString()) != map_string_p_.end()) {
			components_[map_string_p_[p.toString()]]->outputs_[0]->last_value_ = true;
		}
		p.head_ = r_true;
		components_[map_string_p_[p.toString()]]->value_ = true;
		components_[map_string_p_[p.toString()]]->last_value_ = true;
		components_[map_string_p_[p.toString()]]->trues_ = 1;
	}

	for (int i = 0; i < rs.size(); ++i) {
		if (rs[i].head_ == r_derivation) {
			Derivation d = rs[i].toDerivation();
			d.prepareVariables();
			vector<int> distincts;
			for (int j = 0; j < d.subgoals_.size(); ++j) {
				if (d.subgoals_[j].head_ == r_distinct) {
					distincts.push_back(j);
				}
			}
			Proposition &target = d.target_;
			if (target.head_ == r_next) {
//				cout << "1" << endl;
			}
			for (int j = 0; j < map_head_ps_[target.head_].size(); ++j) {
				vector<int> target_variables;
				vector<int> target_values;
				if (target.matches(ps_[map_head_ps_[target.head_][j]], target_variables, target_values)) {
					vector<int> undetermined_vars;
					for (int k = 0; k < d.variables_.size(); ++k) {
						if (find(target_variables.begin(), target_variables.end(), d.variables_[k]) == target_variables.end()) {
							undetermined_vars.push_back(d.variables_[k]);
						}
					}
					vector<pair<int, int> > constant_distincts;
					vector<pair<int, int> > variable_distincts;
					bool distinct_fail = false;
					for (int k = 0; k < distincts.size(); ++k) {
						Proposition distinct = d.subgoals_[distincts[k]];
						distinct.replaceVariables(target_variables, target_values);
						int v0 = distinct.items_[0].head_;
						int v1 = distinct.items_[1].head_;	
						if (distinct.items_[0].is_variable_ && distinct.items_[1].is_variable_) {
							int p0 = find(undetermined_vars.begin(), undetermined_vars.end(), v0) - undetermined_vars.begin();
							int p1 = find(undetermined_vars.begin(), undetermined_vars.end(), v1) - undetermined_vars.begin();
							variable_distincts.push_back(pair<int, int>(p0, p1));
						} else if (distinct.items_[0].is_variable_ && !distinct.items_[1].is_variable_) {
							int p0 = find(undetermined_vars.begin(), undetermined_vars.end(), v0) - undetermined_vars.begin();
							int p1 = v1;
							constant_distincts.push_back(pair<int, int>(p0, p1));
						} else if (!distinct.items_[0].is_variable_ && distinct.items_[1].is_variable_) {
							int p0 = find(undetermined_vars.begin(), undetermined_vars.end(), v1) - undetermined_vars.begin();
							int p1 = v0;
							constant_distincts.push_back(pair<int, int>(p0, p1));
						} else if (!distinct.items_[0].is_variable_ && !distinct.items_[1].is_variable_) {
							if (distinct.items_[0].head_ == distinct.items_[1].head_) {
								distinct_fail = true;
							}
						}
					}
					if (distinct_fail) {
						continue;
					}
					vector<vector<vector<int> > > multiple_combinations(1, vector<vector<int> >(1, vector<int>(undetermined_vars.size(), -1)));
					for (int k = 0; k < d.subgoals_.size(); ++k) {
						Proposition subgoal = d.subgoals_[k];
						if (subgoal.head_ == r_distinct || subgoal.head_ == r_not) {
							continue;
						}
						subgoal.replaceVariables(target_variables, target_values);						
						vector<vector<int> > combinations;
						for (int ii = 0; ii < map_head_ps_[subgoal.head_].size(); ++ii) {
							vector<int> subgoal_variables;
							vector<int> subgoal_values;
							if (subgoal.matches(ps_[map_head_ps_[subgoal.head_][ii]], subgoal_variables, subgoal_values)) {
								vector<int> combination = getCombination(undetermined_vars, subgoal_variables, subgoal_values);
								combinations.push_back(combination);
							}
						}
						multiple_combinations.push_back(combinations);
					}
					vector<vector<int> > combinations = mergeMultipleCombinations(multiple_combinations, variable_distincts, constant_distincts);
					if (combinations.size() > 0) {
						Component *tmp_or = new Component(c_or);
						components_.push_back(tmp_or);
						tmp_or->addOutput(components_[map_head_ps_[target.head_][j]]);
						for (int k = 0; k < combinations.size(); ++k) {
							Component *tmp_and = new Component(c_and);
							components_.push_back(tmp_and);
							tmp_and->addOutput(tmp_or);
							for (int ii = 0; ii < d.subgoals_.size(); ++ii) {
								if (d.subgoals_[ii].head_ == r_distinct) {
									continue;
								} else {
									Proposition subgoal = d.subgoals_[ii];
									subgoal.replaceVariables(target_variables, target_values);
									subgoal.replaceVariables(undetermined_vars, combinations[k]);
									if (subgoal.head_ == r_not) {
										Component *c;
										if (map_string_p_.find(subgoal.items_[0].toString()) == map_string_p_.end()) {
											c = new Component(c_or);
											components_.push_back(c);
										} else {
											c = components_[map_string_p_[subgoal.items_[0].toString()]];
										}
										Component *tmp_not = c->getNotOutput();
										if (tmp_not == NULL){
											tmp_not = new Component(c_not);
											components_.push_back(tmp_not);
											c->addOutput(tmp_not);
										}
										tmp_not->addOutput(tmp_and);
									} else {
										//cout << subgoal.toString() << endl;
										Component *c = components_[map_string_p_[subgoal.toString()]];
										c->addOutput(tmp_and);
										//if (components_[map_head_ps_[r_legal][0]]->value_) {
										//	cout << ps_[map_head_ps_[r_legal][0]].toString() << endl;
										//}
										//if (components_[map_head_ps_[r_legal][1]]->value_) {
										//	cout << ps_[map_head_ps_[r_legal][1]].toString() << endl;
										//}
									}
								}
							}
						}
					}
				}
			}
			//cout << rs[i].toString() << endl;
			//cout << components_.size() << endl;
		}
	}

	printTrues();

}

void Propnet::setState(Propositions state) {
	for (int i = 0; i < map_head_ps_[r_true].size(); ++i) {
		Component * c = components_[map_head_ps_[r_true][i]];
		c->value_ = false;
		c->trues_ = 0;
		Component * t = c->getTransitionInput();
		if (t) {
			t->last_value_ = false;
		}
	}
	for (int i = 0; i < state.size(); ++i) {
		Component * c = components_[map_string_p_[state[i].toString()]];
		c->value_ = true;
		c->trues_ = 1;
		Component * t = c->getTransitionInput();
		if (t) {
			t->last_value_ = true;
		}
	}
	for (int i = 0; i < map_head_ps_[r_true].size(); ++i) {
		components_[map_head_ps_[r_true][i]]->propagate();
	}
}

void Propnet::setMove(Propositions move) {
	for (int i = 0; i < move.size(); ++i) {
		components_[map_string_p_[move[i].toString()]]->value_ = true;
		components_[map_string_p_[move[i].toString()]]->propagate();
	}
	vector<int> add;
	vector<int> minus;
	for (int i = 0; i < transitions_.size(); ++i) {
		if (transitions_[i]->value_ && !transitions_[i]->last_value_) {
			add.push_back(i);
		}
		if (!transitions_[i]->value_ && transitions_[i]->last_value_) {
			minus.push_back(i);
		}
		transitions_[i]->last_value_ = transitions_[i]->value_;
	}
	for (int i = 0; i < add.size(); ++i) {
		Component * t = transitions_[add[i]];
		for (int i = 0; i < t->outputs_.size(); ++i) {
			t->outputs_[i]->add();
		}
	}
	for (int i = 0; i < minus.size(); ++i) {
		Component * t = transitions_[minus[i]];
		for (int i = 0; i < t->outputs_.size(); ++i) {
			t->outputs_[i]->minus();
		}
	}
	for (int i = 0; i < move.size(); ++i) {
		components_[map_string_p_[move[i].toString()]]->value_ = false;
		components_[map_string_p_[move[i].toString()]]->propagate();
	}
}

Propositions Propnet::get(int head) {
	Propositions ps;
	for (int i = 0; i < map_head_ps_[head].size(); ++i) {
		if (components_[map_head_ps_[head][i]]->value_) {
			ps.push_back(ps_[map_head_ps_[head][i]]);
		}
	}
	sort(ps.begin(), ps.end());
	return ps;
}

Propositions Propnet::getState() {
	return get(r_true);
}

Propositions Propnet::getLegals() {
	return get(r_legal);
}

Propositions Propnet::getGoals() {
	return get(r_goal);
}

bool Propnet::isTerminal() {
	return components_[map_head_ps_[r_terminal][0]]->value_;
}

void Propnet::printTrues() {
	int count = 0;
	for (int i = 0; i < ps_.size(); ++i) {
		if (components_[i]->value_) {
			cout << ps_[i].toString() << endl;
			++count;
		}
	}
	cout << count << endl;
}