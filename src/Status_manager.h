#ifndef Status_manager_h
#define Status_manager_h

#include "shared_functions.h"
#include "Control_actions.h"
#include "file_manager.h" // for Parameters struct
#include "Grid_manager.h"

#include <iterator> // for std::next
#include <utility> // for std::iter_swap

extern int verboseLevel;
class Farm;

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

///> 	Keeps track of disease and control statuses during a simulation.
/// 	Filters exposures from local and shipment spread with any control measures in place.
class Status_manager
{
	private:
		std::vector<Farm*> seededFarms;
		const Parameters* parameters;
		Grid_manager* grid; ///< Pointer to Grid object
		Control_actions* control; ///< Pointer to Control object
		const std::unordered_map<int, Farm*>* allPrems; ///< Pointer to actual Farm objects for interfacing with main
		unsigned int recentNotSus; ///< Placeholder for last farm that became not-susceptible during the last timestep
		int pastEndTime;
		int nPrems; ///< Total number of premises, needed to return number of susceptibles

		int verbose; ///< Can be set to override global setting for console output

		std::vector<statusShift> diseaseSeq; ///< Sequence of disease statuses and associated lag times
		std::unordered_map<std::string, diseaseStatus> ds; ///< Disease statuses with affected farms with start/end times
		std::vector<std::string> species;

		std::unordered_map<int, Prem_status*> changedStatus; ///< Premises that have had any disease or control status change
		std::vector<Farm*> notSus; ///< Farms that are in any disease state except susceptible (are not eligible for local spread exposure)
		
		std::unordered_map< Farm*, std::vector<std::tuple<Farm*, int>> > sources; ///< Exposed farm, source of infection, type of spread (0=local, 1=ship)

		void get_seedCos(std::vector<std::string>&);
		void setStatus(Farm*, int, std::string, std::tuple<double,double>);
		//void get_infCounties(std::vector<std::string>&);

	public:
		// initialize with arguments:
		// seedPool: vector of initially infectious farms, listed by ID
		// Parameters reference
		// Grid_manager reference
		// Control_actions reference
		Status_manager(std::vector<Farm*>&, const Parameters*, Grid_manager*, 
		  Control_actions*);
		~Status_manager();

		void addPS(Farm*); // add Prem_status object for use in Status, Control

		void updates(int t);

		void premsWithStatus(std::string, std::vector<Farm*>&); // returns vector of Prem_status*s with status
		int numPremsWithStatus(std::string); // get number of Prem_status*s with status
		int nextEvent(int); // not used?
		void localExposure(std::vector<Farm*>&, int); // check for control before exposure
		void shipExposure(std::vector<Shipment*>&, int); // check for control before exposure
		void expose(std::vector<Farm*>&, int);
		void newNotSus(std::vector<Farm*>&); //inlined
		std::unordered_map< Farm*, std::vector<std::tuple<Farm*, int>> >* get_sources(); // inlined - provides access for Grid_checker
		std::string get_status(Farm*) const;
		std::string formatRepSummary(int, int, double);
		std::string formatDetails(int, int);
};

inline std::unordered_map< Farm*, std::vector<std::tuple<Farm*, int>> >* Status_manager::get_sources()
{
	return &sources;
}

#endif // Status_manager_h
