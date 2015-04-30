/*
Status_manager.cpp
Manages, stores, and retrieves statuses for premises and counties (FIPS). Both are 
unordered maps with the first key being the status, which is one of:
(for premises): sus, exp, (exp2), inf, imm
(for counties): reported, banOrdered, banActive

There are two differences in the premises and county maps. Premises maps have the second key
being the time, as that is frequently referenced in checking statuses. County maps have
FIPS as the second key, as the identity of county statuses is less time-sensitive.
In both maps, the value is time, but in the premises map, this is the END time for that 
particular status. In the county map, it is the START time for that particular status.

This is set up this way because premises (disease) statuses can only be one of the options
at a time: susceptible, exposed, infectious, or immune. Conversely, the FIPS statuses are
nested: those that have an active shipping ban had a ban ordered and those were reported 
at some point. Thus current statuses are determined by whether the current time is BEFORE
the premises map time, and AFTER the county map time.

The link between these two maps is the exp2 status at the premises level. This is an 
exception to the rule that a premises can only have one status at a time, but this is not
an actual disease status. Rather, this is initiated at the same time as the exp status, 
but with an end time of when it will be reported. At that time, the appropriate FIPS is 
initiated as reported.

Status of all premises is initially susceptible, all statuses stored
in the unordered_map "statusTimeFarms": first key is status (integer), second key
is end time (the last day for which that status is valid), stored value is vector of farm*.
Statuses are also stored with the farm object (only for current timepoint) for looking
up the status of an individual premises.
*/

#include "Status_manager.h"
#include "shared_functions.h"

Status_manager::Status_manager(std::vector<Farm*>& seedPool, int numRandomSeed, 
	std::unordered_map<std::string, std::tuple<double,double>>& in_params, 
	const std::unordered_map<int, Farm*>* allPrems, //necessary?
	int endTime,
	Control_actions* control_in)
	:
	params(in_params),
	control(control_in),
	pastEndTime(endTime+100),
	nPrems(allPrems->size())
// initialize with arguments:
// fname: file where seed (initially infectious) farms are listed by ID
// whichSeed: number of random farms to draw from file (0 uses all)
// params: vector of tuples containing mean and variance of delays:
// ...between exposure and infectiousness onset ["latency"]
// ...between infectiousness onset and recovery ["infectious"]
// allPrems: reference map of all other premises
// endTime: last timestep of the simulation (to set temporarily static statuses)
{
	verbose = verboseLevel;
	
	std::vector<Farm*> focalFarms;
	if (numRandomSeed!=0){ // select this number from seed pool
		random_unique(seedPool, abs(numRandomSeed), focalFarms);
		if (verbose>0){std::cout<<focalFarms.size()<<" initial premises infections."<<std::endl;}
	} else { // use all of seedPool
		focalFarms.swap(seedPool);
	}
	
	for (auto& f:focalFarms){ // set seed farms as exposed
		setStatus(f,1,"exp",params["latency"]);		
	}
	seededFarms = focalFarms; // saved for output	
		
if (verbose>2){
	std::cout<<focalFarms.size()<<" exposed premises initiated. End times for exposed premises: "<<std::endl;
	for (auto&e : ds.at("exp").farms){
		std::cout<<e->get_end("exp")<<", ";
	}
	std::cout<<std::endl;
}
	// start control sequence: assign reporting time for exposed farms
	control->addFarm(focalFarms,1, 1); // 3rd argument = 1 uses index-lagged reporting time
		
	// store species for formatting later
	auto speciesMap = (*focalFarms.begin())->get_spCounts(); // just get species list from first focal farm
	for (auto& s:(*speciesMap)){species.emplace_back(s.first);}
	
	// Specify sequence of disease statuses and associated lag times
	// 1. Exposure to infectiousness
	statusShift exp_inf {"exp","inf",params["infectious"]};
	diseaseSeq.emplace_back(exp_inf);
	// 2. Infectiouness to immunity (end time past simulation)
	statusShift inf_imm {"inf","imm",std::make_tuple(pastEndTime,0)};
	diseaseSeq.emplace_back(inf_imm);
}

Status_manager::~Status_manager()
{
	for (auto& f:allNotSus){delete f.second;}
}

