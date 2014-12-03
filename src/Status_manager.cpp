// Status_manager.cpp
// 
// Status of all premises is susceptible by default, all statuses stored
// in the unordered_map "statusTimeFarms": first key is status (integer), second key
// is end time (the last day for which that status is valid), stored value is farm*
// Statuses are also stored with the farm object (only for current timepoint) for looking
// up the status of an individual premises.

#include "Status_manager.h"
#include "shared_functions.h"

Status_manager::Status_manager(std::string fname, std::tuple<double,double> params, std::unordered_map<int, Farm*>& allPrems, int endTime)
{
	// read in initially infected prems from file
		std::vector<Farm*> focalFarms, compFarms;
	 	int fID;
 		std::ifstream f(fname);
		if(!f){std::cout << "Input file not found." << std::endl;}
		if(f.is_open()){
		std::cout << "Loading seed farms." << std::endl;
			while(! f.eof()){
				std::string line;
				getline(f, line); // get line from file "f", save as "line"			
				if(! line.empty()){ // if line has something in it
					str_cast(line, fID);
					focalFarms.emplace_back(allPrems.at(fID));
				} // close "if line_vector not empty"
			} // close "while not end of file"
		} // close "if file is open"
		
		for (auto& p:allPrems){
			compFarms.emplace_back(p.second);}
		// removes focalFarms from compFarms, now compFarms only has susceptibles
 		removeFarmSubset(focalFarms,compFarms); 

	for (auto& c:compFarms){
		statusTimeFarms[0][endTime+1].emplace_back(c);
	}
	
	changeTo(2, focalFarms, 0, params);
	// change focalFarms' status to 2 (infectious), with durations via params, at base time 0
//	if (verbose){
		std::cout<<compFarms.size()<<" susceptible and "<<focalFarms.size()<<" infectious farms initiated. End times for infectious farms: "<<std::endl;
		for (auto&i : statusTimeFarms[2]){
			std::cout<<i.first<<": "<<i.second.size()<<" farms, ";}
//	}
}

Status_manager::~Status_manager()
{
}

/*
// specifically for changing from a susceptible status - removes farm from susceptible list 
// (otherwise its status would never expire)
void Status_manager::changeSusTo(int status, std::vector<Farm*>& toChange, int t, 
	std::tuple<double,double> params)
// status: 0=susceptible, 1=exposed, 2=infectious, 3=resistant/recovered/vaccinated
// toChange: vector of farms to apply this status to
// t: time at which status begins
// pair: mean & var: parameters of normal distribution to assign end time
{
	// for each farm to be exposed, draw an end time (last day this status is valid)
	double mean = std::get<0>(params);
	double var = std::get<1>(params);
	for (auto& f:toChange){
	// Change status in Farm object
		f->set_status(status);
	
	// Determine length of period drawn from normal distribution
		double normDraw = norm_rand()*var + mean; // scaled to # drawn from N(0,1)
		int draw = (int)(normDraw+0.5); // round to nearest day
			if(draw<1){draw = 1;} // at minimum one day in a status
	// ... and add to status map  
		statusTimeFarms[status][t+draw].emplace_back(f);
	
	// Remove from list of susceptibles
		std::unordered_map<int,std::vector<Farm*>>& allSus = statusTimeFarms.at(0);
		std::vector<Farm*> susFarms;
		for (auto& sf:allSus){susFarms = sf;}
		removeFarmSubset(toChange,susFarms);
		
	}
	
}
*/

void Status_manager::changeTo(int status, std::vector<Farm*>& toChange, int t, 
	std::tuple<double,double> params)
// status: 0=susceptible, 1=exposed, 2=infectious, 3=resistant/recovered/vaccinated
// toChange: vector of farms to apply this status to
// t: time at which status begins
// pair: mean & var: parameters of normal distribution to assign end time
{
	// for each farm to be exposed, draw an end time (last day this status is valid)
	double mean = std::get<0>(params);
	double var = std::get<1>(params);
	for (auto& f:toChange){
	// Change status in Farm object
		f->set_status(status);
	
		// determine length of period drawn from normal distribution
		double normDraw = norm_rand()*var + mean; // scaled to # drawn from N(0,1)
		int draw = (int)(normDraw+0.5); // round to nearest day
			if(draw<1){draw = 1;} // at minimum one day in a status
	// Add to status map  
		statusTimeFarms[status][t+draw].emplace_back(f);
	}
	// if farms are being changed to exposed, remove them from susceptible list
	if (status == 1){ 
		removeFarmSubset(toChange,(*statusTimeFarms[0].begin()).second);
		// statusTimeFarms[0] should only have one map since they all have the same end time, 1+ the total runtime.
		// begin() points to this one map, and second gets us to the vector of farms.
	}
}

// overloaded version accepts a fixed end time (used for -1 = indefinite)
void Status_manager::changeTo(int status, std::vector<Farm*>& toChange, int t, 
	int endTime)
{
	for (auto& f:toChange){
		f->set_status(status);
		statusTimeFarms[status][endTime].emplace_back(f);
	}
	// if farms are being changed to exposed, remove them from susceptible list
	if (status == 1){ 
		removeFarmSubset(toChange,(*statusTimeFarms[0].begin()).second);
		// statusTimeFarms[0] should only have one map since they all have the same end time, 1+ the total runtime.
		// begin() points to this one map, and second gets us to the vector of farms.
	}
}

std::vector<Farm*> Status_manager::allWithStatus(int s, int t)
{ // get all farms with status s at time t
	std::vector<Farm*> output;
	
	std::unordered_map< int,std::vector<Farm*> >& status_s = statusTimeFarms.at(s); 
	// step through map of times/farms
	for (auto& st:status_s){
		if(st.first >= t){
			for (auto& f:st.second){
			output.emplace_back(f);} // add farm* to output
		}	
	} // end for each time/farm in this status
 return output;
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
