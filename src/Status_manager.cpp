#include "Status_manager.h"

Status_manager::Status_manager(std::vector<Farm*>& focalFarms, const Parameters* parameters, 
	Grid_manager* grid, Control_actions* control) :
		seededFarms(focalFarms), // saved for output
		parameters(parameters),
		grid(grid),
    control(control),
    allPrems(grid->get_allFarms()),
    recentNotSus(0),
    pastEndTime(parameters->timesteps+100),
    nPrems(allPrems->size())
{
	verbose = verboseLevel;
	
	for (auto& f:focalFarms){ // set seed farms as exposed, also starts control sequence
		set_diseaseStatus(f, 1, "exp", parameters->latencyParams);
		// set time until reported
	}

if (verbose>1){
	std::cout<<focalFarms.size()<<" exposed premises initiated. End times for exposed premises: "<<std::endl;
	for (auto& e : diseaseStatuses.at("exp").farms){
		std::cout << e->get_end("exp") << ", ";
	}
	std::cout<<std::endl;
}

	// store species for formatting later
	species = parameters->species;
	
	// Specify sequence of file statuses and associated lag times
	// 1. notDangerousContact to reported
	statusShift notDC_rep {"notDangerousContact", "reported", parameters->infectiousParams};
	// 2. dangerousContact to reported
	statusShift DC_rep {"dangerousContact", "reported", parameters->infectiousParams};

	// Specify sequence of disease statuses and associated lag times
	// 1. Exposure to infectiousness, and infectiousness period
	statusShift exp_inf {"exp", "inf", parameters->infectiousParams};
	diseaseSeq.emplace_back(exp_inf);
	// 2. Infectiouness to immunity (end time past simulation end)
	statusShift inf_imm {"inf", "imm", std::make_tuple(pastEndTime,0)};
	diseaseSeq.emplace_back(inf_imm);
	
	// Specify sequence of control statuses and associated lag times
	// 1. Reported to implemented
	// 2. Implemented to effective
	// 3. Effective to inactive
	
	std::cout << "Status manager initiated."<<std::endl;
}

Status_manager::~Status_manager()
{
	for (auto& f:changedStatus){delete f.second;}
}

/// Creates Prem_status object for Farm f, sets control file status to "notDangerousContact"
void Status_manager::addPremStatus(Farm* f)
{
	int fid = f->Farm::get_id();
	changedStatus[fid] = new Prem_status(f);
}

/// Changes file status of a farm, and determines time when next file status begins.
/// Adds farm to appropriate status list with start and end times. If this is the first
/// farm listed for a particular status, set the placeholders for that status at the
/// beginning of the status list.
/// \param[in]	f	Farm for which status should be changed
///	\param[in]	startTime	Time at which this status begins (usually immediate)
/// \param[in]	status status to which this farm will be set
///	\param[in]	lag	Tuple of mean, variance of time lag in days until next status begins. For permanent statuses (i.e. reported), use mean=pastEndTime, var=0
void Status_manager::set_fileStatus(Farm* f, int startTime, std::string status, 
	std::tuple<double,double> timeUntilNextStatus)
{
	int fid = f->Farm::get_id();
	// if a prem_status object has not yet been made for this farm, make one
	if (changedStatus.count(fid)==0){
		addPremStatus(f);
	}

	// record status in Prem_status
	changedStatus.at(fid)->Prem_status::set_fileStatus(status);
	// set start and end time
	changedStatus.at(fid)->Prem_status::set_start(status,startTime);
	int endTime = startTime+normDelay(timeUntilNextStatus);
	changedStatus.at(fid)->set_end(status,endTime);
	
	// add to appropriate file-status list
	bool firstOfStatus = 0;
	if (diseaseStatuses.count(status)==0){
		firstOfStatus=1;
	}

	diseaseStatuses[status].farms.emplace_back(changedStatus.at(fid));
	if (firstOfStatus){
		diseaseStatuses.at(status).lo = 0;
		diseaseStatuses.at(status).hi = 0;
	}
	
}

