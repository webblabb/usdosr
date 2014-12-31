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

extern bool verbose;

class Status_manager
{
	private:
		// maps with nested keys: status, status end time, value: vector of Farm*s/FIPS
		// time index makes it faster to look up current statuses
		std::unordered_map< std::string,std::unordered_map<int,std::vector<Farm*>> > statusTimeFarms;
		// status keys: sus, exp, inf, imm, vax, cull, exp2 (to be reported)
		std::unordered_map< std::string,std::unordered_map<std::string, int> > statusFIPSTime;
		// status keys: reported, banOrdered, banCompliant

		std::unordered_map<std::string, std::tuple<double,double>> params;
		// map of mean/variance of lag times for ["latency"]exposed-infectious, 
		// ["infectious"]infectious-recovered, ["report"]exposed-reported, 
		// ["startBan"]reported-banned, ["complyBan"]banned-compliant...
		int pastEndTime;
		// time past the end of the simulation, to assign to permanent status end times
		
//		int normDelay(double mean, double variance);
		int normDelay(std::tuple<double,double>);
		// tuple contains mean and variance
		
	public:
		Status_manager(std::string, std::unordered_map<std::string, std::tuple<double,double>>, 
			std::unordered_map<int, Farm*>&, int);
			
		~Status_manager();		
		
		void changeTo(std::string status, std::vector<Farm*>&toChange, int t, std::tuple<double,double> params);
		void changeTo(std::string status, std::vector<Farm*>&toChange, int endTime);

		void updates(int t);
		
		std::vector<Farm*> premsWithStatus(std::string, int); // get vector of Farm*s with status @ time
		std::vector<std::string> FIPSWithStatus(std::string, int); // get vector of FIPS with status @ time

		void printVector(std::vector<Farm*>&, std::string&) const; // print vector to named file
		void pickInfAndPrint(double, std::unordered_map<int, Farm*>&, std::string); // randomly generate a proportion of infected farms & print to file

//		std::unordered_map< int,std::vector<Farm*> > getReported() const; //not required for run, just informative
//		std::vector<std::string> getBannedFIPS() const;
};

// inline std::unordered_map< int,std::vector<Farm*> > Status_manager::getReported() const
// {
// 	return statusTimeFarms.at(6);
// }
#endif