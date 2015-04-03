// Shipment_manager.cpp
//
// Nov 10 2014

#include <cmath> // floor
#include <iostream> // for error-checking output
#include "Shipment_manager.h" // includes Farm, shared_functions

Shipment_manager::Shipment_manager(
	const std::unordered_map<std::string, std::vector<Farm*>>* in_FIPSmap, std::vector<int>& shipSettings,
	std::vector<std::string>& speciesOnPrems,
	const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Farm*> >>* fipsSpMap)
	:
	FIPSmap(in_FIPSmap),
	fipsSpeciesMap(fipsSpMap),
	banCompliance((double) shipSettings[0]), // will later be compared against a double
	banScale(shipSettings[1]),
	farmFarmMethod(shipSettings[2]),
	species(speciesOnPrems)
{	
	verbose = verboseLevel;
	
	allFIPS.reserve(FIPSmap->size());
	stateFIPSmap.reserve(50);
	
	// copy species
	// record vector of all possible shipment destinations (counties)
	// allFIPS used in generating random shipments, also record state
	for (auto& f:(*FIPSmap)){
		allFIPS.emplace_back(f.first);
		std::string statecode = f.first.substr(0,2); // get first two characters (substring starting at 0, length of 2)
		stateFIPSmap[statecode].emplace_back(f.first); // add to map where index is state, value is vector of counties
	}
	if(verbose>0){std::cout << "Shipment manager constructed: "<<FIPSmap->size()<<" counties with premises." << std::endl;}
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
	countyShipmentList.clear();
	farmShipmentList.clear();
	infFarmShips.clear();
	
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
	if(verbose>1){std::cout << infFarms.size()<<" infectious farms in "<<infFIPSmap.size()<<" counties. ";}
	// get counties (by FIPS codes) of susceptible farms
	std::unordered_map<std::string, std::vector<Farm*>> suscFIPSmap;
	for (auto& s:suscFarms){
		suscFIPSmap[s->get_fips()].emplace_back(s);
	}
	if(verbose>1){std::cout << suscFarms.size()<<" susceptible farms in "<<suscFIPSmap.size()<<" counties. ";}

	// generate county-county shipments for each origin county
	for (auto& oc:allFIPS){ // oc = origin county
		countyCountyShipments(oc, countyMethod); // writes to countyShipmentList
	}
	if(verbose>1){std::cout << std::endl << countyShipmentList.size() <<" total shipments. Assigning to farms... "<<std::endl;}
	farmFarmShipments(infFIPSmap, suscFIPSmap); // reads from countyShipmentList, writes to farmShipmentList
}

void Shipment_manager::countyCountyShipments(std::string& oCounty, int method)
// determines and records county-county shipments and volumes
// tuple contains origin fips, destination fips, volume
{
	shipCount = 0;
	if (method == 0){
		// no shipments
	} else if (method == 1){
	//  randomly assign destination counties with volume = 1
		double random = unif_rand();
		bool makeShipment = (random < 0.5); // proportion of all counties make a shipment
		if (makeShipment){
			std::string dCounty = randomFrom(allFIPS);
			int volume = 1;
			std::string sp = "beef";
			bool activeBan = banShipment(oCounty); // bool for each shipment
			auto countyShip = std::make_tuple(oCounty,dCounty,volume,sp,activeBan);
			countyShipmentList.emplace_back(countyShip);
			shipCount += volume;
		}
	}
	else {std::cout << "Method for countyCountyShipments not recognized." << std::endl;}

}

bool Shipment_manager::banShipment(std::string& oCounty)
// determines whether or not a given shipment is banned based on origin county
{
	bool ban = 0; // initialize as no ban
	if (banCompliance > 0){
		if (isWithin(oCounty,allBannedFIPS)){ban = 1;} // if a ban is active for this FIPS
	}
 return ban;
}

