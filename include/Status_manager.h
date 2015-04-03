// Status_manager.h
//
// Manages changes in premises infection status
// Keeps track of infection times for statuses other than susceptible
// Gets called at each timepoint

#ifndef Status_manager_h
#define Status_manager_h

// included in Farm.h: string, unordered map, vector
#include "Farm.h"

#include <fstream>
#include <tuple>

extern int verboseLevel;

class Status_manager
{
	private:
		int verbose;
		// map with nested keys: status, status END time, value: vector of Farm*s
		// status keys: sus, exp, inf, imm, vax, cull, exp2 (to be reported)
		// time index makes it faster to look up current statuses
		std::unordered_map< std::string,std::unordered_map<int,std::vector<Farm*>> > statusTimeFarms;
		// map with nested keys: status, FIPS, status START time
		std::unordered_map< std::string,std::unordered_map<std::string, int> > statusFIPSTime;
		// status keys: reported, banOrdered, banCompliant
		std::vector<Farm*> notSus;
		// all farms that are not susceptible as of last status update
		std::vector<Farm*> allNotSus;
		// all farms that are not susceptible in this replicate


		std::unordered_map<std::string, std::tuple<double,double>> params;
		// map of mean/variance of lag times for ["latency"]exposed-infectious, 
		// ["infectious"]infectious-recovered, ["report"]exposed-reported, 
		// ["startBan"]reported-banned, ["complyBan"]banned-compliant...
		// time past the end of the simulation, to assign to permanent status end times
		std::vector<std::string> species; // for formatting output
		
		int normDelay(std::tuple<double,double>);
		// tuple contains mean and variance
		
		int pastEndTime;
		int totalNotSus;
		int nPrems;
		
		std::vector<Farm*> seededFarms;
		std::unordered_map< Farm*, std::vector<std::tuple<Farm*, int>> > sources;
		
	public:
		Status_manager(std::string, int, std::unordered_map<std::string, std::tuple<double,double>>&, 
			const std::unordered_map<int, Farm*>*, int);
			
		~Status_manager();		
		
		void changeTo(std::string status, std::vector<Farm*>&toChange, int t, std::tuple<double,double> params);
		void changeTo(std::string status, std::vector<Farm*>&toChange, int endTime);

		void updates(int t);
		
		void premsWithStatus(std::string, int, std::vector<Farm*>&); // get vector of Farm*s with status @ time
		int numpremsWithStatus(std::string, int); // get number of Farm*s with status @ time
		int numFIPSWithStatus(std::string, int); // get number of FIPS with status @ time

};


#endif