/// Changes disease status of a farm, and determines time when next disease status begins.
/// Adds newly exposed farms to the "not susceptible" list to pass to Grid_checker.
/// Adds farm to appropriate status list with start and end times. If this is the first
/// farm listed for a particular status, set the placeholders for that status at the
/// beginning of the status list.
/// \param[in]	f	Farm for which status should be changed
///	\param[in]	startTime	Time at which this status begins (usually immediate)
/// \param[in]	status	Disease status to which this farm will be set
///	\param[in]	lag	Tuple of mean, variance of time lag in days until next disease status begins. For permanent statuses (i.e. immune), use mean=pastEndTime, var=0
void Status_manager::set_diseaseStatus(Farm* f, int startTime, std::string status, 
	std::tuple<double,double> timeUntilNextStatus)
{
	int fid = f->Farm::get_id();
	// if a prem_status object doesn't exist for this farm, make one
	if (changedStatus.count(fid)==0){
		addPremStatus(f);
	}

	if (status.compare("exp")==0){ // if farm is becoming exposed
		notSus.emplace_back(allPrems->at(fid)); // add to list of non-susceptibles
	}

	// set start and end times for this status
	changedStatus.at(fid)->Prem_status::set_start(status,startTime);
	int endTime = startTime+normDelay(timeUntilNextStatus);
	changedStatus.at(fid)->set_end(status,endTime);
	changedStatus.at(fid)->set_diseaseStatus(status);
	// add to appropriate disease-status list
	bool firstOfStatus = 0;
		if (diseaseStatuses.count(status)==0){firstOfStatus=1;}

	diseaseStatuses[status].farms.emplace_back(changedStatus.at(fid));
	if (firstOfStatus){
		diseaseStatuses.at(status).lo = 0;
		diseaseStatuses.at(status).hi = 0;
	}
}