std::vector<int> Shipment_manager::banCompliant(int vol)
// stochastically determines each shipment (of volume # of ships) will be compliant with a ban
// a bit clumsy using int and converting, but oddly std::vector<bool> didn't support emplace_back
{
	std::vector<int> outvec;
	for (auto c=0; c!=vol; c++){
		double randCompliance = unif_rand();
		if (randCompliance <= banCompliance){
			outvec.emplace_back(1); // compliant
		} else {
			outvec.emplace_back(0); // non-compliant
		}
	}
 return outvec;
}

Farm* Shipment_manager::largestStatus(std::vector<Farm*>& premVec, std::string& status)
// return largest premises with a given status
{
	bool found = 0;
	auto i = premVec.back(); // start at end of sorted vector (largest prem) and work backwards
	while (found==0){
		if(i->get_status()==status){ // if prem has this status, stop and return this prem
			found = 1;
		} else if ( i>premVec.front() ){i--; // keep moving backwards
		} else if ( i==premVec.front() ){ i = premVec.back(); found = 1;} // if no prems have this status, return largest
	}
	return i;
}

void Shipment_manager::farmFarmShipments(std::unordered_map<std::string, std::vector<Farm*>> infFIPSmap,
	std::unordered_map<std::string, std::vector<Farm*>> suscFIPSmap)
