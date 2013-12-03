#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#include "reader.h"
#include "relation.h"
#include "prover.h"
#include "domaingraph.h"
#include "tools.h"

using namespace std;

int main() {
	Reader r;
	r.scanGDLFile("gdl\\tic_tac_toe.txt");
	Relations rs;
	r.getRelations(rs);
	State initstate;
	Prover p(rs);
	p.getInitState(initstate);
	State nextstate;
	p.getNextState(initstate, nextstate);
	Relations rs_goal, rs_role;
	p.askRole(rs_role);
	p.askGoal(rs_goal,initstate);
	while(true){
		if(p.askTerminal(initstate)) {
			break;
		}
		Relations rs_legal, rs_legal_oplayer, rs_legal_xplayer;
		//p.askLegalActions(rs_legal,initstate);
		p.askLegalActions(rs_legal_oplayer, *(rs_role.begin()), initstate);
		p.askLegalActions(rs_legal_xplayer, *(++rs_role.begin()), initstate);
		Relations does;
		Relation does_oplayer = *rs_legal_oplayer.begin();
		does_oplayer.content_ = "does";
		does_oplayer.type_ = RelationType::r_does;
		Relation does_xplayer = *rs_legal_xplayer.begin();
		does_xplayer.content_ = "does";
		does_xplayer.type_ = RelationType::r_does;
		does.push_back(does_xplayer);
		does.push_back(does_oplayer);
		nextstate.clear();
		p.askNextState(nextstate, initstate,does);
		initstate = nextstate;
	}
	return 0;
}
