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

extern bool verbose;

class Shipment_manager
{
	private:
		// the following are constant through simulation:
		std::unordered_map<std::string, std::vector<Farm*>> FIPSmap;
		// map linking fips to states for state-wide bans
		std::unordered_map<std::string, std::vector<std::string>> stateFIPSmap;
		// used to generate random shipments
		std::vector<std::string> allFIPS; // list of all possible destination FIPS, based on premises file
		int lastNumBannedFIPS = 0; // last number of banned FIPS
		std::vector<std::string> bannedStates; // states with banned fips
		std::vector<std::string> allBannedFIPS; // list including all other fips in the same states as banned fips (if banScale=1)
		
		double banCompliance;
		int banScale, farmFarmMethod;
	
		// the following are recreated/rewritten at each timestep
		std::vector<std::tuple<std::string,std::string,int,bool>>
			countyShipmentList; // vector of tuples, each containing originFIPS, destFIPS, volume, ban true/false
		std::vector<std::tuple<int,int,int,bool>> 
			farmShipmentList; // 1st key: origin farm ID, 2nd key: destination farm ID, value: shipment count, ban true/false
		std::vector<std::tuple<int,int,int,bool>>
			infFarmShips; // shipments that originate from infectious farms to susceptible farms

		// functions
		void countyCountyShipments(std::string&, int); // determines county-county movements & volumes
		bool banShipment(std::string&);
		void farmFarmShipments(std::unordered_map<std::string, std::vector<Farm*>>, 
			std::unordered_map<std::string, std::vector<Farm*>>); 
			// assigns county shipments to individual farms
			// input is FIPS-keyed maps of infectious farms and susceptible farms, assignment method indicator
		void checkShipTrans(std::vector<std::tuple<int,int,int,bool>>&, 
			std::vector<Farm*>&, std::vector<Farm*>&);
	
	public:
		Shipment_manager( // construct with 
			std::unordered_map<std::string, std::vector<Farm*>>&, std::vector<int>&); // a map of FIPS codes to farms
		
		~Shipment_manager();
		
		void makeShipments(std::vector<Farm*>&, std::vector<Farm*>&, 
			int countyMethod, std::vector<std::string>&);
				
		std::vector<std::tuple<std::string,std::string,int,bool>> // returns vector of tuples of origin FIPS, dest FIPS, volume
			get_countyShipments() const; // inlined
			
		std::vector<std::tuple<int,int,int,bool>> // returns vector of tuples of origin farm id, dest farm id, volume
			get_farmShipments() const; // inlined

		std::vector<std::tuple<int,int,int,bool>> // returns vector of tuples of origin farm id, dest farm id, volume
			get_infFarmShipments() const; // inlined

};

inline std::vector<std::tuple<std::string,std::string,int,bool>>
	Shipment_manager::get_countyShipments() const
{
	return(countyShipmentList);
}

inline std::vector<std::tuple<int,int,int,bool>>
	Shipment_manager::get_farmShipments() const
{
	return(farmShipmentList);
}

inline std::vector<std::tuple<int,int,int,bool>>
	Shipment_manager::get_infFarmShipments() const
{
	return(infFarmShips);
}

#endif