void Status_manager::setStatus(Farm* f_in, int startTime, std::string status, std::tuple<double,double> lag)
// adds to notSus lists and control if farm is just starting disease sequence
// adds to new status lists and assigns end times
// for permanent statuses, set lag params mean=pastEndTime, var=0
{
	Farm* f = f_in; //pointer copy
	
	if (status.compare("exp")==0){ // if farm is becoming exposed (first appearance in Status_manager)
		int fid = f->Farm::get_id();
		allNotSus[fid] = new Farm(*f_in);
		notSus.emplace_back(allNotSus.at(fid)); // gets cleared at each timepoint
		// and start control sequence
		control->addFarm(allNotSus.at(fid),startTime);
if (verbose>2){std::cout<<notSus.size()<<" not sus farms, "<<allNotSus.size()<<" allNotSus farms."<<std::endl;}
	}	 

	// otherwise, farm is already in Status and f_in is the Status_manager copy
	// set start and end times for this status
	f->set_start(status,startTime);
	int endTime = startTime+normDelay(lag);
	f->set_end(status,endTime);
	// add to appropriate disease-status list
	bool firstOfStatus = 0;	
		if (ds.count(status)==0){firstOfStatus=1;}
	ds[status].farms.emplace_back(f);
	// add to event time list if not already there
	eventTimes.emplace(endTime);
	if (firstOfStatus){
		ds.at(status).lo = 0;
		ds.at(status).hi = 0;
	}
}

void Status_manager::updates(int t)
{
	// for exposed and infectious vectors:
	// check between iterators "lo" and "hi"
	// farms before "lo" have already passed end time/expired
	// farms after "hi" have not yet reached start time

if (verbose>2){std::cout<<std::endl<<"Updating farms at time "<<t<<std::endl;}

	for (auto& s:diseaseSeq){ // for each status ("one") (with associated next step "two")
	if (ds.count(s.one)==1 &&  // if there are farms in this status
		ds.at(s.one).lo != ds.at(s.one).farms.size() ){ // and not all have expired
		// last valid position for lo is size() (if everything has expired) - everything before lo has expired
		// move hi iterator to appropriate position
		bool advance = (ds.at(s.one).hi != ds.at(s.one).farms.size()-1); // only continue if not at end
		// last valid position for hi is size()-1, because anything after hi hasn't started yet
		while (advance){
			if ( (ds.at(s.one).farms.at(ds.at(s.one).hi+1))->get_start(s.one) <= t ){ // if next farm started or starts status today (should be true first time)
if (verbose>2){std::cout<<"Next farm starts today."<<std::endl;}
				ds.at(s.one).hi++; // move hi placemarker forward
				advance = ds.at(s.one).hi != ds.at(s.one).farms.size()-1; // if not at end, keep going
			} else { // otherwise next farm doesn't start yet, stay in place
				advance = 0;
			}
		}
if (verbose>2){std::cout<<s.one<<" hi at "<<ds.at(s.one).hi;}		

		// check farms from lo to hi for expired statuses, adjusting lo if needed
		std::vector<Farm*>::iterator lo_it = ds.at(s.one).farms.begin();
		std::vector<Farm*>::iterator hi_it = ds.at(s.one).farms.begin();
		std::advance(lo_it, ds.at(s.one).lo);
		std::advance(hi_it, ds.at(s.one).hi);
			
		for (auto it = lo_it; it <= hi_it; it++){ // check each farm* between lo and hi
			if ( (*it)->get_end(s.one) == t ){ // if validity of this status expires for this farm today
				setStatus(*it, t, s.two, s.lagToTwo); // start next status for this farm
				std::iter_swap(lo_it, it); // switch expired farm into low position
				lo_it++; // shift lo placemarker forward
				ds.at(s.one).lo++; // update stored integer
			}
		}	
if (verbose>2){std::cout<<", lo at "<< ds.at(s.one).lo <<std::endl;}		
	} // end "if there are farms in this status"
	} // end "for each disease transition"
	
	// special case - immune only moves hi forward, because status doesn't expire
	if(ds.count("imm")==1){
		bool advance = (ds.at("imm").hi != ds.at("imm").farms.size()-1); // only continue if not at end
		// last valid position for hi is size()-1, because anything after hi hasn't started yet
		while (advance){
			if ( (ds.at("imm").farms.at(ds.at("imm").hi+1))->get_start("imm") <= t ){ // if next farm started or starts status today (should be true first time)
				ds.at("imm").hi++; // move hi placemarker forward
				advance = ds.at("imm").hi != ds.at("imm").farms.size()-1; // if not at end, keep going
			} else { // otherwise next farm doesn't start yet, stay in place
				advance = 0;
			}
		}
if (verbose>2){std::cout<<"imm hi at "<<ds.at("imm").hi<<", lo at "<< ds.at("imm").lo << std::endl;}
	} // end if any imm
	
	// special case - if any farms vaccinated, add to notSus

}

void Status_manager::localExposure(std::vector<Farm*>& farms, int t)
// filters for any control measures impacting local spread
// records sources of infection
{
	std::vector<Farm*> toExpose = farms;
	// check control parts of farms for premises-specific effects (vax, etc)
	expose(toExpose, t);
}
/*
void Status_manager::shipExposure(shipments, int t)
// filters for any control measures impacting shipment spread
// records sources of infection
{
	std::vector<Farm*> toExpose;
	// check with control for shipping bans
	expose(toExpose, t);
}
*/
void Status_manager::expose(std::vector<Farm*>& farms, int t)
{
	for (auto& f:farms){
		// check if farm has already been exposed
		if (!f->beenExposed()){
			setStatus(f,t,"exp",params["latency"]); 
		}
	}
}

