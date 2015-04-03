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

Status_manager::Status_manager(std::string fname, int numRandomSeed, 
	std::unordered_map<std::string, std::tuple<double,double>>& in_params, 
	const std::unordered_map<int, Farm*>* allPrems, int endTime)
	:
	params(in_params),
	pastEndTime(endTime+100),
	nPrems(allPrems->size())
// initialize with arguments:
// fname: file where seed (initially infectious) farms are listed by ID
// whichSeed: number of random farms to draw from file (0 uses all)
// params: vector of tuples containing mean and variance of delays:
// ...between exposure and infectiousness onset ["latency"]
// ...between infectiousness onset and recovery ["infectious"]
// ...between exposure and reporting ["report"]
// ...between reporting and shipment ban ["startBan"]
// ...between shipment ban and compliance ["complyBan"]
// allPrems: reference map of all other premises
// endTime: last timestep of the simulation (to set temporarily static statuses)
{
	verbose = verboseLevel;
	
	// read in initially infected prems from file
	std::vector<Farm*> seedFarms;
	int fID;
	std::ifstream f(fname);
	if(!f){std::cout << "Input file not found. Exiting..." << std::endl; exit(EXIT_FAILURE);}
	if(verbose>0){std::cout << "Loading seed prems.";}
		while(! f.eof()){
			std::string line;
			getline(f, line); // get line from file "f", save as "line"			
			if(! line.empty()){ // if line has something in it
				str_cast(line, fID);
				seedFarms.emplace_back(allPrems->at(fID));
			} // close "if line_vector not empty"
		} // close "while not end of file"
	if(verbose>0){std::cout << " Closed seed file." << std::endl;}
	
	std::vector<Farm*> focalFarms;
	// if choosing random farms from list
	if (numRandomSeed!=0){
		random_unique(seedFarms, numRandomSeed, focalFarms);
		if (verbose>0){std::cout<<focalFarms.size()<<" initial premises infections."<<std::endl;}
	} else {
		focalFarms.swap(seedFarms);
	}
	seededFarms = focalFarms;
	
	changeTo("inf", focalFarms, 1, params["infectious"]);
	// change focalFarms' status to inf, with durations via params, at base time 1
	// also marks these farms as "not susceptible"
	if (verbose>1){
		std::cout<<focalFarms.size()<<" infectious prems initiated. End times for infectious prems: "<<std::endl;
		for (auto&i : statusTimeFarms["inf"]){
			std::cout<<i.first<<": "<<i.second.size()<<" prems, ";}
	}
	// add different lag for index reports
	int indexLag = 2; // for example (to make external)
	for (auto& f:focalFarms){
		statusTimeFarms["exp2"][indexLag].emplace_back(f);}
	
	// store species for formatting later
	auto speciesMap = allPrems->at(fID)->get_spCounts(); // just get species list from last seed farm accessed
	for (auto& s:(*speciesMap)){species.emplace_back(s.first);}
}

Status_manager::~Status_manager()
{
}

int Status_manager::normDelay(std::tuple<double, double>& params)
// determine length of period drawn from normal distribution
{
	double mean = std::get<0>(params);
	double var = std::get<1>(params);
	double normDraw = norm_rand()*var + mean; // scaled to # drawn from N(0,1)
	int draw = (int)(normDraw+0.5); // round up to nearest day
	if(draw<1){draw = 1;}
	return draw;
}

void Status_manager::changeTo(std::string status, std::vector<Farm*>& toChange, int t, 
	std::tuple<double,double> delayParams)
// status: sus=susceptible, exp=exposed, inf=infectious
// toChange: vector of farms to apply this status to
// t: time at which status begins
// delayParams: mean & var: parameters of normal distribution to assign end time (no variance provided means fixed @ mean)
// if changing to exposed, adds to countdown for reporting
{
	// for each farm to be exposed, draw an end time (last day this status is valid)
	if (status=="sus"){std::cout<<"Cannot assign sus status. Exiting..."<<std::endl; exit(EXIT_FAILURE);}
	for (auto& f:toChange){
	// Add to not-susceptible list if not already there
		if(!isWithin<Farm*>(f,allNotSus)){
			notSus.emplace_back(f);
			allNotSus.emplace_back(f);
			}
	// Add to status map		
		int draw = normDelay(delayParams);  
		statusTimeFarms[status][t+draw].emplace_back(f);
	// If becoming exposed, also add to exp2, with end time when farms become reported		
		if(status=="exp"){
			int repLag = normDelay(params["report"]);
			// params["report"] contains exposure -> reported lag
			statusTimeFarms["exp2"][t+repLag].emplace_back(f);
			// status exp2 = going to be reported - don't record with Farm objects (would interfere w/disease progression statuses)
		}
	}
}

// overloaded version accepts a fixed end time
// used for permanent statuses such as imm=immune/recovered, vax=vaccinated, cull=culled
void Status_manager::changeTo(std::string status, std::vector<Farm*>& toChange, int endTime)
{
	if (status=="sus"){std::cout<<"Cannot assign sus status. Exiting..."<<std::endl; exit(EXIT_FAILURE);}
	if (status=="exp" || status =="inf"){
		std::cout<<"Warning: using fixed end time for non-permanent status "<<status<<std::endl;}
	// warning output if status is not one of the permanent ones
	for (auto& f:toChange){
	// Add to not-susceptible list if not already there
		if(!isWithin<Farm*>(f,allNotSus)){
			notSus.emplace_back(f);
			allNotSus.emplace_back(f);
			}
		statusTimeFarms[status][endTime].emplace_back(f);
	}
}

