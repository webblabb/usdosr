#include <cmath> // floor
#include <iostream> // for error-checking output
#include "Shipment_manager.h" // includes Farm, shared_functions

///	\param[in]	in_FIPSmap		A map of FIPS codes to farms
///	\param[in]	fipsSpMap		Sorted populations of species on farms
///	\param[in]	in_S			Pointer to Status_manager instance for this replicate
///	\param[in]	ffm				farm assignment method
///	\param[in]	speciesOnPrems	List of species
Shipment_manager::Shipment_manager(
	const std::unordered_map<std::string, std::vector<Farm*>>* in_FIPSmap,
	const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Farm*> >>* fipsSpMap, 
	Status_manager* in_S,
	int ffm,
	const std::vector<std::string>& speciesOnPrems
	)
	:
	FIPSmap(in_FIPSmap),
	fipsSpeciesMap(fipsSpMap),
	S(in_S),
	farmFarmMethod(ffm),
	species(speciesOnPrems)
{	
	verbose = verboseLevel;
	
	allFIPS.reserve(FIPSmap->size());
	
	// copy species
	// record allFIPS: vector of all possible shipment destinations (counties)
	// used in generating random shipments
	for (auto& f:(*FIPSmap)){ // f.first is FIPS, f.second is vector of Farms*s
		allFIPS.emplace_back(f.first);
		
		for (auto& s:species){// can remove when USAMM is implemented
			if (fipsSpeciesMap->at(f.first).count(s)==1){speciesFIPS[s].emplace_back(f.first);}
		}
	}

	if(verbose>0){std::cout << "Shipment manager constructed: "<<FIPSmap->size()<<" counties with premises." << std::endl;}
}

Shipment_manager::~Shipment_manager()
{
	for (auto& fs:farmShipmentList){delete fs;}
}

/// Makes shipments for ALL premises in counties, not just inf/susc provided in arguments.
/// County method determines shipments kernel (can change by time).
/// infFarms determines which farm-level assignments to make.
void Shipment_manager::makeShipments(std::vector<Farm*>& infFarms, int countyMethod, std::vector<shipment*>& fs)
{
  if (countyMethod != 0){
	startRecentShips = farmShipmentList.size();
	startCoRecentShips = countyShipmentList.size();
	
	// get counties (by FIPS codes) of infectious farms
	std::set<std::string> FIPSofConcern;
	for (auto& i:infFarms){
		FIPSofConcern.emplace(i->Farm::get_fips()); // unique FIPS values
	}
if(verbose>1){std::cout << FIPSofConcern.size()<<" counties of concern from which to generate shipments."<< std::endl;}

	// generate county-county shipments for each origin county
	for (auto& oc:FIPSofConcern){ // oc = origin county
if(verbose>1){std::cout <<"Making shipment(s) for county "<<oc<<std::endl;}
		countyCountyShipments(oc, countyMethod); // writes to countyShipmentList
	}
	
if(verbose>1){std::cout << countyShipmentList.size()-startCoRecentShips <<" total shipments to assign to farms. "<<std::endl;}
	farmFarmShipments(); // reads from countyShipmentList, writes to farmShipmentList
	// return the farm shipments from this round
	std::vector<shipment*>::iterator it = farmShipmentList.begin()+startRecentShips;
	std::vector<shipment*> output (it, farmShipmentList.end());
	output.swap(fs);
  }
}

void Shipment_manager::countyCountyShipments(std::string oCounty, int method)
{
	switch (method)
	{
		case 0:
			break; // no shipments
		
		case 1:{
		//  randomly assign destination counties
			double random = unif_rand();
			bool makeShipment = (random < 0.99); // proportion of all counties make a shipment
			if (makeShipment){
				std::vector<std::string> possibleSp;
				for (auto& ps:fipsSpeciesMap->at(oCounty)){
					possibleSp.emplace_back(ps.first);
				}
				std::string sp = randomFrom(possibleSp);
				std::string dCounty = randomFrom(speciesFIPS.at(sp));
				
				int volume = 1;
				countyShipmentList.emplace_back(coShipment {0,oCounty,dCounty,sp,volume,0});
			}
			break;
		}
			
		default:{
			std::cout << "Method for countyCountyShipments not recognized." << std::endl;
		}
	}
}

Farm* Shipment_manager::largestStatus(std::vector<Farm*>& premVec, std::string& status)
{
	bool found = 0;
	auto i = premVec.back(); // start at end of sorted vector (largest prem) and work backwards
	while (found==0){
		if(S->get_status(i).compare(status)==0){ // if prem has this status, stop and return this prem
			found = 1;
		} else if ( i>premVec.front() ){i--; // keep moving backwards
		} else if ( i==premVec.front() ){ i = premVec.back(); found = 1;} // if no prems have this status, return largest
	}
	return i;
}

