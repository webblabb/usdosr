// Shipment_manager.cpp
//
// Nov 10 2014

#include <cmath> // floor
#include <iostream> // for error-checking output
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

void Shipment_manager::makeShipments(std::vector<Farm*>& infFarms, std::vector<Farm*>& suscFarms)
// makes shipments for ALL counties, not just inf/susc provided in arguments.
// arguments are for disease bits.
{
	// get counties (by FIPS codes) of infectious farms
	std::unordered_map<std::string, std::vector<Farm*>> infFIPSmap;
	for (auto& i:infFarms){
		infFIPSmap[i->get_fips()].emplace_back(i);
	}
	std::cout << infFarms.size()<<" infectious farms in "<<infFIPSmap.size()<<" counties. ";
	// get counties (by FIPS codes) of susceptible farms
	std::unordered_map<std::string, std::vector<Farm*>> suscFIPSmap;
	for (auto& s:suscFarms){
		suscFIPSmap[s->get_fips()].emplace_back(s);
	}
	std::cout << suscFarms.size()<<" susceptible farms in "<<suscFIPSmap.size()<<" counties. ";
	// generate county-county shipments for each origin county
	for (auto& oc:allFIPS){ // oc = origin county
		countyCountyShipments(oc); // writes to countyShipmentList
	}
	std::cout << std::endl << countyShipmentList.size() <<" shipments at county level. Assigning to farms... "<<std::endl;
	farmFarmShipments(infFIPSmap, suscFIPSmap); // reads from countyShipmentList, writes to farmShipmentList
}

void Shipment_manager::countyCountyShipments(std::string& oCounty, int method)
// determines and records county-county shipments and volumes
{
	if (method == 0){
	//  randomly assign destination counties with volume = 1
		double random = unif_rand();
		bool makeShipment = (random < 0.05); // 5% of all counties make a shipment
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
// distribute randomly (method=0), to only biggest farms in county (method=1)
// by matching farm size rankings (method=2)
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
				std::vector<std::tuple<int,int,int>> farmShips;
				if (method == 0){ // random
					for (int v = 0; v!= volume; v++){ // for each shipment between these counties
						Farm* oFarm = randomFrom(oFarms);
						Farm* dFarm = randomFrom(dFarms);
						std::tuple<int,int,int> fShip (oFarm->get_id(),dFarm->get_id(),1);
						farmShips.emplace_back(fShip);
					}
				} else if (method == 1){ // shipment probability based on relative farm size
					// get sizes of each farm
					std::vector<int> oFarmSizes, dFarmSizes;
					for (auto& o1:oFarms){oFarmSizes.emplace_back(o1->get_size());}
					for (auto& d1:dFarms){dFarmSizes.emplace_back(d1->get_size());}
					// get cumulative sums for farms in county to distribute probabilities
					std::vector<int> oCumSums = {oFarmSizes[0]};
					std::vector<int> dCumSums = {dFarmSizes[0]};
					for (int oi=1; oi!=oFarmSizes.size(); oi++){ //oi = origin iterator
						oCumSums.emplace_back(oCumSums[oi-1]+oFarmSizes[oi]);} 
					for (int di=1; di!=dFarmSizes.size(); di++){ //di = destination iterator
						dCumSums.emplace_back(dCumSums[di-1]+dFarmSizes[di]);} 
					std::cout << std::endl << "Origin county: ";
					for (auto& ocs:oCumSums){
						std::cout <<ocs<< " ";}
					std::cout << std::endl << "Destination county: ";
					for (auto& dcs:dCumSums){
						std::cout <<dcs<< " ";}
						
					for (int v=0; v!=volume; v++){
						double oRand = unif_rand();
						double dRand = unif_rand();
						int oElement = whichElement(oRand,oCumSums); // in shared_functions.h
						int dElement = whichElement(dRand,dCumSums); // in shared_functions.h
						Farm* oFarm = oFarms[oElement];
						Farm* dFarm = dFarms[dElement];
						std::tuple<int,int,int> fShip (oFarm->get_id(),dFarm->get_id(),1);
						farmShips.emplace_back(fShip);
						std::cout << std::endl << "Random O: " << oRand << " matches size "
							<< oCumSums[oElement];
						std::cout << std::endl << "Random D: " << dRand << " matches size "
							<< dCumSums[dElement];
					}				
				} else { 
				} // end methods
				for (auto& f:farmShips){
					farmShipmentList.emplace_back(f); // contains non-infectious shipments as well
				}
				// id infectious shipments from this county
				checkShipTrans(farmShips,infFIPSmap.at(oFIPS),suscFIPSmap.at(dFIPS));
				farmShips.clear();
			} // end if origin has inf and dest has susc
		} // end if origin has inf
	} // end for each shipment
	
}

void Shipment_manager::checkShipTrans(std::vector<std::tuple<int,int,int>>& farmShips, 
	std::vector<Farm*>& infInoFIPS, std::vector<Farm*>& suscIndFIPS)
// identify shipments from infectious to susceptible farms
// input is: (1) tuple of origin farm ID, dest farm ID, volume,
// (2) vector<Farm*> of infectious farms in origin farm's county
// (3) vector<Farm*> of susceptible farms in destination farm's county
{	
	// put IDs of infectious and susceptible farms into vectors for isWithin function
	std::vector<int> infFarmIDs, suscFarmIDs;
	for (auto& ifarm:infInoFIPS){infFarmIDs.emplace_back(ifarm->get_id());}
	for (auto& sfarm:suscIndFIPS){suscFarmIDs.emplace_back(sfarm->get_id());}

	for (auto fs:farmShips){ // check if each origin farm is infectious
		int origFarm = std::get<0>(fs); // get origin farm ID for this shipment
		 if(isWithin(origFarm,infFarmIDs)){ // if origin is infectious
			int destFarm = std::get<1>(fs); // get dest farm ID for this shipment
			if(isWithin(destFarm,suscFarmIDs)){ // check if destination is in susceptible list
				infFarmShips.emplace_back(fs);}
		 }
	}
		
}

