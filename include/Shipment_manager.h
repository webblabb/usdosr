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

class Shipment_manager
{
	private:
		int verbose; 
		// the following are constant through simulation:
		std::unordered_map<std::string, std::vector<Farm*>> FIPSmap;
		// map linking fips to states for state-wide bans
		std::unordered_map<std::string, std::vector<std::string>> stateFIPSmap;
		std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Farm*> >> fipsSpeciesMap;
		// used to generate random shipments
		std::vector<std::string> allFIPS; // list of all possible destination FIPS, based on premises file
		double banCompliance;
		int banScale, farmFarmMethod;
		std::vector<std::string> species;

		int lastNumBannedFIPS = 0; // last number of banned FIPS
		std::vector<std::string> bannedStates; // states with banned fips
		std::vector<std::string> allBannedFIPS; // list including all other fips in the same states as banned fips (if banScale=1)
	
		// the following are recreated/rewritten at each timestep
		int shipCount;
		std::vector<std::tuple<std::string,std::string,int,std::string,bool>>
			countyShipmentList; // vector of tuples, each containing originFIPS, destFIPS, volume, ban true/false
		std::vector<std::tuple<int,int,int,std::string,bool,bool>> 
			farmShipmentList; // 1st key: origin farm ID, 2nd key: destination farm ID, value: shipment count, ban true/false, ban compliant true/false
		std::vector<std::tuple<int,int,int,std::string,bool,bool>>
			infFarmShips; // shipments that originate from infectious farms to susceptible farms

		// functions
		void countyCountyShipments(std::string&, int); // determines county-county movements & volumes
		bool banShipment(std::string&);
		std::vector<int> banCompliant(int);
		Farm* largestStatus(std::vector<Farm*>&, std::string&); // finds largest premises with "status", from vector sorted by population
		void farmFarmShipments(std::unordered_map<std::string, std::vector<Farm*>>, 
			std::unordered_map<std::string, std::vector<Farm*>>); 
			// assigns county shipments to individual farms
			// input is FIPS-keyed maps of infectious farms and susceptible farms, assignment method indicator
		void checkShipTrans(std::vector<std::tuple<int,int,int,std::string,bool,bool>>&, 
			std::vector<Farm*>&, std::vector<Farm*>&);
	
	public:
		Shipment_manager( // construct with 
			std::unordered_map<std::string, std::vector<Farm*>>&, // a map of FIPS codes to farms
			std::vector<int>&, // list of shipping parameters
			std::vector<std::string>&, // list of species on premises
			std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Farm*> >>&); // sorted populations of species on farms
		
		~Shipment_manager();
		
		void makeShipments(std::vector<Farm*>&, std::vector<Farm*>&, 
			int countyMethod, std::vector<std::string>&);
				
		std::vector<std::tuple<std::string,std::string,int,std::string,bool>> // returns vector of tuples of origin FIPS, dest FIPS, volume, species, banned
			get_countyShipments() const; // inlined
			
		std::vector<std::tuple<int,int,int,std::string,bool,bool>> // returns vector of tuples of origin farm id, dest farm id, volume, species, banned, compliant
			get_farmShipments() const; // inlined

		std::vector<std::tuple<int,int,int,std::string,bool,bool>> // returns vector of tuples of origin farm id, dest farm id, volume, species, banned, compliant
			get_infFarmShipments() const; // inlined
			
		std::string formatOutput(int, int); // formats output to string

};

inline std::vector<std::tuple<std::string,std::string,int,std::string,bool>>
	Shipment_manager::get_countyShipments() const
{
	return(countyShipmentList);
}

inline std::vector<std::tuple<int,int,int,std::string,bool,bool>>
	Shipment_manager::get_farmShipments() const
{
	return(farmShipmentList);
}

inline std::vector<std::tuple<int,int,int,std::string,bool,bool>>
	Shipment_manager::get_infFarmShipments() const
{
	return(infFarmShips);
}

#endif