/// Assigns shipments to farms from one county to another, distributed:
/// randomly (method=0), 
/// with probability related to farm size (method=1)
/// from biggest farms in county to random destinations (method=2)
/// from random destinations to biggest farm in county (method=3)
/// distribute only between biggest farms in each county (method = 4)
/// Leaves empty placeholder for ban status - only accounted for when changing status in main.
void Shipment_manager::farmFarmShipments()
{
	std::vector<coShipment>::iterator cit = countyShipmentList.begin()+startCoRecentShips;
	// use only the most recent county shipments
	for (auto& s = cit; s!=countyShipmentList.end(); ++s){
		std::string oFIPS = s->origFIPS;
		std::string dFIPS = s->destFIPS;
		std::string sp = s->species;
		int volume = s->volume;
std::cout<<"Assigning shipment: "<<oFIPS<<", "<<dFIPS<<", "<<sp<<", "<<volume<<std::endl;
				
		// get all farms with species sp in counties - can be sent to/from any farm regardless of status
		std::vector<Farm*> oPrems = fipsSpeciesMap->at(oFIPS).at(sp); // already sorted by population
		std::vector<Farm*> dPrems = fipsSpeciesMap->at(dFIPS).at(sp);
if(verbose>1){std::cout<<"Origin FIPS: "<<oFIPS<<" has "<<oPrems.size()<<" prems, destination FIPS: "<<dFIPS<<" has "<<dPrems.size()<<std::endl;}

		// sizes of prems with this species, sorted by population size
		std::vector<int> oPremSizes, dPremSizes;
		for (auto& ps:oPrems){oPremSizes.emplace_back(ps->Farm::get_size(sp));} // store population
		 // if origin and destination FIPS are the same
		if (oFIPS==dFIPS){
			dPremSizes = oPremSizes;
		} else {
			for (auto& ps:dPrems){dPremSizes.emplace_back(ps->Farm::get_size(sp));} // store population
		}
		
		// check if there are 0 premises in county - prob wouldn't happen in USAMM? For now, skip that shipment
		if (oPrems.size()==0 || dPrems.size()==0){
			std::cout<<"County without proper premises type, skipping shipment."<<std::endl;
		} else {
		// assignment
		switch (farmFarmMethod)
		{
			case 0:{ // random
				std::vector<Farm*> origPrems;
				random_unique(oPrems,volume,origPrems); // pick v random premises, write to origPrems
				std::vector<Farm*> destPrems;
				random_unique(dPrems,volume,destPrems);
				for (int v = 0; v!= volume; v++){ // for each shipment
					 farmShipmentList.emplace_back( new shipment {0, // time (will be filled by Status manager)
						origPrems[v]->get_id(), // origin
						destPrems[v]->get_id(), // destination
						oFIPS,
						dFIPS,
						sp, // species
						0}); // ban (will be filled by Status manager)
				}
				break;
			}	
			case 1:{ // shipment probability based on relative farm size				
				// get cumulative sums of sizes for farms in county to distribute probabilities
				std::vector<int> oCumSums = {0};
				std::vector<int> dCumSums = {0};
				for (unsigned int oi=0; oi!=oPremSizes.size(); oi++){ //oi = origin iterator
					oCumSums.emplace_back(oCumSums[oi-1]+oPremSizes[oi]);} 
				for (unsigned int di=0; di!=dPremSizes.size(); di++){ //di = destination iterator
					dCumSums.emplace_back(dCumSums[di-1]+dPremSizes[di]);} 
					
				for (int v=0; v!=volume; v++){
					int oRand = unif_rand()*oCumSums.back(); // scale random up to maximum
					int dRand = unif_rand()*dCumSums.back(); // scale random up to maximum
					int oElement = whichElement(oRand,oCumSums); // whichElement is in shared_functions.h
					int dElement = whichElement(dRand,dCumSums); // whichElement is in shared_functions.h
					Farm* oPrem = oPrems[oElement];
					Farm* dPrem = dPrems[dElement];
					farmShipmentList.emplace_back( new shipment {0, // time (filled later)
						oPrem->get_id(),
						dPrem->get_id(),
						oFIPS,
						dFIPS,
						sp,
						0}); // ban (filled later)
				}
				break;
			}				
			case 2:{ // shipped from biggest infectious farm to random destinations
				for (int v = 0; v!= volume; v++){ // for each shipment between these counties
					std::string infstring = "inf";
					Farm* oPrem = largestStatus(oPrems,infstring);
					Farm* dPrem = randomFrom(dPrems);
					farmShipmentList.emplace_back( new shipment {0, // time (filled later)
						oPrem->get_id(),
						dPrem->get_id(),
						oFIPS,
						dFIPS,
						sp,
						0}); // ban (filled later)
				}
				break;
			}	
			case 3:{ // shipped from random destinations to the biggest susceptible farm
				for (int v = 0; v!= volume; v++){ // for each shipment between these counties
					std::string susstring = "sus";
					Farm* oPrem = randomFrom(oPrems);
					Farm* dPrem = largestStatus(dPrems,susstring);
					farmShipmentList.emplace_back( new shipment {0, // time (filled later)
						oPrem->get_id(),
						dPrem->get_id(),
						oFIPS,
						dFIPS,
						sp,
						0}); // ban (filled later)
				}
				break;
			}	
			case 4:{ // shipped between biggest farms
				for (int v = 0; v!= volume; v++){ // for each shipment between these counties
					std::string infstring = "inf";
					std::string susstring = "sus";
					Farm* oPrem = largestStatus(oPrems,infstring);
					Farm* dPrem = largestStatus(dPrems,susstring);
					farmShipmentList.emplace_back(new shipment {0, // time (filled later)
						oPrem->get_id(),
						dPrem->get_id(),
						oFIPS,
						dFIPS,
						sp,
						0}); // ban (filled later)
				}
				break;
			}	
			default:{
				std::cout << "Method for farmFarmShipments not recognized." << std::endl;
			}
		} // end switch-case
		} // end if there are premises in origin and dest counties
	} // end for each county-level shipment
	std::cout<<farmShipmentList.size()-startRecentShips<<" total farm shipments."<<std::endl;
}

