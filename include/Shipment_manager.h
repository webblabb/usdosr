#ifndef Shipment_manager_h
#define Shipment_manager_h

#include <set>
#include <tuple> // std::tuple

#include "Status_manager.h"
#include "shared_functions.h"

extern int verboseLevel;

/// County-to-county shipment
struct coShipment{
	int t; ///< Time of shipment
	std::string origFIPS; ///< Premises ID of shipment origin
	std::string destFIPS; ///< Premises ID of shipment destination
	std::string species;
	int volume; ///> Generally 1, but allows for multiple shipments between counties in the same t
	int ban; ///< 0 = no ban, 1 = ban ordered but non-compliant, 2 = ban ordered & compliant
};

///> Manages the shipments (USAMM) part of the simulation
/// Predicts shipments from counties with infectious farms
/// Gets called at each timepoint
/// Bans: go into effect "after" shipments determined - for tracking business continuity
class Shipment_manager
{
	private:
		int verbose; ///< Can be set to override global setting for console output
		
		// const pointers to Grid_manager objects:
		const std::unordered_map<std::string, std::vector<Farm*>>* FIPSmap;
		const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Farm*> >>* fipsSpeciesMap;
		
		Status_manager* S; ///< Const pointer to Status manager (to access up-to-date premises statuses)
		// used to generate random shipments
		std::vector<std::string> allFIPS; // list of all possible destination FIPS, based on premises file
		int farmFarmMethod;
		std::vector<std::string> species;
		std::unordered_map<std::string, std::vector<std::string>> speciesFIPS; // just for countycounty fake assignment to make sure appropriate county is chosen
	
		// the following are recreated/rewritten at each timestep
		std::vector<coShipment>
			countyShipmentList; // coShipment defined above
		std::vector<shipment*> // shipment defined in shared_functions.h
			farmShipmentList;
		int startRecentShips, startCoRecentShips; // indicates index in shipmentList where the most recent set of shipments starts

		// functions
		void countyCountyShipments(std::string, int); ///< Determines county-county movements & volumes
		Farm* largestStatus(std::vector<Farm*>&, std::string&); ///< Finds largest premises with "status", from vector sorted by population
		void farmFarmShipments(); ///< Assigns county shipments to individual farms
	
	public:
		Shipment_manager(
			const std::unordered_map<std::string, std::vector<Farm*>>*, 
			const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Farm*> >>*,
			Status_manager*,
			int,
			const std::vector<std::string>&);
		
		~Shipment_manager();
		
		void makeShipments(std::vector<Farm*>&, int, std::vector<shipment*>&); ///< Generates and returns farm-level shipments				

};

#endif