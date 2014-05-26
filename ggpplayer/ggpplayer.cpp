#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "reader.h"
#include "relation.h"
#include "prover.h"
#include  "dependgraph.h"
#include "statemachine.h"

using namespace std;

int main() {
	for(int i = 0 ; i < relation_type_num; ++i){		
		Relation::addSymbol(relation_type_words[i]);
	}
	Reader r;
	if (!r.scanGDLFile("gdl/rule.txt")) {
        cout << "read file failed." << endl;
        return -1;
    }
	Relations rs;
	r.getRelations(rs);
	StateMachine machine(rs);
	char buf[10000];
	cin.getline(buf, 10000);
	int role;	
	role = Relation::symbol2code[string(buf)];
	for (int i = 0; i < machine.prover_.roles_.size(); ++i) {
		if (machine.prover_.roles_[i].items_[0].head_ == role) {
			role = i;
			break;
		}
	}
	cout << "ready" << endl;
		
	cin.getline(buf, 10000);
	cout << machine.getRandomMove(role).items_[1].toString() << endl;
	
	while (true) {				
		cin.getline(buf, 10000);	
		Reader move_reader;
		move_reader.file_head_ = buf;		
		Propositions joint_move;
		move_reader.getMoves(joint_move);
		for (int i = 0; i < joint_move.size(); ++i) {
			Relation does;
			does.head_ = r_does;						
			does.items_.push_back(machine.prover_.roles_[i].toRelation().items_[0]);
			does.items_.push_back(joint_move[i].toRelation());
			joint_move[i] = does.toProposition();
		}		
		machine.goOneStep(joint_move);
		if (machine.is_terminal_) {
			//cout << "Terminal." << endl;
			break;
		}
		Proposition move = machine.getRandomMove(role);
		cout << move.items_[1].toString() << endl;
	}
	return 0;
}