// assigns shipments to farms from one county to another, distributed:
// randomly (method=0), 
// with probability related to farm size (method=1)
// from biggest farms in county to random destinations (method=2)
// from random destinations to biggest farm in county (method=3)
// distribute only between biggest farms in each county (method = 4)
// Keeps track of ban status, but otherwise ignores it. Only accounted for when changing status in main.
//
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
				// get species
				std::string sp = std::get<3>(s);
				// get ban status (just to copy over)
				bool activeBan = std::get<4>(s);

				// get all farms with species in counties - can be sent to/from any farm regardless of status
				std::vector<Farm*> oPrems = fipsSpeciesMap->at(oFIPS)[sp]; // already sorted by population
				std::vector<Farm*> dPrems = fipsSpeciesMap->at(dFIPS)[sp];
 				if(verbose==2){std::cout<<"Origin FIPS: "<<oFIPS<<" has "<<oPrems.size()<<
 				" prems, destination FIPS: "<<dFIPS<<" has "<<dPrems.size()<<std::endl;}

			  	// sizes of prems with this species, sorted by population size
				std::vector<int> oPremSizes, dPremSizes;
				for (auto& ps:oPrems){oPremSizes.emplace_back(ps->Farm::get_size(sp));} // store population
				if (oFIPS==dFIPS){dPremSizes = oPremSizes;
				} else {
					for (auto& ps:dPrems){dPremSizes.emplace_back(ps->Farm::get_size(sp));} // store population
				} // end "if origin and destination FIPS are the same"
				
				// check if there are 0 premises in county - would this happen in USAMM? For now, skip that shipment
				if (oPrems.size()==0 || dPrems.size()==0){
					std::cout<<"County without proper premises type, skipping shipment."<<std::endl;
				} else {
			    // assignment
				// if ban in effect, determine compliance for all shipments
			    std::vector<int> comply (volume, 0); // initialize to 0 (non-compliant or no ban)
				if(activeBan){comply = banCompliant(volume);} 

				if (farmFarmMethod == 0){ // random
					std::vector<Farm*> origPrems = random_unique(oPrems,volume); // pick v random premises
					std::vector<Farm*> destPrems = random_unique(dPrems,volume);
					for (int v = 0; v!= volume; v++){ // for each shipment
						auto fShip = std::make_tuple(origPrems[v]->get_id(), // origin
							destPrems[v]->get_id(), // destination
							1, // # of shipments are all 1, since we loop through "volume" number of times
							sp, // species
							activeBan, // county-level ban true/false
							(bool)comply[v]); // compliant 
						farmShipmentList.emplace_back(fShip);
						if (origPrems[v]->Farm::get_status()=="inf" && destPrems[v]->Farm::get_status()=="sus"){
							infFarmShips.emplace_back(fShip);}
					}
				} else if (farmFarmMethod == 1){ // shipment probability based on relative farm size				
					// get cumulative sums of sizes for farms in county to distribute probabilities
					std::vector<int> oCumSums = {0};
					std::vector<int> dCumSums = {0};
					for (unsigned int oi=0; oi!=oPremSizes.size(); oi++){ //oi = origin iterator
						oCumSums.emplace_back(oCumSums[oi-1]+oPremSizes[oi]);} 
					for (unsigned int di=0; di!=dPremSizes.size(); di++){ //di = destination iterator
						dCumSums.emplace_back(dCumSums[di-1]+dPremSizes[di]);} 
// 					std::cout << std::endl << "Origin county populations: ";
// 					for (auto& ocs:oCumSums){
// 						std::cout <<ocs<< " ";}
// 					std::cout << std::endl << "Destination county populations: ";
// 					for (auto& dcs:dCumSums){
// 						std::cout <<dcs<< " ";}
						
					for (int v=0; v!=volume; v++){
						int oRand = unif_rand()*oCumSums.back(); // scale up to maximum
						int dRand = unif_rand()*dCumSums.back(); // scale up to maximum
						int oElement = whichElement(oRand,oCumSums); // whichElement is in shared_functions.h
						int dElement = whichElement(dRand,dCumSums); // whichElement is in shared_functions.h
						Farm* oPrem = oPrems[oElement];
						Farm* dPrem = dPrems[dElement];
						auto fShip = std::make_tuple(oPrem->get_id(),dPrem->get_id(),1,sp,activeBan,(bool)comply[v]);
						farmShipmentList.emplace_back(fShip);
// 						std::cout << std::endl << "Random O: " << oRand << " matches size "
// 							<< oCumSums[oElement];
// 						std::cout << std::endl << "Random D: " << dRand << " matches size "
// 							<< dCumSums[dElement];
						if (oPrem->Farm::get_status()=="inf" && dPrem->Farm::get_status()=="sus"){
							infFarmShips.emplace_back(fShip);}
					}				
				} else if (farmFarmMethod == 2){ // shipped from biggest infectious farm to random destinations
					for (int v = 0; v!= volume; v++){ // for each shipment between these counties
						std::string infstring = "inf";
						Farm* oPrem = largestStatus(oPrems,infstring);
						Farm* dPrem = randomFrom(dPrems);
						auto fShip = std::make_tuple(oPrem->get_id(),dPrem->get_id(),1,sp,activeBan,(bool)comply[v]);
						farmShipmentList.emplace_back(fShip);
						if (oPrem->Farm::get_status()=="inf" && dPrem->Farm::get_status()=="sus"){
							infFarmShips.emplace_back(fShip);}
					}
				} else if (farmFarmMethod == 3){ // shipped from random destinations to the biggest susceptible farm
					for (int v = 0; v!= volume; v++){ // for each shipment between these counties
						std::string susstring = "sus";
						Farm* oPrem = randomFrom(oPrems);
						Farm* dPrem = largestStatus(dPrems,susstring);
						auto fShip = std::make_tuple(oPrem->get_id(),dPrem->get_id(),1,sp,activeBan,(bool)comply[v]);
						farmShipmentList.emplace_back(fShip);
						if (oPrem->Farm::get_status()=="inf" && dPrem->Farm::get_status()=="sus"){
							infFarmShips.emplace_back(fShip);}
					}
				} else if (farmFarmMethod == 4){ // shipped between biggest farms
					for (int v = 0; v!= volume; v++){ // for each shipment between these counties
						std::string infstring = "inf";
						std::string susstring = "sus";
						Farm* oPrem = largestStatus(oPrems,infstring);
						Farm* dPrem = largestStatus(dPrems,susstring);
						auto fShip = std::make_tuple(oPrem->get_id(),dPrem->get_id(),1,sp,activeBan,(bool)comply[v]);
						farmShipmentList.emplace_back(fShip);
						if (oPrem->Farm::get_status()=="inf" && dPrem->Farm::get_status()=="sus"){
							infFarmShips.emplace_back(fShip);}
					}
				} else {std::cout << "Method for farmFarmShipments not recognized." << std::endl;}
				} // end if there are premises in origin and dest counties
			} // end if destination FIPS has susceptibles
		} // end if origin FIPS has infectious
	} // end for each county-level shipment
	std::cout<<farmShipmentList.size()<<" total farm shipments."<<std::endl;
}

