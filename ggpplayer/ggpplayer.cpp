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
		Relation::string2int_[relation_type_words[i]] = i;
		Relation::int2string_.push_back(relation_type_words[i]);
	}
	Reader r;
	if (!r.scanGDLFile("gdl/connect_four.txt")) {
        cout << "read file failed." << endl;
        return -1;
    }
	Relations rs;
	r.getRelations(rs);
	StateMachine machine(rs);
	Propositions goals = machine.randomGo();
	cout << "total: " << Prover::generate_time << endl;
	cout << "time1: " << Prover::time1 << endl;
	cout << "time2: " << Prover::time2 << endl;
	cout << "time3: " << Prover::time3 << endl;
	cout << "time4: " << Prover::time4 << endl;
	cout << "time5: " << Prover::time5 << endl;
	cout << "time6: " << Prover::time6 << endl;
	cout << "time7: " << Prover::time7 << endl;
	cout << "time8: " << Prover::time8 << endl;
	cout << "time9: " << Prover::time9 << endl;
	cout << "time10: " << Prover::time10 << endl;
	cout << "time11: " << Prover::time11 << endl;
	cout << "time12: " << Prover::time12 << endl;
	cout << "time13: " << Prover::time13 << endl;
	cout << "time14: " << Prover::time14 << endl;
	cout << "time15: " << Prover::time15 << endl;
	cout << "time16: " << Prover::time16 << endl;
	cout << "time17: " << Prover::time17 << endl;
	cout << "time18: " << Prover::time18 << endl;
	cout << "time19: " << Prover::time19 << endl;
	cout << "time20: " << Prover::time20 << endl;
	cout << "time21: " << Prover::time21 << endl;
	
	char buf[10000];
	cin.getline(buf, 10000);
	int role;	
	role = Relation::string2int_[string(buf)];
	cout << "ready" << endl;
		
	cin.getline(buf, 10000);
	if (machine.getRandomMove(role).toRelation().items_[1].items_.size() == 0) {		
		cout << machine.getRandomMove(role).toRelation().items_[1].toString() << endl;
	} else {
		cout << "( " << machine.getRandomMove(role).toRelation().items_[1].toString() << " )" << endl;
	}
	while (true) {				
		cin.getline(buf, 10000);	
		Reader move_reader;
		move_reader.file_content_ = buf;		
		Propositions joint_move;
		move_reader.getMoves(joint_move);
		for (int i = 0; i < joint_move.size(); ++i) {
			Relation does;
			does.content_ = r_does;
			does.type_ = r_does;			
			does.items_.push_back(machine.prover_.roles_[i].toRelation().items_[0]);
			does.items_.push_back(joint_move[i].toRelation());
			joint_move[i] = does.toProposition();
		}		
		machine.goOneStep(joint_move);
		if (machine.isTerminal()) {
			//cout << "Terminal." << endl;
			break;
		}
		if (machine.getRandomMove(role).toRelation().items_[1].items_.size() == 0) {		
			cout << machine.getRandomMove(role).toRelation().items_[1].toString() << endl;
		} else {
			cout << "( " << machine.getRandomMove(role).toRelation().items_[1].toString() << " )" << endl;
		}
	}
	return 0;
}
