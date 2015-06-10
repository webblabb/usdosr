// Control_rules.h
// This is where rules for control actions are specified and control progression is tracked.
// For example,
//  - cull all premises within a 10 km radius of any infected premises
//  - ban all shipments from the county or state of an infected premises
//  - comply with bans, culls, etc. at a given percentage
//  - vaccinated farms transmit with reduced efficiency

// Types of control
// for all types: Level 0: reported but no action taken
// Shipping bans ("shipBan")
// Level 1: ban ordered but not yet implemented
// Level 2: ban implemented

// Cull
// Level 1: cull ordered but not yet implemented
// Level 2: cull implemented

// Vaccination (anticipated)
// Level 1: vax ordered but not yet implemented
// Level 2: vax implemented with some supply restriction
// Level 3: vax implemented with more supply

#ifndef Control_actions_h
#define Control_actions_h

#include "shared_functions.h"

#include <iterator> // std::distance in updates
#include <map>
#include <unordered_set>

extern int verboseLevel;

template <typename T>
struct nextChange
{ // a struct to specify what needs to be changed and how, stored at a given time in _ToChange
	T unit;
	std::string controlType;
	int level;
};

class County;

class Control_actions
// Control_actions use pointers to Farms from Status
{
	private:
		int verbose;
		std::unordered_map< std::string, std::vector<std::tuple<double,double>> > cl; // control parameters
		std::unordered_map< std::string, int> cTypeMax; // maximum level for each control type

		std::unordered_map<int, std::vector< nextChange<Farm*> >> farmsToChange; // key: times that require action, value: specifics of action for farms
		std::unordered_map<int, std::vector< nextChange<County*> >> countiesToChange; // key: times that require action, value: specifics of action for counties

		std::unordered_set<Farm*> farms; // any farms that enter the control system
		std::unordered_map< std::string, County* > counties; // mapped by fips

		std::unordered_map<std::string, std::vector<int>> farmStatusCounts; // counts of farms in each status
		std::unordered_map<std::string, std::vector<int>> countyStatusCounts;

		void scheduleLevelUp_f(Farm*, std::string, int);
		void scheduleLevelUp_c(County*, std::string, int);
		void startControlSeq_f(Farm*, std::string);
		void startControlSeq_c(County*, std::string);

		double compliance_shipBan();

	public:
		Control_actions(std::unordered_map< std::string, std::vector<std::tuple<double,double>> >&);
		~Control_actions();
		void addFarm(std::vector<Farm*>&, int, bool first=0); // uses regular lag params unless first=1 (default=0)
		void addFarm(Farm*, int, bool first=0); // overloaded for single farm
		void updates(int);
		int getNfarms(std::string, int) const; // inlined, total farms with this status (control type-level)
		int getNcounties(std::string, int) const; // inlined, total farms with this status (control type-level)
		int checkShipBan(Shipment*);

};

inline int Control_actions::getNfarms(std::string ctype, int level) const
{ // initialized to 0 in case ctype doesn't exist
	return farmStatusCounts.at(ctype).at(level);
}

inline int Control_actions::getNcounties(std::string ctype, int level) const
{ // initialized to 0 in case ctype doesn't exist
	return countyStatusCounts.at(ctype).at(level);
}

#endif
