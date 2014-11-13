// Shipment_manager.cpp
//
// Nov 10 2014

#include <cmath> // floor
// included in Shipment_manager.h: Farm, shared_functions
#include "Shipment_manager.h"

Shipment_manager::Shipment_manager(std::unordered_map<std::string, 
	std::vector<Farm*>>& in_FIPSmap)
{	
	// copy FIPSmap
	FIPSmap = in_FIPSmap;
	// record vector of all possible shipment destinations (counties)
	
	// only needed for random shipments?
	for (auto& f:FIPSmap){
		allFIPS.emplace_back(f.first);
	}
	std::cout << "Shipment manager instantiated: "<<FIPSmap.size()<<" counties with premises." << std::endl;
}

Shipment_manager::~Shipment_manager()
{
}

void Shipment_manager::shipFrom(std::vector<Farm*>& infFarms, std::vector<Farm*>& suscFarms)
{
	// get counties (by FIPS codes) of infectious farms
	std::unordered_map<std::string, std::vector<Farm*>> infFIPSmap;
	for (auto& i:infFarms){
		infFIPSmap[i->get_fips()].emplace_back(i);
	}
	// get counties (by FIPS codes) of susceptible farms
	std::unordered_map<std::string, std::vector<Farm*>> suscFIPSmap;
	for (auto& s:suscFarms){
		suscFIPSmap[s->get_fips()].emplace_back(s);
	}
	
	// generate county-county shipments for each origin county
	for (auto& oc:allFIPS){ // oc = origin county
		countyCountyShipments(oc); // writes to countyShipmentList
	}
	farmFarmShipments(infFIPSmap, suscFIPSmap); // reads from countyShipmentList, writes to farmShipmentList

}

void Shipment_manager::countyCountyShipments(std::string& oCounty, int method)
// determines and records county-county shipments and volumes
{
	if (method == 0){
	//  randomly assign destination counties with volume = 1
		double random = unif_rand();
		bool makeShipment = (random < 0.25); // 1/4 of all counties make a shipment
		if (makeShipment){
			std::string dCounty = randomFrom(allFIPS);
			std::tuple<std::string,std::string,int> countyShip (oCounty,dCounty,1);
			countyShipmentList.emplace_back(countyShip);
		}
	}
}

void Shipment_manager::farmFarmShipments(std::unordered_map<std::string, std::vector<Farm*>> infFIPSmap,
	std::unordered_map<std::string, std::vector<Farm*>> suscFIPSmap, int method)
// assigns shipments to farms from one county to another
// distribute to only biggest farms, by farm rankings, random
{
	for (auto& s:countyShipmentList){
		std::string oFIPS = std::get<0>(s); // 1st element of tuple (origin FIPS)
		// only make farm-level assignments for infectious-to-susceptible county shipments
		if (infFIPSmap.count(oFIPS)==1){ // if origin FIPS is in list of infectious FIPS
			// get destination FIPS
			std::string dFIPS = std::get<1>(s);
			if (suscFIPSmap.count(dFIPS) == 1){ // if dest FIPS is in list of susceptible FIPS
				// get volume
				int volume = std::get<2>(s);
				// get farms in counties
				std::vector<Farm*> oFarms = FIPSmap.at(oFIPS); // can be sent from any farm regardless of status
				std::vector<Farm*> dFarms = FIPSmap.at(dFIPS); // can be sent to any farm regardless of status
				// assignment
				std::vector<std::tuple<double,double,int>> farmShips;
				if (method == 0){ // random
					while (farmShips.size() < volume){ // for each shipment between these counties
						Farm* oFarm = randomFrom(oFarms);
						Farm* dFarm = randomFrom(dFarms);
						std::tuple<double, double, int> fShip (oFarm->get_id(),dFarm->get_id(),1);
						farmShips.emplace_back(fShip);
					}
				} else if (method == 1){ // all shipments to single largest farm (random origin)
					while (farmShips.size() < volume){
						Farm* oFarm = randomFrom<Farm*>(oFarms);
						Farm* dFarm = dFarms.back(); // last value has largest pop
						std::tuple<double, double, int> fShip (oFarm->get_id(),dFarm->get_id(),1);
						farmShips.emplace_back(fShip);
					}				
				} else if (method == 2){ // doesn't work yet
					// assign based on rankings
					// number of shipments, farms in o, farms in d
					// one farm in each - assign all shipments to these two
					// more farms in o than d, both less than volume - 1:5 to 1:3, v=10
					// - divide larger by smaller - p(top proportion) goes to first, etc
					// reverse for more farms in d than o
					if(oFarms.size() > dFarms.size()){
					} else if (dFarms.size() > oFarms.size()){
					} else if (oFarms.size == dFarms.size()){ // one-to-one line-up
						while (farmShips.size() < volume){
							double re = unif_rand(); // re = random element
							if (re==1){re=0.99999;}
							int randomElement = floor(re*oFarms.size());
							std::tuple<double, double, int> fShip (
								oFarms[randomElement]->get_id(),
								dFarms[randomElement]->get_id(), 1);
							farmShips.emplace_back(fShip);
						}	
					}
				}
				for (auto& f:farmShips){
					farmShipmentList.emplace_back(f); // contains non-infectious shipments
				}
			}
		}
	}
	
}

int Shipment_manager::getMaxFarmPop(std::string& FIPS){
	std::vector<Farm*> farmsInCo = FIPSmap.at(FIPS);
	int largest = -1;
	double farmID = -1;
	for (auto& f:farmsInCo){
		int farmpop = f->get_size();
		if (farmpop > largest){
			largest = farmpop;
			farmID = f->get_id();
		}
	}
}