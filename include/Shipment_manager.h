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

#include <tuple> // std::tuple

class Shipment_manager
{
	private:
		// the following are constant through simulation:
		std::unordered_map<std::string, std::vector<Farm*>> FIPSmap;
		// only needed for random shipments?
		std::vector<std::string> allFIPS; // list of all possible destination FIPS, based on premises file
		// the following are recreated/rewritten at each timestep
		std::vector<std::tuple<std::string,std::string,int>>
			countyShipmentList; // vector of tuples, each containing originFIPS, destFIPS, volume
		std::vector<std::tuple<int,int,int>> 
			farmShipmentList; // 1st key: origin farm ID, 2nd key: destination farm ID, value: shipment count
		std::vector<std::tuple<int,int,int>>
			infFarmShips; // shipments that originate from infectious farms to susceptible farms

		// functions
		void countyCountyShipments(std::string&, int method = 0); // determines county-county movements & volumes
		void farmFarmShipments(std::unordered_map<std::string, std::vector<Farm*>>, 
			std::unordered_map<std::string, std::vector<Farm*>>, int); 
		// assign county shipments to individual farms
		// input is FIPS-keyed maps of infectious farms and susceptible farms, assignment method indicator
		void checkShipTrans(std::vector<std::tuple<int,int,int>>&, 
			std::vector<Farm*>&, std::vector<Farm*>&);
	
	public:
		Shipment_manager( // construct with 
			std::unordered_map<std::string, std::vector<Farm*>>&); // a map of FIPS codes to farms
		
		~Shipment_manager();
		
		void makeShipments(std::vector<Farm*>&, std::vector<Farm*>&, int farmFarmMethod=1);
				
		std::vector<std::tuple<std::string,std::string,int>> // returns vector of tuples of origin FIPS, dest FIPS, volume
			get_countyShipments() const; // inlined
			
		std::vector<std::tuple<int,int,int>> // returns vector of tuples of origin farm id, dest farm id, volume
			get_farmShipments() const; // inlined

		std::vector<std::tuple<int,int,int>> // returns vector of tuples of origin farm id, dest farm id, volume
			get_infFarmShipments() const; // inlined

};

inline std::vector<std::tuple<std::string,std::string,int>>
	Shipment_manager::get_countyShipments() const
{
	return(countyShipmentList);
}

inline std::vector<std::tuple<int,int,int>>
	Shipment_manager::get_farmShipments() const
{
	return(farmShipmentList);
}

inline std::vector<std::tuple<int,int,int>>
	Shipment_manager::get_infFarmShipments() const
{
	return(infFarmShips);
}

#endif