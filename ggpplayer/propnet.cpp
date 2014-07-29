#include <vector>
#include <algorithm>

#include "propnet.h"
#include "component.h"
#include "relation.h" 
#include "prover.h"
#include "combination.h"

using namespace std;

Propositions getInits(Relations rs) {
	Propositions ret;
	for (int i = 0; i < rs.size(); ++i) {
		if (rs[i].head_ == r_init) {
			ret.push_back(rs[i].toProposition());
		}
	}
	return ret;
}

void Propnet::init(Relations rs) {
	for (int i = 0; i < rs.size(); ++i) {
		rs[i].s_ = rs[i].toString();
	}

	Prover p(rs);
	Propositions bases = p.bases_;
	Propositions inputs = p.inputs_;
	for (int i = 0; i < rs.size(); ++i) {
		if (rs[i].head_ == r_derivation) {
			for (vector<Relation>::iterator j = rs[i].items_.begin() + 1; j != rs[i].items_.end(); ) {
				if (j->head_ == r_not) {
					j = rs[i].items_.erase(j);
				} else {
					++j;
				}
			}
		}
	}
	
	Prover p2(rs);
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
		if (ps_[i].head_ != r_next) {
			components_.push_back(new Component(c_or));
		} else {
			components_.push_back(new Component(c_transition));
		}
	}

	map<int, vector<int> > map_head_ps;
	map<string, int> map_string_p;
	for (int i = 0; i < ps_.size(); ++i) {
		map_head_ps[ps_[i].head_].push_back(i);
		map_string_p[ps_[i].toString()] = i;
	}

	for (int i = 0; i < p.statics_.size(); ++i) {
		Component *c = components_[map_string_p[p.statics_[i].toString()]];
		c->value_ = true;
		c->last_value_ = true;
	}

	vector<string> scans;
	for (int i = 0; i < ps_.size(); ++i) {
		scans.push_back(ps_[i].toString());
	}
	
	Propositions inits = getInits(rs);
	for (int i = 0; i < inits.size(); ++i) {
		Proposition p = inits[i];
		p.head_ = r_next;
		components_[map_string_p[p.toString()]]->last_value_ = true;
		p.head_ = r_true;
		components_[map_string_p[p.toString()]]->value_ = true;
		components_[map_string_p[p.toString()]]->last_value_ = true;
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
			if (target.head_ == r_goal) {
				//cout << "1" << endl;
			}
			for (int j = 0; j < map_head_ps[target.head_].size(); ++j) {
				vector<int> target_variables;
				vector<int> target_values;
				if (target.matches(ps_[map_head_ps[target.head_][j]], target_variables, target_values)) {
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
					vector<vector<vector<int> > > multiple_combinations;
					for (int k = 0; k < d.subgoals_.size(); ++k) {
						Proposition subgoal = d.subgoals_[k];
						if (subgoal.head_ == r_distinct) {
							continue;
						}
						if (subgoal.head_ == r_not) {
							subgoal = subgoal.items_[0];
						}
						subgoal.replaceVariables(target_variables, target_values);						
						vector<vector<int> > combinations;
						for (int ii = 0; ii < map_head_ps[subgoal.head_].size(); ++ii) {
							vector<int> subgoal_variables;
							vector<int> subgoal_values;
							if (subgoal.matches(ps_[map_head_ps[subgoal.head_][ii]], subgoal_variables, subgoal_values)) {
								vector<int> combination = getCombination(undetermined_vars, subgoal_variables, subgoal_values);
								combinations.push_back(combination);
							}
						}
						multiple_combinations.push_back(combinations);
					}
					vector<vector<int> > combinations = mergeMultipleCombinations(multiple_combinations, variable_distincts, constant_distincts);
					if (combinations.size() > 0) {
						Component *or = new Component(c_or);
						components_.push_back(or);
						or->addOutput(components_[map_head_ps[target.head_][j]]);
						for (int k = 0; k < combinations.size(); ++k) {
							Component *and = new Component(c_and);
							components_.push_back(and);
							and->addOutput(or);
							for (int ii = 0; ii < d.subgoals_.size(); ++ii) {
								if (d.subgoals_[ii].head_ == r_distinct) {
									continue;
								} else {
									Proposition subgoal = d.subgoals_[ii];
									subgoal.replaceVariables(target_variables, target_values);
									subgoal.replaceVariables(undetermined_vars, combinations[k]);
									if (d.subgoals_[ii].head_ == r_not) {
										Component *c = components_[map_string_p[subgoal.items_[0].toString()]];
										Component *not = c->getNotOutput();
										if (not == NULL){
											not = new Component(c_not);
											components_.push_back(not);
											c->addOutput(not);
										}
										not->addOutput(and);
									} else {
										//cout << subgoal.toString() << endl;
										Component *c = components_[map_string_p[subgoal.toString()]];
										c->addOutput(and);
										if (components_[map_head_ps[r_goal][0]]->value_) {
											cout << ps_[map_head_ps[r_goal][0]].toString() << endl;
										}
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
	for (int i = 0; i < map_head_ps[r_next].size(); ++i) {
		Proposition next_p = ps_[map_head_ps[r_next][i]];
		Proposition true_p = next_p;
		true_p.head_ = r_true;
		Component *n = components_[map_string_p[next_p.toString()]];
		Component *t = components_[map_string_p[true_p.toString()]];
		n->addOutput(t);
	}
	
	for (int i = 0; i < ps_.size(); ++i) {
		if (components_[i]->value_) {
			cout << ps_[i].toString() << endl;
			system("pause");
		}
	}
}