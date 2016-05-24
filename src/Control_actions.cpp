#include "Control_actions.h"
#include "Farm.h"
#include "file_manager.h" // for parameters, Control_rule structs
#include "County.h"

Control_actions::Control_actions(const Parameters* p)
//	:
//	cl(p->controlLags)
{
	verbose = verboseLevel;

	// establish control types, set up related decision-making functions?
}

Control_actions::~Control_actions()
{
}
