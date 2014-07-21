#include <vector>

#include "propnet.h"
#include "component.h"
#include "relation.h" 
#include "prover.h"

void Propnet::init(Relations rs) {
	Prover p(rs);
	Propositions bases = p.bases_;
	Propositions inputs = p.inputs_;
	
}