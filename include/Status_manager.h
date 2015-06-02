///
///		\class Status_manager
///
///		Keeps track of disease statuses during a simulation.

#ifndef Status_manager_h
#define Status_manager_h

#include "shared_functions.h"
#include "Control_actions.h"

#include <iterator> // for std::next
#include <utility> // for std::iter_swap

extern int verboseLevel;

/// List of all farms that have/had a given disease status, with placeholders to indicate current validity
struct diseaseStatus 
{
	std::vector<Prem_status*> farms;///< List of all farms with this status, in chronological order of start time
	unsigned int lo; ///< Before this placeholder, end times are less than t (have already passed)
	unsigned int hi; ///< After this placeholder, start times are greater than t (have not started yet)
};

/// Defines transitions from one disease status to the next
struct statusShift
{
	std::string one; ///< Preceding disease status
	std::string two; ///< Following disease status
	std::tuple<double,double> lagToTwo; ///< Mean and variance of lag time from old to new
};

class Status_manager
{
	private:
		int verbose;
		
		std::vector<statusShift> diseaseSeq; ///< Sequence of disease statuses and associated lag times
		std::unordered_map<std::string, std::tuple<double,double>> params; ///< Map of mean/variance of times spent in ["latency"]: exposed-to-infectious, and ["infectious"]: infectious-to-recovered 
		std::unordered_map<std::string, diseaseStatus> ds; ///< Disease statuses with affected farms with start/end times
		std::vector<std::string> species;
		
		Control_actions* control; ///< Pointer to Control object
		const std::unordered_map<int, Farm*>* allFarms; ///< Pointer to non-status Farm objects for interfacing with main
		
		std::unordered_map<int, Prem_status*> changedStatus; ///< Premises that have had any disease or control status change
		std::vector<Farm*> notSus; ///< Farms that are in any disease state except susceptible
		unsigned int recentNotSus; ///< Placeholder for last farm that became not-susceptible during the last timestep
		
		int pastEndTime;
		int nPrems; ///< Total number of premises, needed to return number of susceptibles
		std::vector<Farm*> seededFarms;
		std::unordered_map< Farm*, std::vector<std::tuple<Farm*, int>> > sources; ///< Exposed farm, source of infection, type of spread (0=local, 1=ship)
		
		void get_seedCos(std::vector<std::string>&);
		void setStatus(Farm*, int, std::string, std::tuple<double,double>, bool first=0);
		void get_infCounties(std::vector<std::string>&);

	public:
		// initialize with arguments:
		// fname: file where seed (initially infectious) farms are listed by ID
		// whichSeed: number of random farms to draw from file (0 uses all)
		// params: vector of tuples containing mean and variance of delays:
		// ...between exposure and infectiousness onset ["latency"]
		// ...between infectiousness onset and recovery ["infectious"]
		// allPrems: reference map of all other premises
		// endTime: last timestep of the simulation (to set temporarily static statuses)
		Status_manager(std::vector<Farm*>&, int, std::unordered_map<std::string, std::tuple<double,double>>&, 
			const std::unordered_map<int, Farm*>*, int, Control_actions*);
			
		~Status_manager();		
		
		void addPS(Farm*); // add Prem_status object for use in Status, Control

		void updates(int t);
		
		void premsWithStatus(std::string, std::vector<Farm*>&); // returns vector of Prem_status*s with status
		int numPremsWithStatus(std::string); // get number of Prem_status*s with status
		
		int nextEvent(int);
		void localExposure(std::vector<Farm*>&, int);
		void shipExposure(std::vector<shipment*>&, int);
		void expose(std::vector<Farm*>&, int);
		
		void newNotSus(std::vector<Farm*>&); //inlined
		std::unordered_map< Farm*, std::vector<std::tuple<Farm*, int>> >* get_sources(); // inlined - provides access for Grid_checker
		std::string get_status(Farm*) const;

		std::string formatRepSummary(int, int, double);
		std::string formatDetails(int, int);
};
	
inline std::unordered_map< Farm*, std::vector<std::tuple<Farm*, int>> >* Status_manager::get_sources(){
	return &sources;}
	
#endif