void Status_manager::premsWithStatus(std::string s, int t, std::vector<Farm*>&output1)
{ // get all farms with status s at time t
	std::vector<Farm*> output;

	if (s=="sus"){
		{std::cout<<"Cannot retrieve susceptible farms via Status Manager. Exiting..."<<std::endl; exit(EXIT_FAILURE);}
	} else if (statusTimeFarms.count(s)!=0){
	std::unordered_map< int,std::vector<Farm*> >& status_s = statusTimeFarms.at(s); 
	// step through map of times/farms
	for (auto& st:status_s){
		if(t < st.first){ // st.first is time at which next stage starts
			for (auto& f:st.second){
			output.emplace_back(f);} // add farm* to output
		}	
	} // end for each time in this status
	}
 output.swap(output1);
}

int Status_manager::numPremsWithStatus(std::string s, int t)
{ // get # farms with status s at time t
	int total = 0;

	if (s=="sus"){
		total = nPrems - allNotSus.size();
	} else if (statusTimeFarms.count(s)!=0){
	std::unordered_map< int,std::vector<Farm*> >& status_s = statusTimeFarms.at(s); 
	// step through map of times/farms
	for (auto& st:status_s){
		if(t < st.first){ // st.first is time at which next stage starts
			total += st.second.size();
		}	
	} // end for each time in this status
	}
 return total;
}

int Status_manager::numFIPSWithStatus(std::string s, int t)
{ // get all FIPS with status s at time t
	int total = 0;
	
	if (s=="sus"){
		{std::cout<<"Cannot retrieve susceptible FIPS via Status Manager. Exiting..."<<std::endl; exit(EXIT_FAILURE);}
	} else if (statusFIPSTime.count(s)!=0){
		std::unordered_map< std::string,int >& status_s = statusFIPSTime.at(s); //i.e. statusFIPStime["reported"]
		// step through map of times/farms
		for (auto& st:status_s){
			if(t > st.second){total++;}	
		} // end for each FIPS in this status
	}
 return total;
}

void Status_manager::updates(int t)
{
	if (verbose>1){std::cout<<"Updating farm statuses. ";}
// i. Reported farms->FIPS are assigned a ban start time
	// Check the other "exposed" list with end time being the day of reporting
	if (statusTimeFarms.count("exp2")!=0){ // if there are any reported farms
	std::unordered_map<int,std::vector<Farm*>>& reportedFarms = statusTimeFarms.at("exp2");
	// if there are premises at t (t is the day they are reported)
	if(reportedFarms.count(t)==1){ 
		if (verbose>1){std::cout<<"There are reported farms at this time. ";}
		std::vector<Farm*>& farmsToReport = reportedFarms.at(t);
		// get FIPS for each farm that's reported at this time
		for (auto& ftr:farmsToReport){
			std::string farmFIPS = ftr->Farm::get_fips();
			// check if FIPS already has a ban time. if not, assign ban time
			if (statusFIPSTime["reported"].count(farmFIPS)==0){
				int banLag = normDelay(params["startBan"]); // time at which ban will be ordered
				statusFIPSTime["reported"][farmFIPS] = t+banLag;
				if (verbose>2){std::cout<<"Adding FIPS to ban list, time "<<t+banLag<<". ";}

			} 
		}
	}
	}
	
// ii. Order ban for reported FIPS, add time lag until compliance
	if (statusFIPSTime.count("reported")!=0){ // if there are any reported FIPS
	for (auto& rf:statusFIPSTime.at("reported")){
	// rf.first is FIPS, rf.second is first day of ban
		if (rf.second == t){ // if today is the day the ban is ordered
			int compLag = normDelay(params["complyBan"]); 
			statusFIPSTime["banOrdered"][rf.first] = t+compLag;
		}
	}
	}
	
// iii. Ban-ordered FIPS become compliant
	if (statusFIPSTime.count("banOrdered")!=0){ // if there are any ban-ordered FIPS
	for (auto& bf:statusFIPSTime.at("banOrdered")){
	// bf.first is FIPS, bf.second is first day of ban
		if (bf.second == t){ // if today is the first day of ban compliance
			statusFIPSTime["banActive"][bf.first] = t;
		}
	}
	}
	
// iv. Exposed farms become infectious
	if (statusTimeFarms.count("exp")!=0){ // if there are any exposed farms
	std::unordered_map<int,std::vector<Farm*>>& expFarms = statusTimeFarms.at("exp");
	// if there are exposed farms at t (t is their last day of latency)
	if(expFarms.size()>0 && expFarms.count(t)==1){ 
		std::vector<Farm*>& expToInf = expFarms.at(t);
		changeTo("inf", expToInf, t, params["infectious"]); 
		// change these exposed premises to infectious (status 2), with end time according to "infectious" parameters
	}
	}
	
// v. Infectious premises recover
	if (statusTimeFarms.count("inf")!=0){ // if there are any infectious farms
	std::unordered_map<int,std::vector<Farm*>>& infPrems = statusTimeFarms.at("inf");
	// if there are infectious farms at t (t is their last day of infectiousness)
	if(infPrems.size()>0 && infPrems.count(t)>0){ 
		std::vector<Farm*>& infToRecovered = infPrems.at(t);
		changeTo("imm", infToRecovered, pastEndTime); 
		// change these infectious premises to immune, with end time past end
	}
	}
}