void Status_manager::premsWithStatus(std::string s, std::vector<Farm*>&output1)
{ // get all farms with status s between iterators (as of last call to updates)

	std::vector<Farm*> output; // if no s, returns empty vector
	
	if (ds.count(s)==1){
		std::vector<Farm*>::iterator lo_it = ds.at(s).farms.begin() + ds.at(s).lo;
		std::vector<Farm*>::iterator hi_it = ds.at(s).farms.begin() + ds.at(s).hi +1;
		std::vector<Farm*> output2(lo_it, hi_it); // return farms in [lo, hi]
		output2.swap(output); // workaround to use copy initializer with iterators
	}
	
 output.swap(output1);
}

int Status_manager::numPremsWithStatus(std::string s)
{ // get # farms with status s (as of last call to updates)
	int total = 0;
	if (s.compare("sus")==0){ total = nPrems - allNotSus.size();
	} else if (ds.count(s)==1){ 
		total = ds.at(s).hi - ds.at(s).lo +1;
	} // return number of farms in [lo, hi]
 return total;
}

void Status_manager::get_seedCos(std::vector<std::string>& output)
{
	std::vector<std::string> fips;
	fips.reserve(seededFarms.size());
	for (auto& s:seededFarms){
		fips.emplace_back(s->get_fips());
	}
	fips.swap(output);
}

std::string Status_manager::formatRepSummary(int rep, int duration, double repTimeMS)
{
	int nInf = 0;
	if (ds.count("inf")==1){nInf = ds.at("inf").farms.size();}
	std::vector<int> seedIDs; seedIDs.reserve(seededFarms.size());
	for (auto& sf:seededFarms){
		seedIDs.emplace_back(sf->Farm::get_id());
	}
	std::string seeds = vecToCommaSepString(seedIDs);
	
	std::vector<std::string> seedFips;
	get_seedCos(seedFips);
	std::string seedCos = vecToCommaSepString(seedFips);

	double repTimeS = repTimeMS/1000;
	std::string toPrint;
	addItemTab(toPrint, rep); // rep #
if(verbose>1){std::cout<<"rep "<<rep;}
	addItemTab(toPrint, nInf); // # total infectious (includes seeds)
if(verbose>1){std::cout<<", nInf "<<nInf;}
	addItemTab(toPrint, duration); // duration of epidemic	
if(verbose>1){std::cout<<", duration "<<duration;}
	addItemTab(toPrint, seeds); // seed farm(s)
if(verbose>1){std::cout<<", seeds "<<seeds;}
	addItemTab(toPrint, seedCos); // seed county(s)
if(verbose>1){std::cout<<", seedCos "<<seedCos;}
	addItemTab(toPrint, repTimeS); // runtime
if(verbose>1){std::cout<<", repTimeSec "<<repTimeS<<std::endl;}
	toPrint.replace(toPrint.end()-1, toPrint.end(), "\n"); // add line break at end
if(verbose>1){std::cout<<"toPrint: "<<toPrint<<std::endl;}
	
	return toPrint;
}

std::string Status_manager::formatDetails(int rep, int t)
// rep, ID, time, sourceID, method - not including initial seeds
{
	std::string toPrint;
	std::unordered_map< Farm*, std::vector<std::tuple<Farm*, int>> > empty;
	
	if (sources.size()>0){
	for (auto& info:sources){
		int expPrem = info.first->Farm::get_id();
		std::vector<int> sourceIDs; sourceIDs.reserve(info.second.size());
		std::vector<int> routes; routes.reserve(info.second.size());
		for (auto& si:info.second){
			sourceIDs.emplace_back(std::get<0>(si)->Farm::get_id()); // source premises ID
			routes.emplace_back(std::get<1>(si)); // route of exposure
		}
		std::string sourceIDs_str = vecToCommaSepString(sourceIDs);
		std::string routes_str = vecToCommaSepString(routes);

		addItemTab(toPrint, rep); // rep #
		addItemTab(toPrint, expPrem); // exposed prem ID
		addItemTab(toPrint, t); // time
		addItemTab(toPrint, sourceIDs_str); // source prem ID(s)
		addItemTab(toPrint, routes_str); // route(s) of exposure: 0=local, 1=shipment
		toPrint.replace(toPrint.end()-1, toPrint.end(), "\n"); // add line break at end
	}
	empty.swap(sources); // clear sources so only one timepoint is printed at a time
	}
	return toPrint;
}
