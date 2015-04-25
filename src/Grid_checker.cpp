// Grid_checker.cpp

#include "Grid_checker.h"

Grid_checker::Grid_checker(const std::unordered_map<int, grid_cell*>* in_allCells, 
	std::unordered_map< Farm*, std::vector< std::tuple<Farm*,int> >>* in_sources)
	:
	allCells(in_allCells),
	sources(in_sources)
// makes shallow copy of grid cells to start as susceptible - we only modify the vector of pointers to Farms, not the Farms themselves
{
	verbose = verboseLevel;
	int fcount = 0;
	// initially copy all cells (containing all farms) into susceptible
	susceptible.reserve(allCells->size());
	for (auto& c:(*allCells)){
		susceptible.emplace_back(new grid_cell(*(c.second))); // copy c and make new pointer to copy
		fcount += c.second->get_num_farms();
	}
	std::sort(susceptible.begin(),susceptible.end(),sortByID<grid_cell*>);
	
if (verbose>1){std::cout<<"Grid checker constructed. "<<fcount<<" initially susceptible farms in "
	<<susceptible.size()<<" cells."<<std::endl;}
}

Grid_checker::~Grid_checker()
{
	for (auto& s:susceptible){delete s;}
}

void Grid_checker::stepThroughCells(std::vector<Farm*>& focalFarms, 
	std::vector<Farm*>& nonSus)
// in_focalFarms is all infectious premises
// nonSus is all non-susceptible premises, including focalFarms
{
//================================= update susceptible (vector of) grid_cells*
	if (nonSus.size()>0){
		if (verbose>2){std::cout<<"Removing "<<nonSus.size()<<" non-susceptible farms by ID."<<std::endl;}
		std::unordered_map<int, std::vector<int>> nonSusCellMap;
		for (auto& ns:nonSus){ // ns is Farm*
			nonSusCellMap[ns->Farm::get_cellID()].emplace_back(ns->Farm::get_id());
		}
		std::vector<grid_cell*> empties;
		for (auto& s:susceptible){
			int sID = s->get_id(); // cell id
			if (nonSusCellMap.count(sID)==1){ // if this cell is represented in nonSus
if (verbose>2){std::cout<<"Removing matches for cell "<<sID<<std::endl;}
				s->removeFarmSubset(nonSusCellMap.at(sID)); // remove those farms from sus cell
				if (s->get_num_farms()==0){empties.emplace_back(s);}
			}
		}
		// remove any cells that have no (susceptible) farms left
		if (empties.size()>0){
			auto newSEnd = std::remove_if(susceptible.begin(),susceptible.end(),isInList<grid_cell*>(empties));
if (verbose>1){
	int cCount = std::distance(newSEnd,susceptible.end());
	std::cout<<"Removing "<<cCount<<" cell(s) no longer susceptible."<<std::endl;
}
			susceptible.erase(newSEnd, susceptible.end());
		}
if (verbose>1){
	int scount = 0;
	for (auto& s:susceptible){scount += s->get_num_farms();}
	std::cout<<"Susceptible cells updated, now contain "<<scount<<" farms."<<std::endl;}
		
 	}
	
//================================= loop through pairs of inf farms and sus grid_cells
	  // for each focal farm
	  for (auto& f1:focalFarms){ 
	  	int fcID = f1->Farm::get_cellID();
	  	grid_cell* fc = allCells->at(fcID);
if (verbose>2){std::cout<<"Focal farm "<<f1->Farm::get_id()<<" in cell "<<fcID<<std::endl;}
		for (auto& c2:susceptible){
			int ccID = c2->grid_cell::get_id();
			if (fc->canInfect(ccID)){ // check if cell-cell tx possible
if (verbose>2){std::cout<<"Checking in-range comparison cell "<<ccID<<std::endl;}
			// Evaluation via gridding
			std::vector<Farm*> fToCellExp;
			binomialEval(f1,fc,c2,ccID,fToCellExp);
			// record sources of infection
			for (auto& exp1:fToCellExp){ 
				(*sources)[exp1].emplace_back(std::make_tuple(f1,0)); // exposed farm is key, value is exposure source (0=local)
			}
			// add to exposed list from other farm-cell comparisons if not already present
			for (auto& exp2:fToCellExp){
				if (!isWithin<Farm*>(exp2,exposed)){
					exposed.emplace_back(exp2);
				}
			}
			
/*			if (pairwiseOn){
				double i = f1->Farm::get_inf();
				double pmax = oneMinusExp(-i * (fc->grid_cell::kernelTo(ccID)));
				pairwise pw(f1,farmsToCheck,pmax); // makes all pw calculations
				// check if same farms were infected in gridding and pairwise
				std::string pwCheck = pw.compare(farmToCellExposures); // tab delimited: # in agreement, # gridding only, # pairwise only	
				std::string outfile = "PWcompare.txt";
				printLine(outfile,pwCheck); //shared.cpp	
			} // end if pairwiseOn
*/
			} // end if fc can infect cc
		} // end for loop through comparison cells
	  } // end for each focal farm

}

