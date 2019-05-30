#include <Rcpp.h>

#include <cmath> // floor
#include <iostream> // for error-checking output
#include <algorithm>
#include "Shipment_manager.h" // includes Farm, shared_functions
#include "Shipment_kernel.h"
#include "County.h"
#include "Status_manager.h"
#include "shared_functions.h"
#include <gsl/gsl_randist.h>

///	\param[in]	in_FIPS_map		A map of FIPS codes to farms
///	\param[in]	fipsSpMap		Sorted populations of species on farms
///	\param[in]	in_S			Pointer to Status_manager instance for this replicate
///	\param[in]	ffm				farm assignment method
///	\param[in]	speciesOnPrems	List of species
Shipment_manager::Shipment_manager(
	const std::unordered_map<std::string, County*>* in_FIPS_map,
	const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Farm*> >>* fipsSpMap,
	Status_manager* in_S,
	int ffm,
	const std::vector<std::string>& speciesOnPrems,
	const Parameters* p) :
        FIPS_map(in_FIPS_map),
        fipsSpeciesMap(fipsSpMap),
        parameters(p),
        S(in_S),
        farmFarmMethod(ffm),
        species(speciesOnPrems)
{
	verbose = verboseLevel;
	initialize();
}

Shipment_manager::Shipment_manager(
	const std::unordered_map<std::string, County*>* in_FIPS_map,
	const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Farm*> >>* fipsSpMap,
	int ffm,
	const std::vector<std::string>& speciesOnPrems,
	const Parameters* p) :
        FIPS_map(in_FIPS_map),
        fipsSpeciesMap(fipsSpMap),
        parameters(p),
        farmFarmMethod(ffm),
        species(speciesOnPrems)
{
    S = nullptr;
	verbose = verboseLevel;
	initialize();
}

Shipment_manager::~Shipment_manager()
{
	for (auto& fs:farmShipmentList){delete fs;}
	gsl_rng_free(R);
}

void Shipment_manager::initialize()
{
    //Initialize the random number generator.
    size_t seed = generate_distribution_seed();
    R = gsl_rng_alloc(gsl_rng_mt19937);
    gsl_rng_set(R, seed);

    // Determine if shipping is turned off in parameters
    shipments_on = parameters->shipments_on;
    if(shipments_on){
        allCounties.reserve(FIPS_map->size());
        // copy species
        for (auto& FIPS_county_pair:(*FIPS_map)){ // first is FIPS id, second County*
            allCounties.push_back(FIPS_county_pair.second);
            if(allStates_set.find(FIPS_county_pair.second->get_parent_state()) == allStates_set.end())
            {
                allStates_set.insert(FIPS_county_pair.second->get_parent_state());
            }
            for (auto& s:species){// can remove when USAMM is implemented
                if (fipsSpeciesMap->at(FIPS_county_pair.first).count(s)==1){speciesFIPS[s].emplace_back(FIPS_county_pair.first);}
            }
        }
        if(verbose>0){std::cout << "Shipment manager constructed: "<<FIPS_map->size()<<" counties with premises."
                                << " Kernel in use: " << parameters->shipment_kernel << "." << std::endl;}
    } else {
    	if(verbose>0){std::cout << "Shipment manager constructed but not initialized since shipments are turned off." << std::endl;}
    }
}

