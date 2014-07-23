#include <vector>

#include "propnet.h"
#include "component.h"
#include "relation.h" 
#include "prover.h"

using namespace std;

void Propnet::init(Relations rs) {
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
	for (int i = 0; i < rs.size(); ++i) {
		rs[i].s_ = rs[i].toString();
	}
	Prover p2(rs);
	Propositions trues;
	for (int i = 0; i < bases.size(); ++i) {
		bases[i].head_ = r_true;
	}
	for (int i = 0; i < inputs.size(); ++i) {
		inputs[i].head_ = r_does;
	}
	trues.insert(trues.end(), bases.begin(), bases.end());
	trues.insert(trues.end(), inputs.begin(), inputs.end());
	p2.generateTrueProps(trues, 0, p2.dpg_.stra_deriv_.size() - 1);
	trues.insert(trues.end(), p2.statics_.begin(), p2.statics_.end());
	vector<string> scans;
	for (int i = 0; i < trues.size(); ++i) {
		scans.push_back(trues[i].toString());
	}

}