// Status_manager.cpp
// 
// Status of all premises is susceptible by default, all statuses stored
// in the unordered_map "statusTimeFarms": first key is status (integer), second key
// is end time (the last day for which that status is valid), stored value is farm*
// Statuses are also stored with the farm object (only for current timepoint) for looking
// up the status of an individual premises.
// [] corresponds to "reported", which is not stored with the Farm object (only actual disease states are stored with the Farm object)
// when the corresponding time matches with the time in the simulation, the FIPS of that farm is added to the banned list (combine and unique func)

#include "Status_manager.h"
#include "shared_functions.h"

Status_manager::Status_manager(std::string fname, /*int whichSeed,*/ std::unordered_map<std::string, 
	std::tuple<double,double>> in_params, std::unordered_map<int, Farm*>& allPrems, 
	int endTime)
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
	params = in_params;
	pastEndTime = endTime+100;
	// initialize all farms as susceptible, with end time after end of run time
	for (auto& ap:allPrems){
		statusTimeFarms["sus"][pastEndTime].emplace_back(ap.second);
		ap.second->set_status("sus"); // set Farm object status to sus
		// put each under status key sus, time key 1+end
	}
	// read in initially infected prems from file
		std::vector<Farm*> focalFarms;
	 	int fID;
 		std::ifstream f(fname);
		if(!f){std::cout << "Input file not found." << std::endl;}
		if(f.is_open()){
		if(verbose){std::cout << "Loading seed prems." << std::endl;}
			while(! f.eof()){
				std::string line;
				getline(f, line); // get line from file "f", save as "line"			
				if(! line.empty()){ // if line has something in it
					str_cast(line, fID);
					focalFarms.emplace_back(allPrems.at(fID));
				} // close "if line_vector not empty"
			} // close "while not end of file"
		} // close "if file is open"
		if(verbose){std::cout << "Closed seed file." << std::endl;}
/*	
	// if choosing random farms from list
	if (whichSeed>0){
		if (
	}
*/	
	changeTo("inf", focalFarms, 0, params["infectious"]);
	// change focalFarms' status to inf, with durations via params, at base time 0
	// also removes these farms from susceptible list
	if (verbose){
		std::cout<<focalFarms.size()<<" infectious prems initiated. End times for infectious prems: "<<std::endl;
		for (auto&i : statusTimeFarms["inf"]){
			std::cout<<i.first<<": "<<i.second.size()<<" prems, ";}
	}
}

Status_manager::~Status_manager()
{
}

// int Status_manager::normDelay(double mean, double var)
// // determine length of period drawn from normal distribution
// {
// 	double normDraw = norm_rand()*var + mean; // scaled to # drawn from N(0,1)
// 	int draw = (int)(normDraw+0.5); // round to nearest day
// 	if(draw<1){draw = 1;}
// 	return draw;
// }

int Status_manager::normDelay(std::tuple<double, double> params)
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
	std::vector<Farm*> toRemoveFromSus;
	for (auto& f:toChange){
	// Change status in Farm object
		if(f->get_status()=="sus"){toRemoveFromSus.emplace_back(f);}
		f->set_status(status);
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
	// if any farms were susceptible before the change, remove them from susceptible list
	if (toRemoveFromSus.size()>0){ 
		removeFarmSubset(toRemoveFromSus,(statusTimeFarms.at("sus").at(pastEndTime)));
		// statusTimeFarms["sus"] should only have one map, all with the same end time (1+ the total runtime).
	}
}

// overloaded version accepts a fixed end time
// used for permanent statuses such as imm=immune/recovered, vax=vaccinated, cull=culled
void Status_manager::changeTo(std::string status, std::vector<Farm*>& toChange, int endTime)
{
	if (status=="sus" || status =="exp"){
		std::cout<<"Warning: using fixed end time for non-permanent status "<<status<<std::endl;}
	// warning output if status is not one of the permanent ones
	std::vector<Farm*> toRemoveFromSus;
	for (auto& f:toChange){
		if(f->get_status()=="sus"){toRemoveFromSus.emplace_back(f);}
		f->set_status(status);
		statusTimeFarms[status][endTime].emplace_back(f);
	}
	// if any farms were susceptible before the change, remove them from susceptible list
	if (toRemoveFromSus.size()>0){ 
		removeFarmSubset(toRemoveFromSus,(statusTimeFarms.at("sus").at(pastEndTime)));
		// statusTimeFarms["sus"] should only have one map, all with the same end time (1+ the total runtime).
	}
}

std::vector<Farm*> Status_manager::premsWithStatus(std::string s, int t)
{ // get all farms with status s at time t
	std::vector<Farm*> output;

	if (statusTimeFarms.count(s)!=0){
	std::unordered_map< int,std::vector<Farm*> >& status_s = statusTimeFarms.at(s); 
	// step through map of times/farms
	for (auto& st:status_s){
		if(t < st.first){ // st.first is time at which next stage starts
			for (auto& f:st.second){
			output.emplace_back(f);} // add farm* to output
		}	
	} // end for each time in this status
	}
 return output;
}