void Grid_checker::binomialEval(Farm* f1, grid_cell* fc, grid_cell* c2, int ccID,
	std::vector<Farm*>& output)
{
	// calculate pmax & N
	// draw from binomial
	// randomly choose that many farms
	// evaluate adjusted probs	
	
	double focalInf = f1->Farm::get_inf();
	double kern = fc->grid_cell::kernelTo(ccID);
	double pmax = oneMinusExp(-focalInf * kern);
	double N = c2->get_num_farms();
	std::vector<Farm*> fcexp; fcexp.reserve(N);
	
	// draw number of hypothetical farms exposed, from binomial
	int numExp = draw_binom(N,pmax);
	
	if (numExp == 0){ // no infected premises in this cell
	} else if (numExp > 0){
		// randomly choose numExp farms
		std::vector<Farm*> hypExposed; // hypothetically exposed
		std::vector<Farm*> compFarms = c2->get_farms();
		random_unique(compFarms,numExp,hypExposed); 
		// evaluate each of the randomly selected farms
		for (auto& f2:hypExposed){
			// calc actual probabilities
			double f1x = f1 -> Farm::get_x();
			double f1y = f1 -> Farm::get_y();
			double f2x = f2 -> Farm::get_x();
			double f2y = f2 -> Farm::get_y();
			double xdiff = (f1x - f2x);
			double ydiff = (f1y - f2y);
			double distBWfarmssq = xdiff*xdiff + ydiff*ydiff;
			double kernelBWfarms = kernelsq(distBWfarmssq); // kernelsq calculates kernel based on distance squared
			double compSus = f2->Farm::get_sus(); // susceptible farm in comparison cell

			// calculate probability between these specific farms
			double ptrue = oneMinusExp(-focalInf * compSus * kernelBWfarms); // prob tx between this farm pair

			double random = unif_rand();
			if (random <= ptrue/pmax){ // actual infection
if(verbose>1){std::cout << "Infection @ distance: "<< std::sqrt(distBWfarmssq)/1000 << " km, prob "<<ptrue<<std::endl;}
				fcexp.emplace_back(f2);
			}
		 } // end "for each hypothetically exposed farm"
		} // end "if any hypothetically exposed farms"
	fcexp.swap(output);
}
/*
std::vector<Farm*> Grid_manager::countdownEval(Farm* focalFarm, std::vector<Farm*> compFarms)
{
	std::vector<Farm*> exposedFarmsInCell;
	exposedFarmsInCell.reserve(compFarms.size());
	
	double focalInf = focalFarm->Farm::get_inf();
	grid_cell* focalCell = allCells.at(focalFarm->Farm::get_cellID());
	grid_cell* compCell = allCells.at(compFarms[0]->Farm::get_cellID());
	int N = compFarms.size();
	double pcell = 1 - exp(-focalInf * susxKern[focalCell][compCell] * N); // susxKern is max susceptibility in comp cell times kernel @ distance betweeen focal and comp cell

	double s = 1; // on/off switch, 1 = on (single hypothetical infection hasn't happened yet)
	double random1 = unif_rand();
// Grid checkpoint A
	if (random1 <= pcell){ // if farm to cell succeeds
 		int f2count = 1; // how many farms in comparison cell have been checked
		// "prob6" in MT's Fortran code:
		double pmax = 1 - exp(-focalInf * susxKern[focalCell][compCell]);
		for (auto& f2:compFarms){
			double pcellAdj =1-(s*exp(-focalInf * susxKern[focalCell][compCell] * (N+1-f2count)));
			double random2 = unif_rand(); // "prob4" in MT's Fortran code
// Grid checkpoint B
			if (random2 <= pmax/pcellAdj){
			// if (one max susceptible)/(entrance prob accounting for # of farms checked) succeeds
			s = 0; // remainingFarmProb recalculates to 1 for remainder of loop
			// get actual distances between farms
			double f1x = focalFarm -> Farm::get_x();
			double f1y = focalFarm -> Farm::get_y();
			double f2x = f2 -> Farm::get_x();
			double f2y = f2 -> Farm::get_y();
			double xdiff = (f1x - f2x);
			double ydiff = (f1y - f2y);
			double distBWfarmssq = xdiff*xdiff + ydiff*ydiff;
			double kernelBWfarms = kernelsq(distBWfarmssq); // kernelsq calculates kernel based on distance squared
			double compSus = f2->Farm::get_sus(); // susceptible farm in comparison cell (farmInf already defined from focal cell)

			// calculate probability between these specific farms
			// "prob3" in MT's Fortran code
			double ptrue = 1-exp(-focalInf * compSus * kernelBWfarms);
// Grid checkpoint C
			if (random2 <= ptrue/pcellAdj){
				// infect
				if(verbose>1){
					std::cout << "Infection @ distance: ";
					std::cout << std::sqrt(distBWfarmssq)/1000 << ", prob "<<ptrue<<std::endl;
				}
				exposedFarmsInCell.emplace_back(f2);
			}
		 } // end "if farm hypothetically exposed"
		 f2count++;
		} // end "for each comparison farm"
	} // end "if >1 hypothetical infection"
	return exposedFarmsInCell;
}
*/