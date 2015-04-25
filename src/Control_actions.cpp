#include "Control_actions.h"

Control_actions::Control_actions(std::unordered_map< std::string, std::vector<std::tuple<double,double>> >& controlParams) // input map of tuples of lag parameters "cl"
:
cl(controlParams)
{
	// could reserve estimated # of exposures for farms
	// could reserve total number counties for counties
	
	farmIndex = farmsToChange.begin();
	countyIndex = countiesToChange.begin();
	
	// set counts for each status (controltype[level], i.e. shipBan[1] = shipping ban ordered) to 0
	for (auto& type:cl){ // type.first = control type
		for (int i = 0; i < type.second.size(); i++){
			farmStatusCounts[type.first].emplace_back(0);
			countyStatusCounts[type.first].emplace_back(0); 
		} // should have vector of 0s, 0 through max level
		cTypeMax[type.first] = type.second.size()-1; // maximum level for this control type
	}
	
}

Control_actions::~Control_actions()
{
	for (auto& c:counties){delete c.second;}
}

// Add farms to the control system, copies farms and starts reporting sequence
void Control_actions::addFarm(std::vector<Farm*>& fvec, int t, bool first) // by default "first"=0
{
	for (auto& f:fvec){
	  	std::tuple<double,double> reportParams = cl.at("report").front();
	  	if (first){ reportParams = cl.at("indexReport").front();  }// use index lag to determine time farm will be reported
	  	int rTime = t + normDelay(reportParams); // determine time farm will be reported
	  
		f->set_status("report",0); // set farm's control status, 0 = not reported
		farms.emplace(f); // add to unordered_set
		nextChange<Farm> nc{ f, "report", 1 }; // this farm will have "reported" status 1...
		farmsToChange[rTime].emplace_back(nc); // ...at rTime
	}
}

// Overload for single farm
void Control_actions::addFarm(Farm* f, int t, bool first) // by default "first"=0
{
	std::vector<Farm*> fvec;
	fvec.emplace_back(f);
	
	addFarm(fvec,t,first); // call vector version
}

void Control_actions::updates(int t)
{
	// check indexed item in countiesToChange to see if changes need to be made
	while (countyIndex->first == t){ // if the next timepoint requiring action is today (t)
		std::vector< nextChange<County> >& changes = countyIndex->second; // isolate the farms to be changed
		for (auto& c:changes){
			County* co = c.unit;
			co->statuses[c.controlType] = c.level;
			countyStatusCounts.at(c.controlType).at(c.level)++; // add to total count of this status-level
			// if not at end of control sequence, add next status change to list
			if (c.level < cTypeMax.at(c.controlType)){ // if at any stage before last
				scheduleLevelUp_c(co,c.controlType,c.level+1);
			}
		}
		countyIndex++; // start next check at the following "nextChange" item
	}

	// check indexed item in farmsToChange to see if changes need to be made
	while (farmIndex->first == t){ // if the next timepoint requiring action is today (t)
		std::vector< nextChange<Farm> >& changes = farmIndex->second; // isolate the farms to be changed
		for (auto& c:changes){
			Farm* f = c.unit;
			f->set_status(c.controlType,c.level);
			farmStatusCounts.at(c.controlType).at(c.level)++; // add to total count of this status-level
			// if before end of control sequence for this type, schedule next status update
			if (c.level < cTypeMax.at(c.controlType)){			
				scheduleLevelUp_f(f,c.controlType,c.level+1);
			}
			// at reporting time, cross-post at county level
			if (c.controlType.compare("report") && c.level==1){ // if this farm was just reported
				std::string farmfips = f->get_fips();
				
				if (counties.count(farmfips)==0){ // if this county hasn't been encountered yet
					// create and add to county map
					County* co = new County;
					co->fips = farmfips;
					co->statuses["report"] = 1; // county is also reported when farm is reported
					countyStatusCounts["report"].at(1)++;
					counties[farmfips] = co;
				}
				
				// start shipping ban sequence
				startControlSeq_c(counties.at(farmfips), "shipBan");	
			}
		}
		farmIndex++; // start next check at the following "nextChange" item
	}
}

void Control_actions::scheduleLevelUp_f(Farm* f, std::string cType, int level)
// add a status shift to the to-do list for farms
{
	nextChange<Farm> next{ f, cType, level };
	int time = normDelay(cl.at(cType).at(level));
	farmsToChange[time].emplace_back(next);
}

void Control_actions::scheduleLevelUp_c(County* co, std::string cType, int level)
// add a status shift to the to-do list for counties
{
	nextChange<County> next{ co, cType, level };
	int time = normDelay(cl.at(cType).at(level));
	countiesToChange[time].emplace_back(next);
}

void Control_actions::startControlSeq_c(County* co, std::string cType)
{
	co->statuses[cType] = 0;
	countyStatusCounts.at(cType).at(0)++; // increase count at level 0 (starting sequence)
	scheduleLevelUp_c(co,cType,1);
}	
	
void Control_actions::startControlSeq_f(Farm* f, std::string cType)
{
	f->set_status(cType,0);
	farmStatusCounts.at(cType).at(0)++; // increase count at level 0 (starting sequence)
	scheduleLevelUp_f(f,cType,1);
}	

// should be accessible directly from Status_manager's copy of Farm
// int Control_actions::getFarmLevel(Farm* f, std::string ctype) const
// {
// 	int output = -1; // returned if not in control list
// 	if (farms.count(f)==1){ 
// 		output = farms.at(f).statuses.at(ctype);
// 	}
// 	return output;
// }

int Control_actions::getCountyLevel(std::string fips, std::string ctype) const
{
	int output = -1; // returned if not in control list
	if (counties.count(fips)==1){
		output = counties.at(fips)->statuses.at(ctype);
	}
	return output;
}