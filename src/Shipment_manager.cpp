// Shipment_manager.cpp
//
// Nov 10 2014

#include <cmath> // floor
#include <iostream> // for error-checking output
// included in Shipment_manager.h: Farm, shared_functions
#include "Shipment_manager.h"

Shipment_manager::Shipment_manager(
	std::unordered_map<std::string, std::vector<Farm*>>& in_FIPSmap, std::vector<int>& shipSettings)
{	
	// copy FIPSmap
	FIPSmap = in_FIPSmap;
	// copy shipping settings
	banCompliance = shipSettings[0];
	banScale = shipSettings[1];
	farmFarmMethod = shipSettings[2];
	// record vector of all possible shipment destinations (counties)
	// allFIPS used in generating random shipments
	for (auto& f:FIPSmap){
		allFIPS.emplace_back(f.first);
		std::string statecode = f.first.substr(0,2); // get first two characters (substring starting at 0, length of 2)
		stateFIPSmap[statecode].emplace_back(f.first); // add to map where index is state, value is vector of counties
	}
	if(verbose){std::cout << "Shipment manager constructed: "<<FIPSmap.size()<<" counties with premises." << std::endl;}
}

Shipment_manager::~Shipment_manager()
{
}

void Shipment_manager::makeShipments(std::vector<Farm*>& infFarms, std::vector<Farm*>& suscFarms, 
	int countyMethod, std::vector<std::string>& in_bannedFIPS)
// makes shipments for ALL premises in counties, not just inf/susc provided in arguments.
// county method determines shipments kernel (can change by time)
// infFarms, suscFarms, bannedFIPS arguments are for disease bits.
{
	if (banScale==1){ 
	// compare with last number of banned FIPS for updated version
	if (lastNumBannedFIPS != in_bannedFIPS.size()){
		lastNumBannedFIPS = in_bannedFIPS.size();
		for (auto& f:in_bannedFIPS){
			std::string statecode = f.substr(0,2); // get first two characters (substring starting at 0, length of 2)
			if (!isWithin(statecode,bannedStates)){ // if this state is not in the map
				bannedStates.emplace_back(statecode);
				std::vector<std::string> FIPSInState = stateFIPSmap.at(statecode);
				for (auto& fs:FIPSInState){
					allBannedFIPS.emplace_back(fs); // add to map
				}
			}
		}
	}
	} else {
		allBannedFIPS = in_bannedFIPS; // if ban is at county level, just use bannedFIPS
	}
	
	// get counties (by FIPS codes) of infectious farms
	std::unordered_map<std::string, std::vector<Farm*>> infFIPSmap;
	for (auto& i:infFarms){
		infFIPSmap[i->get_fips()].emplace_back(i);
	}
	if(verbose){std::cout << infFarms.size()<<" infectious farms in "<<infFIPSmap.size()<<" counties. ";}
	// get counties (by FIPS codes) of susceptible farms
	std::unordered_map<std::string, std::vector<Farm*>> suscFIPSmap;
	for (auto& s:suscFarms){
		suscFIPSmap[s->get_fips()].emplace_back(s);
	}
	if(verbose){std::cout << suscFarms.size()<<" susceptible farms in "<<suscFIPSmap.size()<<" counties. ";}

	// generate county-county shipments for each origin county
	for (auto& oc:allFIPS){ // oc = origin county
		countyCountyShipments(oc, countyMethod); // writes to countyShipmentList
	}
	if(verbose){std::cout << std::endl << countyShipmentList.size() <<" county-level shipments. Assigning to farms... ";}
	farmFarmShipments(infFIPSmap, suscFIPSmap); // reads from countyShipmentList, writes to farmShipmentList
}

void Shipment_manager::countyCountyShipments(std::string& oCounty, int method)
// determines and records county-county shipments and volumes
// tuple contains origin fips, destination fips, volume
{
	if (method == 0){
		// no shipments
	} else if (method == 1){
	//  randomly assign destination counties with volume = 1
		double random = unif_rand();
		bool makeShipment = (random < 0.05); // 5% of all counties make a shipment
		if (makeShipment){
			std::string dCounty = randomFrom(allFIPS);
			bool activeBan = banShipment(oCounty); // bool for each shipment
			std::tuple<std::string,std::string,int,bool> countyShip (oCounty,dCounty,1,activeBan);
			countyShipmentList.emplace_back(countyShip);
		}
	}
	else {std::cout << "Method for countyCountyShipments not recognized." << std::endl;}

}

bool Shipment_manager::banShipment(std::string& oCounty)
// determines whether or not a given shipment is banned
// based on origin county and compliance level
{
	bool ban = 0; // initialize as no ban
	if (banCompliance > 0){
	if (isWithin(oCounty,allBannedFIPS)){ // if a ban is active for this FIPS
		if (banCompliance == 100){
			ban = 1;
		} else {
			double randDraw = unif_rand();
			if (randDraw <= banCompliance/100){ban = 1;} // stochastic compliance
		}
	}
	}
 return ban;
}

void Shipment_manager::farmFarmShipments(std::unordered_map<std::string, std::vector<Farm*>> infFIPSmap,
	std::unordered_map<std::string, std::vector<Farm*>> suscFIPSmap)
