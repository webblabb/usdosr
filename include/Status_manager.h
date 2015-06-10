// Status_manager.h
//
// Manages changes in premises infection status
// Keeps track of infection times for statuses other than susceptible
// Gets called at each timepoint

#ifndef Status_manager_h
#define Status_manager_h

#include "shared_functions.h"
#include "Control_actions.h"

#include <utility> // std::iter_swap
#include <set> // for susCounties, eventTimes

extern int verboseLevel;
class Farm;

struct diseaseStatus
{
	std::vector<Farm*> farms;
	unsigned int lo; // before this placeholder, end times are less than t - passed
	unsigned int hi; // after this placeholder, start times are greater than t - not started yet
};

// To define transitions from one disease status to the next
struct statusShift
{
	std::string one; // old disease status
	std::string two; // new disease status
	std::tuple<double,double> lagToTwo; // mean and variance of lag time from old to new
};

class Status_manager
// makes new copies of Farms on which to change statuses
{
	private:
		int verbose;

		std::vector<statusShift> diseaseSeq; // Sequence of disease statuses and associated lag times
		std::unordered_map<std::string, std::tuple<double,double>> params;
		// map of mean/variance of times spent in
		// ["latency"] exposed-to-infectious,
		// ["infectious"] infectious-to-recovered
		std::unordered_map<int, Farm*> allNotSus; // farms that have had any status change (so not susceptible) in this replicate
		std::vector<Farm*> notSus; // pointers to farms that became not-susceptible during the last timestep
		std::unordered_map<std::string, diseaseStatus> ds; // disease statuses with affected farms with start/end times
		std::vector<std::string> species;

		Control_actions* control; // pointer to Control object
		const std::unordered_map<int, Farm*>* allFarms;// pointer to allFarms for checking shipments

		int pastEndTime;
		int nPrems; // needed to return number of susceptibles
		std::vector<Farm*> seededFarms;
		std::unordered_map< Farm*, std::vector<std::tuple<Farm*, int>> > sources; // exposed farm, source of infection, type of spread (0=local, 1=ship)
//		std::set<std::string> susCounties; // list of counties that are susceptible (chose set for insertion of unique values, easy check for presence/absence, removal from middle)
		// Grid_checker and Shipment_manager write to this (not implemented yet)
 		std::set<int> eventTimes; // any time a status change is scheduled to occur

		void get_seedCos(std::vector<std::string>&);
		void setStatus(Farm*, int, std::string, std::tuple<double,double>, bool first=0);
		void get_infCounties(std::vector<std::string>&);

	public:
		Status_manager(std::vector<Farm*>&, int, std::unordered_map<std::string, std::tuple<double,double>>&,
			const std::unordered_map<int, Farm*>*, int, Control_actions*);

		~Status_manager();

		void updates(int t);

		void premsWithStatus(std::string, std::vector<Farm*>&); // get vector of Farm*s with status
		int numPremsWithStatus(std::string); // get number of Farm*s with status

		int nextEvent(int);
		void localExposure(std::vector<Farm*>&, int);
		void shipExposure(std::vector<Shipment*>&, int);
		void expose(std::vector<Farm*>&, int);

		void take_notSus(std::vector<Farm*>&); //inlined
		std::unordered_map< Farm*, std::vector<std::tuple<Farm*, int>> >* get_sources(); // inlined - provides access for Grid_checker

		std::string formatRepSummary(int, int, double);
		std::string formatDetails(int, int);
};

inline void Status_manager::take_notSus(std::vector<Farm*>& output){
	notSus.swap(output);} // most recently not susceptible

inline std::unordered_map< Farm*, std::vector<std::tuple<Farm*, int>> >* Status_manager::get_sources(){
	return &sources;}

inline int Status_manager::nextEvent(int t){ // return time of next event after t
	return *eventTimes.upper_bound(t);}

#endif
