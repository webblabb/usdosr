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

struct coShipment{
	int t, // time of shipment
	std::string origFIPS, // premises ID of shipment origin
	std::string destFIPS; // premises ID of shipment destination
	int ban; // 0 = no ban, 1 = ban ordered but non-compliant, 2 = ban ordered & compliant
};

class Shipment_manager
{
	private:
		int verbose; 
		
		// const pointers to Grid_manager objects:
		const std::unordered_map<std::string, std::vector<Farm*>>* FIPSmap;
		const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Farm*> >>* fipsSpeciesMap;
		// used to generate random shipments
		std::vector<std::string> allFIPS; // list of all possible destination FIPS, based on premises file
		int farmFarmMethod;
		std::vector<std::string> species;
	
		// the following are recreated/rewritten at each timestep
		int shipCount;
		std::vector<coShipment>
			countyShipmentList; // vector of tuples, each containing originFIPS, destFIPS, volume, ban true/false
		std::vector<shipment> 
			farmShipmentList; // 1st key: origin farm ID, 2nd key: destination farm ID, value: shipment count, ban true/false, ban compliant true/false

		// functions
		void countyCountyShipments(std::string&, int); // determines county-county movements & volumes
		Farm* largestStatus(std::vector<Farm*>&, std::string&); // finds largest premises with "status", from vector sorted by population
		void farmFarmShipments(std::unordered_map<std::string, std::vector<Farm*>>, 
			std::unordered_map<std::string, std::vector<Farm*>>); 
			// assigns county shipments to individual farms
			// input is FIPS-keyed maps of infectious farms and susceptible farms, assignment method indicator
	
	public:
		Shipment_manager( // construct with 
			const std::unordered_map<std::string, std::vector<Farm*>>*, // a map of FIPS codes to farms
			std::vector<int>&, // list of shipping parameters
			std::vector<std::string>&, // list of species on premises
			const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Farm*> >>*); // sorted populations of species on farms
		
		~Shipment_manager();
		
		void makeShipments(std::vector<Farm*>&, std::vector<Farm*>&, 
			int countyMethod, std::vector<std::string>&);				
		void take_countyShipments(std::vector<coShipment>&); // inlined
		void take_shipments(std::vector<shipment>&); // inlined
		std::string formatOutput(int, int); // formats output to string

};

inline void Shipment_manager::take_countyShipments(std::vector<coShipment>& output)
{
	output.swap(countyShipmentList);
}

inline void Shipment_manager::take_farmShipments(std::vector<shipment>& output)
{
	output.swap(farmShipmentList);
}

#endif