std::vector<std::string> Status_manager::FIPSWithStatus(std::string s, int t)
{ // get all FIPS with status s at time t
	std::vector<std::string> output;
	
	if (statusFIPSTime.count(s)!=0){
	std::unordered_map< std::string,int >& status_s = statusFIPSTime.at(s); 
	// step through map of times/farms
	for (auto& st:status_s){
		if(t < st.second){ // st.second is time at which next stage starts
			output.emplace_back(st.first); // add FIPS to output
		}	
	} // end for each FIPS in this status
	}
 return output;
}

void Status_manager::updates(int t)
{
	if (verbose){std::cout<<"Updating farm statuses. ";}
// i. Reported farms->FIPS are assigned a ban start time
	if (statusTimeFarms.count("exp2")!=0){ // if there are any reported farms
	// Check the other "exposed" list with end time being the day of reporting
	std::unordered_map<int,std::vector<Farm*>>& reportedFarms = statusTimeFarms.at("exp2");
	// if there are premises at t (t is the day they are reported)
	if(reportedFarms.size()>0 && reportedFarms.count(t)==1){ 
		std::vector<Farm*>& farmsToReport = reportedFarms.at(t);
		// get FIPS for each farm that's reported at this time
		for (auto& ftr:farmsToReport){
			std::string farmFIPS = ftr->Farm::get_fips();
			// check if FIPS already has a ban time. if not, assign ban time
			if (statusFIPSTime["reported"].count(farmFIPS)==0){
				int banLag = normDelay(params["startBan"]); // time at which ban will be ordered
				statusFIPSTime["reported"][farmFIPS] = t+banLag;
//				if(verbose){std::cout<<FIPSWithStatus("reported",t).size()<<" reported FIPS. ";}
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
//			if(verbose){std::cout<<FIPSWithStatus("banOrdered",t).size()<<" ban-ordered FIPS. ";}
		}
	}
	}
	
// iii. Ban-ordered FIPS become compliant
	if (statusFIPSTime.count("banOrdered")!=0){ // if there are any ban-ordered FIPS
	for (auto& bf:statusFIPSTime.at("banOrdered")){
	// bf.first is FIPS, bf.second is first day of ban
		if (bf.second == t){ // if today is the first day of ban compliance
			statusFIPSTime["banActive"][bf.first] = pastEndTime;
//			if(verbose){std::cout<<FIPSWithStatus("banCompliant",t).size()<<" ban-compliant FIPS. ";}
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
//	if(verbose){std::cout<<premsWithStatus("inf",t).size()<<" infectious premises. ";}
	}
	}
	
// v. Infectious premises recover
	if (statusTimeFarms.count("inf")!=0){ // if there are any infectious farms
	std::unordered_map<int,std::vector<Farm*>>& infPrems = statusTimeFarms.at("inf");
	// if there are infectious farms at t (t is their last day of infectiousness)
	if(infPrems.size()>0 && infPrems.count(t)==1){ 
		std::vector<Farm*>& infToRecovered = infPrems.at(t);
		changeTo("imm", infToRecovered, pastEndTime); 
		// change these infectious premises to immune, with end time past end
//		if(verbose){std::cout<<premsWithStatus("imm",t).size()<<" immune premises. ";}
	}
	}
}

//not tested
void Status_manager::printVector(std::vector<Farm*>& vec, std::string& fname) const
// temporarily disabled due to incompatible std::to_string use
{
	std::string tabdelim;
	for(auto& it:vec){
		double fid = it->Farm::get_id();
		tabdelim += std::to_string(fid);
		tabdelim += "\n";
	}
	
	std::ofstream f(fname); 
	if(f.is_open()){
		f << tabdelim;
		f.close();
	}
	std::cout << "Vector printed to " << fname <<std::endl;
}

void Status_manager::pickInfAndPrint(double propFocal, 
	std::unordered_map<int, Farm*>& farm_map, std::string fname)
// input is proportion of focal farms (random), map of all farms, fname is file name to print to
{ 
 	std::vector <Farm*> focal, comp; // two vectors of focal/comp farms

	for (auto& i:farm_map){
		double randomnum = unif_rand();
		if (randomnum <= propFocal){
			focal.emplace_back(i.second);
		} else {
			comp.emplace_back(i.second);
		}
	}
	printVector(focal,fname);
 }
/*
std::vector<Farm*> Status_manager::sumPremsWithStatus(std::string s, int t)
{ // get all farms with status s at and before time t
	std::vector<Farm*> output;

	if (statusTimeFarms.count(s)!=0){
	std::unordered_map< int,std::vector<Farm*> >& status_s = statusTimeFarms.at(s); 
	// step through map of times/farms
	for (auto& st:status_s){
		if(t < st.first){ // st.first is time at which next stage starts
			for (auto& f:st.second){
			output.emplace_back(f);} // add farm* to output
		}	
	} // end for each time in this status
	}
 return output;
}
*/