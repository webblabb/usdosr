#include "Control_actions.h"
#include "Farm.h"
#include "County.h"

Control_actions::Control_actions(std::unordered_map< std::string, std::vector<std::tuple<double,double>> >& controlParams) // input map of tuples of lag parameters "cl"
	:
	cl(controlParams)
{
	verbose = verboseLevel;
	// could reserve estimated # of exposures for farms
	// could reserve total number counties for counties

	// set counts to 0 for each status where applicable

	// "report" applies to both counties and farms
	int size = cl.at("report").size()+1; // how many sets of parameters in vector for this control type
	// add one because no lag needed for first status
	std::vector<int> v1 (size, 0);
	farmStatusCounts["report"] = v1;
	countyStatusCounts["report"] = v1;
if(verbose>1){
	std::cout<<"Control initiation: Report counts for farms set to ";
	for (auto&v:farmStatusCounts["report"]){std::cout<<v<<", ";}
	std::cout<<std::endl;
	std::cout<<"Report counts for counties set to ";
	for (auto&v:countyStatusCounts["report"]){std::cout<<v<<", ";}
	std::cout<<std::endl;
}

	// "shipBan": 0: will have action ordered, 1: ban ordered, 2: ban implemented
	// farms can have level 3, "compliant", assigned separately from regular updates
	size = cl.at("shipBan").size();
	std::vector<int> v2 (size, 0);
	countyStatusCounts["shipBan"] = v2;
if(verbose>1){
	std::cout<<"shipBan counts for county set to ";
	for (auto&v:countyStatusCounts["shipBan"]){std::cout<<v<<", ";}
	std::cout<<std::endl;
}
	for (auto& type:cl){ // type.first = control type
		cTypeMax[type.first] = type.second.size()-1; // maximum level for this control type (subtract 1 to discount 0-index)
if(verbose>1){std::cout<<"Maximums for control types: "<<type.first<<": "<<cTypeMax[type.first]<<std::endl;}
	}

}

Control_actions::~Control_actions()
{
	for (auto& c:counties){delete c.second;}
}

// Add farms to the control system and starts reporting sequence
void Control_actions::addFarm(std::vector<Prem_status*>& fvec, int t, bool first) // by default "first"=0
{
	for (auto& f:fvec){
	  	std::tuple<double,double> reportParams = cl.at("report").front(); // only one entry, access via front()
	  	if (first){ reportParams = cl.at("indexReport").front();  }// use index lag to determine time farm will be reported
	  	int rTime = t + normDelay(reportParams); // determine time farm will be reported
if(verbose>2){std::cout<<"Assigning report time of "<<rTime<<std::endl;}
		f->set_status("report",0); // set farm's control status, 0 = not reported
		nextChange<Prem_status> nc{ f, "report", 1 }; // this farm will have "reported" status 1...
		farmsToChange[rTime].emplace_back(nc); // ...at rTime
	}
}

// Overload for single farm
void Control_actions::addFarm(Prem_status* f, int t, bool first) // by default "first"=0
{
	std::vector<Prem_status*> fvec;
	fvec.emplace_back(f);

	addFarm(fvec,t,first); // call vector version of addFarm function
}

void Control_actions::updates(int t)
{
	// check countiesToChange to see if changes need to be made
	if (countiesToChange.count(t)==1){ // if any changes happen today (t)
		std::vector< nextChange<County*> >& changes = countiesToChange.at(t); // isolate the farms to be changed
		for (auto& c:changes){
			County* co = c.unit;
			co->set_control_status(c.controlType, c.level); // set level for this control type
			countyStatusCounts.at(c.controlType).at(c.level)++; // add to total count of this status-level

if(verbose>1){std::cout<<co->get_id()<<" updated to "<<c.controlType<<" level "<<c.level<<std::endl;}
			// if not at end of control sequence, add next status change to list
			if (c.level < cTypeMax.at(c.controlType)){ // if at any stage before last
				scheduleLevelUp_c(co,c.controlType,c.level+1, t);
if(verbose>1){std::cout<<"Next level-up to "<<c.level+1<<" scheduled."<<std::endl;}
			}
		}
	}

	// check indexed item in farmsToChange to see if changes need to be made
	if (farmsToChange.count(t)==1){ // if the next timepoint requiring action is today (t)
		std::vector< nextChange<Prem_status> >& changes = farmsToChange.at(t); // isolate the farms to be changed
if(verbose>2){std::cout<<changes.size()<<" farm changes happening today."<<std::endl;}
		for (auto& c:changes){
			Prem_status* f = c.unit;
			f->set_status(c.controlType,c.level);
			farmStatusCounts.at(c.controlType).at(c.level)++; // add to total count of this status-level
if(verbose>1){std::cout<<"Farm "<<f->get_id()<<" updated to "<<c.controlType<<" level "<<c.level<<std::endl;}
			// if before end of control sequence for this type, schedule next status update
			if (c.level < cTypeMax.at(c.controlType)){
				scheduleLevelUp_f(f,c.controlType,c.level+1);
			}
			// at reporting time, cross-post at county level
			if (c.controlType.compare("report")==0 && c.level==1){ // if this farm was just reported
				//std::string farmfips = f->get_fips();

				County* farmfips = f->get_parent_county();
				if(farmfips->get_control_status("report") == 0) //This county is not yet reported.
                {
                    farmfips->set_control_status("report", 1);
                    countyStatusCounts["report"].at(1)++;
                }

				// start shipping ban sequence at county level
				startControlSeq_c(farmfips, "shipBan");
			}
		}
	}
}

void Control_actions::scheduleLevelUp_f(Prem_status* f, std::string cType, int level, int t)
// add a status shift to the to-do list for farms
{
	nextChange<Prem_status> next{ f, cType, level };
	int time = t + normDelay(cl.at(cType).at(level));
	farmsToChange[time].emplace_back(next);
}

void Control_actions::scheduleLevelUp_c(County* co, std::string cType, int level, int t)
// add a status shift to the to-do list for counties
{
	nextChange<County*> next{ co, cType, level };
	int time = t + normDelay(cl.at(cType).at(level));
	countiesToChange[time].emplace_back(next);
if(verbose>1){std::cout<<"Level up scheduled for "<<cType<<" to level "<<level<<" at "<<time<<std::endl;}
}

void Control_actions::startControlSeq_c(County* co, std::string cType, int t)
{
    co->set_control_status(cType, 0);
//	co->statuses[cType] = 0;
	countyStatusCounts.at(cType).at(0)++; // increase count at level 0 (starting sequence)
	scheduleLevelUp_c(co,cType,1,t);
}

void Control_actions::startControlSeq_f(Prem_status* f, std::string cType, int t)
{
	f->set_control_status(cType,0);
	farmStatusCounts.at(cType).at(0)++; // increase count at level 0 (starting sequence)
	scheduleLevelUp_f(f,cType,1,t);
}

double Control_actions::compliance_shipBan()
// arguments are whatever's needed to decide
{
	return 0.5; // temp set at 50% compliance
}

int Control_actions::checkShipBan(Shipment* ship)
{
	int level = 0; // returned if no ban in place
	if (counties.count(ship->origFIPS)==1){
		level = counties.at(ship->origFIPS)->get_control_status("shipBan");
		if (level == 2){ // if a ban is implemented for this county
			// determine compliance
			double banCompliance = compliance_shipBan(); // based on decision rules
			double randCompliance = unif_rand();
			if (randCompliance <= banCompliance){
				++level; // compliant: level 3
			}
		}
	}
	return level;
}