void Shipment_manager::checkShipTrans(std::vector<std::tuple<int,int,int,std::string,bool,bool>>& farmShips, 
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

std::string Shipment_manager::formatOutput(int shipRes, int t)
{
// formats one line for output to existing file
	std::string toPrint;
	char temp[10];

	if (shipRes==0){ // premises-to-premises shipments
		std::cout<<farmShipmentList.size()<<" farm shipments."<<std::endl;
		if (farmShipmentList.size()>0){
			if (t==1){ // additional steps to print column headings if this is first output
				toPrint += "Time\t";
				toPrint += "OriginID\t";
				toPrint += "DestID\t";
				toPrint += "NumShips\t";
				toPrint += "Species\t";
				toPrint += "Banned\t";
				toPrint += "Compliant";
				toPrint.replace(toPrint.end()-1, toPrint.end(), "\n"); // add line break at end
			}
		
			for (auto& ships:farmShipmentList){
				sprintf(temp, "%d\t", t);
				toPrint += "\t";
				sprintf(temp, "%d\t", std::get<0>(ships));
				toPrint	+= temp; // origin ID
				sprintf(temp, "%d\t", std::get<1>(ships));
				toPrint	+= temp; // destination ID
				sprintf(temp, "%d\t", std::get<2>(ships));
				toPrint	+= temp; // shipment volume
				toPrint	+= std::get<3>(ships);
				toPrint += "\t"; // species
				sprintf(temp, "%d\t", std::get<4>(ships));
				toPrint	+= temp; // banned
				sprintf(temp, "%d\t", std::get<5>(ships));
				toPrint	+= temp; // compliant
 				toPrint.replace(toPrint.end()-1, toPrint.end(), "\n"); // add line break at ends
			} // end "for each shipment in list"
		} // end "if there are any shipments"
	} else if (shipRes==1){
		if (countyShipmentList.size()>0){
			// time, origin FIPS, dest FIPS, shipment volume, ban true/false
			if (t==1){ // additional steps to print column headings if this is first output
				toPrint += "Time\t";
				toPrint += "OriginFIPS\t";
				toPrint += "DestFIPS\t";
				toPrint += "NumShips\t"; //sum
				toPrint += "Species\t";				
				toPrint += "Banned\t";
				toPrint += "NumCompliant\t"; //sum
				toPrint.replace(toPrint.end()-1, toPrint.end(), "\n"); // add line break at end
			}
		
			for (auto& ships:countyShipmentList){
				sprintf(temp, "%d\t", t);
				toPrint += temp;
				toPrint	+= std::get<0>(ships); // origin FIPS
				toPrint += "\t";
				toPrint	+= std::get<1>(ships); // destination FIPS
				toPrint += "\t";
				sprintf(temp, "%d\t", std::get<2>(ships));
				toPrint	+= temp; // shipment volume
				toPrint += std::get<3>(ships); // species
				toPrint += "\t";
				sprintf(temp, "%d\t", std::get<4>(ships));
				toPrint	+= temp; // banned
				toPrint.replace(toPrint.end()-1, toPrint.end(), "\n"); // add line break at ends
			} // end "for each shipment in list"
		} // end "if there are any shipments"

	} else { std::cout<< "Resolution level for printing shipments invalid."<<std::endl; }

	return toPrint;
}
