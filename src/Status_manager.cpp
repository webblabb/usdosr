#include "Status_manager.h"

<<<<<<< HEAD

Status_manager::Status_manager(std::vector<Farm*>& seedPool, int numRandomSeed, 
	std::unordered_map<std::string, std::tuple<double,double>>& in_params, 
=======
Status_manager::Status_manager(std::vector<Farm*>& seedPool, int numRandomSeed,
	std::unordered_map<std::string, std::tuple<double,double>>& in_params,
>>>>>>> origin/master
	const std::unordered_map<int, Farm*>* allPrems, // needed for interfacing with main
	int endTime,
	Control_actions* control_in) :
        params(in_params),
        control(control_in),
        allFarms(allPrems),
        recentNotSus(0),
        pastEndTime(endTime+100),
        nPrems(allPrems->size())
{
	verbose = verboseLevel;
	std::vector<Farm*> focalFarms;
	if (numRandomSeed!=0){ // select this number from seed pool
		random_unique(seedPool, abs(numRandomSeed), focalFarms);
if (verbose>0){std::cout<<focalFarms.size()<<" initial premises infection(s): ";
for (auto& sf:focalFarms){
	std::cout<<sf->Farm::get_id()<<", ";
}
	std::cout<<std::endl;
}
	} else { // use all of seedPool
		focalFarms.swap(seedPool);
	}

	for (auto& f:focalFarms){ // set seed farms as exposed, also starts control sequence
		setStatus(f,1,"exp",params.at("latency"),1); // last argument says 'first' is true - use index lags
	}
	seededFarms = focalFarms; // saved for output

if (verbose>1){
	std::cout<<focalFarms.size()<<" exposed premises initiated. End times for exposed premises: "<<std::endl;
	for (auto&e : ds.at("exp").farms){
		std::cout<<e->get_end("exp")<<", ";
	}
	std::cout<<std::endl;
}

	// store species for formatting later
	auto speciesMap = (*focalFarms.begin())->get_spCounts(); // just get species list from first focal farm
	for (auto& s:(*speciesMap)){species.emplace_back(s.first);}

	// Specify sequence of disease statuses and associated lag times
	// 1. Exposure to infectiousness
	statusShift exp_inf {"exp","inf",params.at("infectious")};
	diseaseSeq.emplace_back(exp_inf);
	// 2. Infectiouness to immunity (end time past simulation)
	statusShift inf_imm {"inf","imm",std::make_tuple(pastEndTime,0)};
	diseaseSeq.emplace_back(inf_imm);
std::cout << "Status manager initiated."<<std::endl;
}

Status_manager::~Status_manager()
{
	for (auto& f:changedStatus){delete f.second;}
}

void Status_manager::addPS(Farm* f)
{
	int fid = f->Farm::get_id();
	changedStatus[fid] = new Prem_status(f);
}

/// Changes disease status of a farm, and determines time when next disease status begins.
/// Adds newly exposed farms to the "not susceptible" list to pass to Grid_checker,
/// and begins control sequence (starting with "reported"). For all other disease statuses,
/// adds farm to appropriate status list with start and end times. If this is the first
/// farm listed for a particular status, set the placeholders for that status at the
/// beginning of the status list.
/// \param[in]	f	Farm for which status should be changed
///	\param[in]	startTime	Time at which this status begins (usually immediate)
/// \param[in]	status	Disease status to which this farm will be set
///	\param[in]	lag	Tuple of mean, variance of time lag in days until next disease status begins. For permanent statuses (i.e. immune), use mean=pastEndTime, var=0
/// \param[in]	index	Boolean: is this the first case that will be reported? If so, will use the delayed index case parameters
void Status_manager::setStatus(Farm* f, int startTime, std::string status, std::tuple<double,double> lag, bool index)
{
	int fid = f->Farm::get_id();
	if (changedStatus.count(fid)==0){addPS(f);} // if a prem_status object doesn't exist for this farm, make one

	if (status.compare("exp")==0){ // if farm is becoming exposed
		int fid = f->Farm::get_id();
		notSus.emplace_back(allFarms->at(fid)); // add to list of non-susceptibles
		// start control sequence
		control->addFarm(changedStatus.at(fid),startTime,index); // first indicates index report
	}

	// set start and end times for this status
	changedStatus.at(fid)->set_start(status,startTime);
	int endTime = startTime+normDelay(lag);
	changedStatus.at(fid)->set_end(status,endTime);
	changedStatus.at(fid)->set_diseaseStatus(status);
	// add to appropriate disease-status list
	bool firstOfStatus = 0;
		if (ds.count(status)==0){firstOfStatus=1;}

	ds[status].farms.emplace_back(changedStatus.at(fid));
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
		std::vector<Prem_status*>::iterator lo_it = ds.at(s.one).farms.begin();
		std::vector<Prem_status*>::iterator hi_it = ds.at(s.one).farms.begin();
		std::advance(lo_it, ds.at(s.one).lo); // move iterator to lo
		std::advance(hi_it, ds.at(s.one).hi); // move iterator to hi

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
		if (changedStatus.count(s->origID)==1){ // if origin farm has at least had any status change
			if (changedStatus.at(s->origID)->get_diseaseStatus().compare("inf")==0){ // and if origin farm is infectious
				if (!isWithin(allFarms->at(s->destID),notSus)){ // and if dest farm is susceptible
					s->transmission = 1; // overwrite default of 0
std::cout<<"Exposure via shipment"<<std::endl;
				}
			}
		}
		// check with control for shipping ban level
		s->ban = control->checkShipBan(s);
		if (s->ban < 3 // if anything other than compliant with a ban: no ban (0), not yet in effect (1) or non-compliant (2)
			&& s->transmission == 1){// and transmission is possible
			toExpose.emplace_back(allFarms->at(s->destID));
			sources[allFarms->at(s->destID)].emplace_back(std::make_tuple(allFarms->at(s->origID),1)); // 1 is shipping as method of exposure
		}
	}
	expose(toExpose, time);
}

void Status_manager::expose(std::vector<Farm*>& farms, int t)
{
	for (auto& f:farms){
		// check if farm is susceptible
		if (get_status(f).compare("sus")==0){
			setStatus(f,t,"exp",params["latency"]);
		}
	}
}

std::string Status_manager::get_status(Farm* f) const
{
	std::string output;
	int fid = f->Farm::get_id();
	if (changedStatus.count(fid)==0){output = "sus";
	} else {
		output = changedStatus.at(fid)->get_diseaseStatus();
	}
	return output;
}

void Status_manager::premsWithStatus(std::string s, std::vector<Farm*>& output2)
{ // get all farms with status s between iterators (as of last call to updates)
	std::vector<Farm*> output1;
	if (ds.count(s)==1){
		std::vector<Prem_status*>::iterator lo_it = ds.at(s).farms.begin() + ds.at(s).lo;
		std::vector<Prem_status*>::iterator hi_it = ds.at(s).farms.begin() + ds.at(s).hi +1;
		std::vector<Prem_status*> outputPS(lo_it, hi_it); // return farms in [lo, hi]

		for (auto f:outputPS){
			output1.emplace_back(allFarms->at(f->get_id()));
		}
	}
	output1.swap(output2);
}

int Status_manager::numPremsWithStatus(std::string s)
{ // get # farms with status s (as of last call to updates)
	int total = 0;
	if (s.compare("sus")==0){ total = nPrems - notSus.size();
	} else if (ds.count(s)==1){
		total = ds.at(s).hi - ds.at(s).lo +1;
	} // return number of farms in [lo, hi]
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