/// Changes control status of a farm, and determines time when next control status begins.
/// Adds farm to appropriate status list with start and end times. If this is the first
/// farm listed for a particular status, set the placeholders for that status at the
/// beginning of the status list.
/// \param[in]	f	Farm for which status should be changed
///	\param[in]	startTime	Time at which this status begins (usually immediate)
/// \param[in]	status	Status to which this farm will be set
///	\param[in]	lag	Tuple of mean, variance of time lag in days until next status begins. For permanent statuses (i.e. culled), use mean=pastEndTime, var=0
void Status_manager::set_controlStatus(Farm* f, int startTime, std::string status, 
	std::string level, std::tuple<double,double> timeUntilNextStatus)
{
	int fid = f->Farm::get_id();
	// if a prem_status object doesn't exist for this farm, make one
	if (changedStatus.count(fid)==0){
		addPremStatus(f);
	}
	
	changedStatus.at(fid)->set_controlStatus(status, level);
	// set start and end times for this status
	changedStatus.at(fid)->Prem_status::set_start(status,startTime);
	int endTime = startTime+normDelay(timeUntilNextStatus);
	changedStatus.at(fid)->set_end(status,endTime);
	
	// add to appropriate disease-status list
	bool firstOfStatus = 0;
		if (diseaseStatuses.count(status)==0){firstOfStatus=1;}

	diseaseStatuses[status].farms.emplace_back(changedStatus.at(fid));
	if (firstOfStatus){
		diseaseStatuses.at(status).lo = 0;
		diseaseStatuses.at(status).hi = 0;
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
	if (diseaseStatuses.count(s.one)==1 &&  // if there are farms in this status
		diseaseStatuses.at(s.one).lo != diseaseStatuses.at(s.one).farms.size() ){ // and not all have expired
		// last valid position for lo is size() (if everything has expired) - everything before lo has expired
		// move hi iterator to appropriate position
		bool advance = (diseaseStatuses.at(s.one).hi != diseaseStatuses.at(s.one).farms.size()-1); // only continue if not at end
		// last valid position for hi is size()-1, because anything after hi hasn't started yet
		while (advance){
			if ( (diseaseStatuses.at(s.one).farms.at(diseaseStatuses.at(s.one).hi+1))->get_start(s.one) <= t ){ // if next farm started or starts status today (should be true first time)
if (verbose>2){std::cout<<"Next farm starts today."<<std::endl;}
				diseaseStatuses.at(s.one).hi++; // move hi placemarker forward
				advance = diseaseStatuses.at(s.one).hi != diseaseStatuses.at(s.one).farms.size()-1; // if not at end, keep going
			} else { // otherwise next farm doesn't start yet, stay in place
				advance = 0;
			}
		}
if (verbose>2){std::cout<<s.one<<" hi at "<<diseaseStatuses.at(s.one).hi;}

		// check farms from lo to hi for expired statuses, adjusting lo if needed
		std::vector<Prem_status*>::iterator lo_it = diseaseStatuses.at(s.one).farms.begin();
		std::vector<Prem_status*>::iterator hi_it = diseaseStatuses.at(s.one).farms.begin();
		std::advance(lo_it, diseaseStatuses.at(s.one).lo); // move iterator to lo
		std::advance(hi_it, diseaseStatuses.at(s.one).hi); // move iterator to hi

		for (auto it = lo_it; it <= hi_it; it++){ // check each farm* between lo and hi
			if ( (*it)->get_end(s.one) == t ){ // if validity of this status expires for this farm today
				set_diseaseStatus(*it, t, s.two, s.lagToTwo); // start next status for this farm - need to determine appropriate lag
				std::iter_swap(lo_it, it); // switch expired farm into low position
				lo_it++; // shift lo placemarker forward
				diseaseStatuses.at(s.one).lo++; // update stored integer
			}
		}
if (verbose>2){std::cout<<", lo at "<< diseaseStatuses.at(s.one).lo <<std::endl;}
	} // end "if there are farms in this status"
	} // end "for each disease transition"

	// special case - immune only moves hi forward, because status doesn't expire
	if(diseaseStatuses.count("imm")==1){
		bool advance = (diseaseStatuses.at("imm").hi != diseaseStatuses.at("imm").farms.size()-1); // only continue if not at end
		// last valid position for hi is size()-1, because anything after hi hasn't started yet
		while (advance){
			if ( (diseaseStatuses.at("imm").farms.at(diseaseStatuses.at("imm").hi+1))->get_start("imm") <= t ){ // if next farm started or starts status today (should be true first time)
				diseaseStatuses.at("imm").hi++; // move hi placemarker forward
				advance = diseaseStatuses.at("imm").hi != diseaseStatuses.at("imm").farms.size()-1; // if not at end, keep going
			} else { // otherwise next farm doesn't start yet, stay in place
				advance = 0;
			}
		}
if (verbose>2){std::cout<<"imm hi at "<<diseaseStatuses.at("imm").hi<<", lo at "<< diseaseStatuses.at("imm").lo << std::endl;}
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

void Status_manager::shipExposure(std::vector<Shipment*>& ships, int time)
// filters for any control measures impacting shipment spread
// records sources of infection in sources
// shipment has t, farm origID, farm destID, origin FIPS, dest FIPS, species, ban (0-2)
{
	std::vector<Farm*> toExpose;
	// fill in fields t, transmission (if applicable), ban, for each shipment
	for (auto& s:ships){
		s->t = time; // set time of shipment
		// if source is infectious and destination is susceptible
		// check with control for shipping ban level
// 		s->ban = control->checkShipBan(s);
// 		if (s->ban < 3 // if anything other than compliant with a ban: no ban (0), not yet in effect (1) or non-compliant (2)
// 			&& s->transmission == 1){// and transmission is possible
 			toExpose.emplace_back(allPrems->at(s->destID));
 			sources[allPrems->at(s->destID)].emplace_back(std::make_tuple(allPrems->at(s->origID),1)); // 1 is shipping as method of exposure
// 		}
	}
	// by this point, toExpose is modified less any bans
	expose(toExpose, time);
}

void Status_manager::expose(std::vector<Farm*>& farms, int t)
{
	for (auto& f:farms){
		// check if farm is susceptible
		if (get_diseaseStatus(f).compare("sus")==0){
			set_diseaseStatus(f,t,"exp",parameters->latencyParams);
		}
	}
}

std::string Status_manager::get_diseaseStatus(Farm* f) const
{
	std::string output;
	int fid = f->Farm::get_id();
	if (changedStatus.count(fid)==0){
		output = "sus";
	} else {
		output = changedStatus.at(fid)->get_diseaseStatus();
	}
	return output;
}

/// Copies all premises with current status s (as of last call to updates) into input vector
void Status_manager::premsWithStatus(std::string s, std::vector<Farm*>& output2)
{
	std::vector<Farm*> output1;
	if (diseaseStatuses.count(s)==1){
		std::vector<Prem_status*>::iterator lo_it = diseaseStatuses.at(s).farms.begin() + diseaseStatuses.at(s).lo;
		std::vector<Prem_status*>::iterator hi_it = diseaseStatuses.at(s).farms.begin() + diseaseStatuses.at(s).hi +1;
		std::vector<Prem_status*> outputPS(lo_it, hi_it); // return farms in [lo, hi]

		for (auto f:outputPS){
			output1.emplace_back(allPrems->at(f->get_id()));
		}
	}
	output1.swap(output2);
}

/// Returns current number of premises with status s (as of last call to updates) 
int Status_manager::numPremsWithStatus(std::string s)
{
	int total = 0;
	if (s.compare("sus")==0){ 
		total = nPrems - notSus.size();
	} else if (diseaseStatuses.count(s)==1){
		total = diseaseStatuses.at(s).hi - diseaseStatuses.at(s).lo +1; // number of farms between [lo, hi]
	} 
 return total;
}

/// \param[out] Vector of newly not-susceptible farms
/// Returns farms that became not-susceptible since this function was last called
void Status_manager::newNotSus(std::vector<Farm*>& output)
{
	// set start point to last endpoint marker (recentNotSus)
	std::vector<Farm*>::iterator start = std::next(notSus.begin(), recentNotSus); //advance iterator to recentNotSus places past begin()
	std::vector<Farm*>::iterator end = notSus.end();
	// return int to end of notSus
	std::vector<Farm*> recent (start,end);// return farms in [start, end]
	recent.swap(output);
	// move recentNotSus to past end of list (will be the beginning for next round)
	recentNotSus = notSus.size();
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
	if (diseaseStatuses.count("inf")==1){nInf = diseaseStatuses.at("inf").farms.size();}
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
