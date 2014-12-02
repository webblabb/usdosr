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

class Status_manager
{
	private:
	// map with nested keys: status, status end time, value: vector of Farm*s
		std::unordered_map< int,std::unordered_map<int,std::vector<Farm*>> > statusTimeFarms;
		// extra time index allows us to skip over "expired" statuses
		
	public:
		Status_manager(std::string, std::tuple<double,double>, std::unordered_map<int, Farm*>&, int);
			
		~Status_manager();
	
		void changeTo(int status,std::vector<Farm*>&toChange,int t,std::tuple<double,double> params);
		void changeTo(int status,std::vector<Farm*>&toChange,int t,int endTime);

		std::vector<Farm*> allWithStatus(int, int); // get vector of Farm*s with status @ time

		void printVector(std::vector<Farm*>&, std::string&) const; // print vector to named file
		void pickInfAndPrint(double, std::unordered_map<int, Farm*>&, std::string); // randomly generate a proportion of infected farms & print to file

};

#endif