void Shipment_manager::makeShipmentsMultinomial(size_t timestep, size_t days_in_period, size_t days_rem,
                                                std::string time_period, std::vector<Shipment*>& output,
                                                std::vector<Farm*>& infFarms, std::vector<Farm_type*> ft_vec)
{
    //Select what farms will be involved in the genereation of shipments.
    //Running with an empty infFarms is a signal to generating a full network of shipments,
    //so then we use all farms.
    std::vector<Farm*> affected_farms;
    affected_farms.reserve(1000000);
    size_t day_of_year = get_day_of_year(timestep, parameters->start_day);
    if(infFarms.empty())
    {
        for(County* c : allCounties)
        {
            std::vector<Farm*> c_farms = c->get_farms();
            affected_farms.insert(affected_farms.end(), c_farms.begin(), c_farms.end());
        }
    }
    else
    {
        affected_farms.assign(infFarms.begin(), infFarms.end());
    }

    //Sort all affected fams according to farm type and state.
    std::map<Farm_type*, std::map<State*, std::vector<Farm*>>> affected_farms_by_ft_state;
    for(Farm* f : affected_farms)
    {
        affected_farms_by_ft_state[f->get_farm_type()][f->get_parent_state()].push_back(f);
    }

    //For each farm type and state, draw the number of shipments originating from there
    //this time step from a Poisson distribution using the sum of each individiual
    //farms shipping rate as the state-level rate. Then, assign these shipments to
    //the farms within the state using a multinomial distribution where each farm
    //is weighted by some parameter (here all farms have equal weight).
    for(auto& ft_and_state_farms_pair : affected_farms_by_ft_state)
    {
        Farm_type* ft = ft_and_state_farms_pair.first;
        for(auto& state_farms_pair : ft_and_state_farms_pair.second)
        {
            State* s = state_farms_pair.first;
            //Generate the number of shipments that originate from this state. This is done internally
            //within the state object based on the usamm version selected.
            int n_shipments = s->generate_daily_shipments(ft, days_rem);

            size_t n_affected_farms = state_farms_pair.second.size(); //These are the farms for which shipments will be generated in this state. When simulating outbreak these will be the infectious farms in the state.
            double f_weights[n_affected_farms + 1]; //Last element is weight of non-infected making a shipment.

            //Fill weight vector with origin farms weight and save the sum of weights so that prob of shipment originating from unaffected farm can be calculated.
            double affected_f_weight_sum = 0.0;
            for(int i = 0; i<n_affected_farms; i++)
            {
                double f_weight = state_farms_pair.second[i]->get_normalized_oweight(); //Sums to 1.0 for all farms within a state.
                f_weights[i] = f_weight;
                affected_f_weight_sum += f_weight;
            }

            f_weights[n_affected_farms] = 1.0 - affected_f_weight_sum; //Last weight is the sum of the weights of all farms that are not infectious.
            unsigned int f_outcome[n_affected_farms + 1];
            gsl_ran_multinomial(R, n_affected_farms + 1, n_shipments,
                                f_weights, f_outcome);
            for(size_t i = 0; i < n_affected_farms; i++)
            {
                if(f_outcome[i] > 0)
                {
                    Farm* current_farm = state_farms_pair.second[i];
                    for(size_t j = 0; j < f_outcome[i]; j++)
                    {
                        Shipment* s = generateInfectiousShipment(current_farm, timestep, day_of_year, time_period);
                        output.push_back(s);
                        farmShipmentList.push_back(s);
                    }
                }
            }
        }
    }
}

void Shipment_manager::makeNetwork(std::vector<std::string> out_fnames, Grid_manager& G)
{
    std::vector<std::ofstream*> f_vec;
    for(std::string fname : out_fnames)
    {
        std::ofstream* f = new std::ofstream(fname + ".network");
        if(f->is_open())
        {
            //Write the header to output file.
            *f << "index\toCountyId\tdCountyId\tdayOfYear\tvolume\tunused\tperiod\toStateAbbr\toStateId\tdStateAbbr\tdStateId" <<
                  //"\tdistance\tunnormProb\tdState_inflow\tdState_n\tdCounty_n\tdCounty_weight" <<
                  std::endl;
            f_vec.push_back(f);
        }
        else
        {
            std::cout << "Failed to open network generation output file: " << fname
                      << ". Exiting..." << std::endl;
            Rcpp::stop("");
        }
    }


    std::cout << "Generating shipments..." << std::endl;
    std::vector<Shipment*> shipments;
    shipments.reserve(100000);
    size_t shipment_counter = 1;
    for(int day_i = 1; day_i < parameters->timesteps+1; day_i++)
    {
        G.updateShippingParameters(day_i);
        std::string time_period = G.get_time_period();
        size_t days_in_period = G.get_days_in_period();
        size_t rem_days = G.get_rem_days_of_period();
        std::vector<Farm*> dummy_vec = {};
        makeShipmentsMultinomial(day_i, days_in_period, rem_days,
                                 time_period, shipments, dummy_vec,
                                 G.get_farm_types());
        for(Shipment* s : shipments)
        {
            Farm_type* ft = G.get_farm_type_by_name(s->species);
            size_t ft_index = ft->get_index();
            *(f_vec.at(ft_index)) << shipment_counter << "\t"
                        << s->origFIPS << "\t"
                        << s->destFIPS << "\t"
                        << s->day_of_year << "\t"
                        << s->volume << "\t"
                        << 1 << "\t"
                        << time_period << "\t"
                        << s->origState_abbrev << "\t"
                        << s->origState_id << "\t"
                        << s->destState_abbrev << "\t"
                        << s->destState_id << std::endl;
//                        << 0.0 << "\t" //Distance
//                        << 0.0 << "\t" //Probability
//                        << 0.0 << "\t" //Dest state inflow
//                        << 0 << "\t" //Dest state n farms
//                        << 0 << "\t" //Dest county n farms
//                        << 0.0 << std::endl; //Dest county inflow.
            shipment_counter += 1;
        }
        shipments.clear();
    }
    for(size_t i = 0; i > f_vec.size(); i++)
    {
        f_vec[i]->close();
        delete f_vec[i];
    }

    std::vector<Farm_type*> ft_vec = G.get_farm_types();
    for(size_t i = 0; i < ft_vec.size(); i++)
    {
        std::ofstream f(out_fnames.at(i) + ".gen");
        if(f.is_open())
        {
            f << G.get_generation_string(ft_vec.at(i));
            f.close();
        }
    }
    std::cout << "...done." << std::endl;
}

