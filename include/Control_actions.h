#ifndef Control_actions_h
#define Control_actions_h

#include <map>
#include <unordered_set>

#include "file_manager.h" // For parameters struct. Also includes shared functions.

extern int verboseLevel;

///> Specifies the next status change to occur, at a time indicated by a time index in farmsToChange or countiesToChange
template <typename T>
struct nextChange
{
	T* unit; // For premises, the unit is Prem_status (from Status_manager)
	std::string controlType;
	int level; // Level that unit will be set to
};

///> Manages control-action related decisions and effects.
///
/// Defines control actions, updates progression of control statuses. 
/// Examples: 
///  - cull all premises within a 10 km radius of any infected premises
///  - ban all shipments from the county or state of an infected premises
///  - comply with bans, culls, etc. at a given percentage according to some rule
///  - vaccinated farms transmit with reduced efficiency
///
/// Types and progress levels of control
/// Shipping bans ("shipBan")
/// Level 0: reported but no action taken
/// Level 1: ban ordered but not yet implemented
/// Level 2: ban implemented
///
/// Cull
/// Level 1: cull ordered but not yet implemented
/// Level 2: cull implemented
///
/// Vaccination (anticipated)
/// Level 1: vax ordered but not yet implemented
/// Level 2: vax implemented with some supply restriction
/// Level 3: vax implemented with more supply

class Control_actions
{
	private:
		int verbose; ///< Can be set to override global setting for console output
		std::unordered_map< std::string, std::vector<std::tuple<double,double>> > cl; ///< Control parameters
		std::unordered_map< std::string, int> cTypeMax; ///< Maximum level for each control type
		
		std::unordered_map<int, std::vector< nextChange<Prem_status> >> farmsToChange; ///< Key is time at which change occurs, value is status change for a farm
		std::unordered_map<int, std::vector< nextChange<County> >> countiesToChange; ///< Key is time at which change occurs, value is status change for a county
		
		std::unordered_map< std::string, County* > counties; ///< Map of counties being controlled, key is county identifier (FIPS)
		
		std::unordered_map<std::string, std::vector<int>> farmStatusCounts; ///< Counts of farms in each status (for output)
		std::unordered_map<std::string, std::vector<int>> countyStatusCounts; ///< Counts of counties in each status (for output)
		
		void scheduleLevelUp_f(Prem_status*, std::string, int, int); ///< Schedule a status shift for a premises
		void scheduleLevelUp_c(County*, std::string, int, int); ///< Schedule a status shift for a county
		void startControlSeq_f(Prem_status*, std::string, int); ///< Begin control sequence for a type of control at the premises level
		void startControlSeq_c(County*, std::string, int); ///< Begin control sequence for a type of control at the county level
		
		double compliance_shipBan(); ///< Determine compliance level of a premises with an implemented shipping ban
		
	public:
		Control_actions(const Parameters*);
		~Control_actions();
		void addFarm(std::vector<Prem_status*>&, int, bool index=0); ///< Adds premises (and later, counties) to the control system
		void addFarm(Prem_status*, int, bool index=0); ///< Accepts single farm to add to control system
		void updates(int); ///< Checks countiesToChange and farmsToChange lists to see if any changes need to be made now (time t)
		int getNfarms(std::string, int) const; // inlined ///< Return the number of premises with given control type and level
		int getNcounties(std::string, int) const; // inlined ///< Return the number of counties with given control type and level
		int checkShipBan(Shipment*); ///< Determines if a shipment occurs, depending on status and stochastic compliance

};

/// \param[in]	ctype	Type of control action (i.e. "shipBan")
///	\param[in]	level	Progress level of control
inline int Control_actions::getNfarms(std::string ctype, int level) const
{ // initialized to 0 in case ctype doesn't exist
	return farmStatusCounts.at(ctype).at(level);
}

/// \param[in]	ctype	Type of control action (i.e. "shipBan")
///	\param[in]	level	Progress level of control
inline int Control_actions::getNcounties(std::string ctype, int level) const
{ // initialized to 0 in case ctype doesn't exist
	return countyStatusCounts.at(ctype).at(level);
}

#endif
