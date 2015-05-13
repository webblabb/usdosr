// Shipment_manager.h
//
// Predicts shipments from counties with infectious farms
// Gets called at each timepoint
// Bans: go into effect "after" shipments determined - for tracking business continuity

#ifndef Shipment_manager_h
#define Shipment_manager_h

// included in Farm.h: string, unordered map, vector
#include "Farm.h"
#include "shared_functions.h"

#include <set>
#include <tuple> // std::tuple

extern int verboseLevel;

class County;

struct coShipment{
	int t; // time of shipment
	std::string origFIPS; // premises ID of shipment origin
	std::string destFIPS; // premises ID of shipment destination
	std::string species;
	int volume; // will be 1 most of the time, but allows for multiple shipments between counties in the same t
	int ban; // 0 = no ban, 1 = ban ordered but non-compliant, 2 = ban ordered & compliant
};

class Shipment_manager
{
	private:
		int verbose;

		// const pointers to Grid_manager objects:
		const std::unordered_map<std::string, County*>* FIPSmap;
		const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Farm*> >>* fipsSpeciesMap;
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
		void countyCountyShipments(std::string, int); // determines county-county movements & volumes
		Farm* largestStatus(std::vector<Farm*>&, std::string&); // finds largest premises with "status", from vector sorted by population
		void farmFarmShipments(); // assigns county shipments to individual farms

	public:
		Shipment_manager( // construct with
			const std::unordered_map<std::string, County*>*, // a map of FIPS codes to farms
			const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Farm*> >>*, // sorted populations of species on farms
			int, // farm assignment method
			std::vector<std::string>&); // list of species on premises

		~Shipment_manager();

		void makeShipments(std::vector<Farm*>&, int, std::vector<shipment*>&); // generates and returns farm-level shipments
		std::string formatOutput(int, int); // formats output to string

};

#endif