// assigns shipments to farms from one county to another, distributed:
// randomly (method=0), 
// with probability related to farm size (method=1)
// from biggest farms in county to random destinations (method=2)
// from random destinations to biggest farm in county (method=3)
// distribute only between biggest farms in each county (method = 4)
// Keeps track of ban status but otherwise ignores it. Only accounted for in the changing status step in main.
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
				// get ban status (just to copy over)
				bool activeBan = std::get<3>(s);
				// get farms in counties
				std::vector<Farm*> oFarms = FIPSmap.at(oFIPS); // can be sent from any farm regardless of status
				std::vector<Farm*> dFarms = FIPSmap.at(dFIPS); // can be sent to any farm regardless of status

				// assignment
				std::vector<std::tuple<int,int,int,bool>> farmShips;
				
				if (farmFarmMethod == 0){ // random
					for (int v = 0; v!= volume; v++){ // for each shipment between these counties
						Farm* oFarm = randomFrom(oFarms);
						Farm* dFarm = randomFrom(dFarms);
						std::tuple<int,int,int,bool> fShip (oFarm->get_id(),dFarm->get_id(),1,activeBan);
						farmShips.emplace_back(fShip);
					}
				} else if (farmFarmMethod == 1){ // shipment probability based on relative farm size of "Cattle"
					// get sizes of each farm
					std::vector<int> oFarmSizes, dFarmSizes;
					for (auto& o1:oFarms){oFarmSizes.emplace_back(o1->get_size("Cattle"));}
					for (auto& d1:dFarms){dFarmSizes.emplace_back(d1->get_size("Cattle"));}
					// get cumulative sums of sizes for farms in county to distribute probabilities
					std::vector<int> oCumSums = {0};
					std::vector<int> dCumSums = {0};
					for (int oi=0; oi!=oFarmSizes.size(); oi++){ //oi = origin iterator
						oCumSums.emplace_back(oCumSums[oi-1]+oFarmSizes[oi]);} 
					for (int di=0; di!=dFarmSizes.size(); di++){ //di = destination iterator
						dCumSums.emplace_back(dCumSums[di-1]+dFarmSizes[di]);} 
// 					std::cout << std::endl << "Origin county: ";
// 					for (auto& ocs:oCumSums){
// 						std::cout <<ocs<< " ";}
// 					std::cout << std::endl << "Destination county: ";
// 					for (auto& dcs:dCumSums){
// 						std::cout <<dcs<< " ";}
						
					for (int v=0; v!=volume; v++){
						int oRand = unif_rand()*oCumSums.back(); // scale up to maximum
						int dRand = unif_rand()*dCumSums.back(); // scale up to maximum
						int oElement = whichElement(oRand,oCumSums); // in shared_functions.h
						int dElement = whichElement(dRand,dCumSums); // in shared_functions.h
						Farm* oFarm = oFarms[oElement];
						Farm* dFarm = dFarms[dElement];
						std::tuple<int,int,int,bool> fShip (oFarm->get_id(),dFarm->get_id(),1,activeBan);
						farmShips.emplace_back(fShip);
// 						std::cout << std::endl << "Random O: " << oRand << " matches size "
// 							<< oCumSums[oElement];
// 						std::cout << std::endl << "Random D: " << dRand << " matches size "
// 							<< dCumSums[dElement];
					}				
				} else if (farmFarmMethod == 2){ // shipped from biggest farm to random destinations
					for (int v = 0; v!= volume; v++){ // for each shipment between these counties
						Farm* oFarm = oFarms.back();
						Farm* dFarm = randomFrom(dFarms);
						std::tuple<int,int,int,bool> fShip (oFarm->get_id(),dFarm->get_id(),1,activeBan);
						farmShips.emplace_back(fShip);
					}
				} else if (farmFarmMethod == 3){ // shipped from random destinations to the biggest farm
					for (int v = 0; v!= volume; v++){ // for each shipment between these counties
						Farm* oFarm = randomFrom(oFarms);
						Farm* dFarm = dFarms.back();
						std::tuple<int,int,int,bool> fShip (oFarm->get_id(),dFarm->get_id(),1,activeBan);
						farmShips.emplace_back(fShip);
					}
				} else if (farmFarmMethod == 4){ // shipped between biggest farms
					for (int v = 0; v!= volume; v++){ // for each shipment between these counties
						Farm* oFarm = oFarms.back();
						Farm* dFarm = dFarms.back();
						std::tuple<int,int,int,bool> fShip (oFarm->get_id(),dFarm->get_id(),1,activeBan);
						farmShips.emplace_back(fShip);
					}
				} else {std::cout << "Method for farmFarmShipments not recognized." << std::endl;}
				
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

void Shipment_manager::checkShipTrans(std::vector<std::tuple<int,int,int,bool>>& farmShips, 
	std::vector<Farm*>& infInoFIPS, std::vector<Farm*>& suscIndFIPS)
// identify shipments from infectious to susceptible farms
// input is: (1) tuple of origin farm ID, dest farm ID, volume, banStatus
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