Farm* Shipment_manager::largestStatus(std::vector<Farm*>& premVec, std::string& status)
{
    if(S == nullptr)
    {
        std::cout << "This shipment manager was created without status manager. "
                  << "Use the correct constructor." << std::endl << "Exiting..."
                  << std::endl;
        Rcpp::stop("");
    }
	bool found = 0;
	auto i = premVec.back(); // start at end of sorted vector (largest prem) and work backwards
	while (found==0){
		if(S->getAny_diseaseStatus(i).compare(status)==0){ // if prem has this status, stop and return this prem
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
//void Shipment_manager::farmFarmShipments()
//{
//	std::vector<coShipment>::iterator cit = countyShipmentList.begin()+startCoRecentShips;
//	// use only the most recent county shipments
//	for (auto& s = cit; s!=countyShipmentList.end(); ++s){
//		std::string oFIPS = s->origFIPS;
//		std::string dFIPS = s->destFIPS;
//		std::string sp = s->species;
//		int volume = s->volume;
//std::cout<<"Assigning shipment: "<<oFIPS<<", "<<dFIPS<<", "<<sp<<", "<<volume<<std::endl;
//
//		// get all farms with species sp in counties - can be sent to/from any farm regardless of status
//		std::vector<Farm*> oPrems = fipsSpeciesMap->at(oFIPS).at(sp); // already sorted by population
//		std::vector<Farm*> dPrems = fipsSpeciesMap->at(dFIPS).at(sp);
//if(verbose>1){std::cout<<"Origin FIPS: "<<oFIPS<<" has "<<oPrems.size()<<" prems, destination FIPS: "<<dFIPS<<" has "<<dPrems.size()<<std::endl;}
//
//		// sizes of prems with this species, sorted by population size
//		std::vector<int> oPremSizes, dPremSizes;
//		for (auto& ps:oPrems){oPremSizes.emplace_back(ps->Farm::get_size(sp));} // store population
//		 // if origin and destination FIPS are the same
//		if (oFIPS==dFIPS){
//			dPremSizes = oPremSizes;
//		} else {
//			for (auto& ps:dPrems){dPremSizes.emplace_back(ps->Farm::get_size(sp));} // store population
//		}
//
//		// check if there are 0 premises in county - prob wouldn't happen in USAMM? For now, skip that shipment
//		if (oPrems.size()==0 || dPrems.size()==0){
//			std::cout<<"County without proper premises type, skipping shipment."<<std::endl;
//		} else {
//		// assignment
//		switch (farmFarmMethod)
//		{
//			case 0:{ // random
//				std::vector<Farm*> origPrems;
//				random_unique(oPrems,volume,origPrems); // pick v random premises, write to origPrems
//				std::vector<Farm*> destPrems;
//				random_unique(dPrems,volume,destPrems);
//				for (int v = 0; v!= volume; v++){ // for each shipment
//					 farmShipmentList.emplace_back( new Shipment {0, // time (will be filled by Status manager)
//						origPrems[v]->get_id(), // origin
//						destPrems[v]->get_id(), // destination
//						oFIPS,
//						dFIPS,
//						sp, // species
//						""}); // ban (will be filled by Status manager)
//				}
//				break;
//			}
//			case 1:{ // shipment probability based on relative farm size
//				// get cumulative sums of sizes for farms in county to distribute probabilities
//				std::vector<int> oCumSums = {0};
//				std::vector<int> dCumSums = {0};
//				for (unsigned int oi=0; oi!=oPremSizes.size(); oi++){ //oi = origin iterator
//					oCumSums.emplace_back(oCumSums[oi-1]+oPremSizes[oi]);}
//				for (unsigned int di=0; di!=dPremSizes.size(); di++){ //di = destination iterator
//					dCumSums.emplace_back(dCumSums[di-1]+dPremSizes[di]);}
//
//				for (int v=0; v!=volume; v++){
//					int oRand = uniform_rand()*oCumSums.back(); // scale random up to maximum
//					int dRand = uniform_rand()*dCumSums.back(); // scale random up to maximum
//					int oElement = whichElement(oRand,oCumSums); // whichElement is in shared_functions.h
//					int dElement = whichElement(dRand,dCumSums); // whichElement is in shared_functions.h
//					Farm* oPrem = oPrems[oElement];
//					Farm* dPrem = dPrems[dElement];
//					farmShipmentList.emplace_back( new Shipment {0, // time (filled later)
//						oPrem->get_id(),
//						dPrem->get_id(),
//						oFIPS,
//						dFIPS,
//						sp,
//						""}); // ban (filled later)
//				}
//				break;
//			}
//			case 2:{ // shipped from biggest infectious farm to random destinations
//				for (int v = 0; v!= volume; v++){ // for each shipment between these counties
//					std::string infstring = "inf";
//					Farm* oPrem = largestStatus(oPrems,infstring);
//					Farm* dPrem = randomFrom(dPrems);
//					farmShipmentList.emplace_back( new Shipment {0, // time (filled later)
//						oPrem->get_id(),
//						dPrem->get_id(),
//						oFIPS,
//						dFIPS,
//						sp,
//						""}); // ban (filled later)
//				}
//				break;
//			}
//			case 3:{ // shipped from random destinations to the biggest susceptible farm
//				for (int v = 0; v!= volume; v++){ // for each shipment between these counties
//					std::string susstring = "sus";
//					Farm* oPrem = randomFrom(oPrems);
//					Farm* dPrem = largestStatus(dPrems,susstring);
//					farmShipmentList.emplace_back( new Shipment {0, // time (filled later)
//						oPrem->get_id(),
//						dPrem->get_id(),
//						oFIPS,
//						dFIPS,
//						sp,
//						""}); // ban (filled later)
//				}
//				break;
//			}
//			case 4:{ // shipped between biggest farms
//				for (int v = 0; v!= volume; v++){ // for each shipment between these counties
//					std::string infstring = "inf";
//					std::string susstring = "sus";
//					Farm* oPrem = largestStatus(oPrems,infstring);
//					Farm* dPrem = largestStatus(dPrems,susstring);
//					farmShipmentList.emplace_back(new Shipment {0, // time (filled later)
//						oPrem->get_id(),
//						dPrem->get_id(),
//						oFIPS,
//						dFIPS,
//						sp,
//						""}); // ban (filled later)
//				}
//				break;
//			}
//			default:{
//				std::cout << "Method for farmFarmShipments not recognized." << std::endl;
//			}
//		} // end switch-case
//		} // end if there are premises in origin and dest counties
//	} // end for each county-level shipment
//	std::cout<<farmShipmentList.size()-startRecentShips<<" total farm shipments."<<std::endl;
//}

Shipment* Shipment_manager::generateInfectiousShipment(Farm* origin_farm, size_t timestep, size_t day_of_year,
                                                       const std::string& time_period)
{
    //Generate a destination county until one is found that has the type required.
    Farm_type* origin_type = origin_farm->get_farm_type();
    County* origin_county = origin_farm->get_parent_county();
    County* dest_county = origin_county->get_shipment_destination(origin_type);

    while(dest_county->get_farms(origin_type).size() == 0)
    {
if(verbose>1){std::cout << "The destination county (" << dest_county->get_id() <<
                    ") does not contain any farms of type " << origin_type->get_species() <<
                    ", but has a probability of receiving shipments of that type. " <<
                    "This is likely due to the county having an inflow parameter != 0, " <<
                    "while simultaneously no premises of this type in the FLAPS data file " <<
                    "has this county as its parent. Choosing another..." << std::endl;}
        dest_county = origin_county->get_shipment_destination(origin_type);
    }

    //Pick one random element from the dest. countys vector of farms of correct type.
    std::vector<Farm*> destination_county_farms = dest_county->get_farms(origin_type);
    Farm* destination_farm = randomFrom(destination_county_farms);
    size_t shipment_volume = 0;
    return new Shipment{static_cast<int>(timestep), // timestep of shipment
                        day_of_year,
                        origin_farm->get_id(),
                        destination_farm->get_id(),
                        origin_county->get_id(),
                        dest_county->get_id(),
                        origin_type->get_species(),
                        shipment_volume,
                        true, //infectious
                        0, // ban (filled in filter_shipExposure if applicable)
                        time_period,
                        origin_county->get_parent_state()->get_code(),
                        origin_county->get_parent_state()->get_id(),
                        dest_county->get_parent_state()->get_code(),
                        dest_county->get_parent_state()->get_id()};
}
