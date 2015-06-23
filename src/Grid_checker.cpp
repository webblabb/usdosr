#include "Grid_checker.h"

/// Makes shallow copy of grid_cells to start as susceptible. Only the vector of pointers 
/// to Farms is modified, not the Farms themselves (hence the shallow copy). Statuses are
/// only actually changed in Status_manager.
Grid_checker::Grid_checker(const std::unordered_map<int, grid_cell*>* in_allCells, 
	std::unordered_map< Farm*, std::vector< std::tuple<Farm*,int> >>* in_sources,
	Local_spread* k)
	:
	allCells(in_allCells),
	sources(in_sources),
	kernel(k)
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

/// Updates static list of cells with susceptible premises within. After transmission
/// evaluation, records sources of exposure in "sources" map from Status_manager and
/// records exposures in "exposed" vector, later accessed by Status_manager
/// \param[in] focalFarms	All currently infectious premises
/// \param[in] nonSus		All currently non-susceptible premises that cannot become exposed, including focalFarms
void Grid_checker::stepThroughCells(std::vector<Farm*>& focalFarms, 
	std::vector<Farm*>& nonSus)
{
//================================= update susceptible (vector of) grid_cells*
	if (nonSus.size()>0){
if (verbose>1){std::cout<<"Removing "<<nonSus.size()<<" non-susceptible farms by ID."<<std::endl;}
		std::unordered_map<int, std::vector<int>> nonSusCellMap;
		for (auto& ns:nonSus){ // ns is Farm*
			nonSusCellMap[ns->Farm::get_cellID()].emplace_back(ns->Farm::get_id());
		}
		std::vector<grid_cell*> empties;
		for (auto& s:susceptible){
			int sID = s->get_id(); // cell id
			if (nonSusCellMap.count(sID)==1){ // if this cell is represented in nonSus
if (verbose>1){std::cout<<"Removing matches for cell "<<sID<<std::endl;}
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
if (verbose>0){
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
			if (fc->kernelTo(ccID)>0){ // check if cell-cell tx possible
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
			
			} // end if fc can infect cc
		} // end for loop through comparison cells
	  } // end for each focal farm

}

///	Calculates pmax of cell and N, draws h successes from binomial distribution
///	Randomly selects h farms, evaluates adjusted probabilities
/// \param[in]	f1	Infectious farm from which to evaluate transmission
///	\param[in]	fc	Focal cell containing infectious premises
///	\param[in]	c2	Comparison cell containing susceptible premises (can be same as fc)
///	\param[in]	ccID	ID of comparison cell
///	\param[out]	output	Vector of Farm*s exposed by this infectious farm
void Grid_checker::binomialEval(Farm* f1, grid_cell* fc, grid_cell* c2, int ccID,
	std::vector<Farm*>& output)
{
	double focalInf = f1->Farm::get_inf();
	double kern = fc->grid_cell::kernelTo(ccID);
	double pmax = oneMinusExp(-focalInf * kern); // Overestimated probability for any single premises
	double N = c2->grid_cell::get_num_farms();
	std::vector<Farm*> fcexp; fcexp.reserve(N); // fcexp = "focal-comparison exposures"
	
	// draw number of hypothetical farms exposed, from binomial
	int numExp = draw_binom(N,pmax);
	
	if (numExp == 0){ // no infected premises in this cell
	} else if (numExp > 0){
		// randomly choose numExp farms
		std::vector<Farm*> hypExposed; // hypothetically exposed
		std::vector<Farm*> compFarms = c2->get_farms();
		random_unique(compFarms,numExp,hypExposed); 
		// evaluate each of the randomly selected farms
if(verbose>1){std::cout<<"Pmax: "<<pmax<<", "<<hypExposed.size()<<" hypothetical infections out of "
	<<compFarms.size()<<" farms in cell."<<std::endl;}
		for (auto& f2:hypExposed){
			// calc actual probabilities
			double f1x = f1 -> Farm::get_x();
			double f1y = f1 -> Farm::get_y();
			double f2x = f2 -> Farm::get_x();
			double f2y = f2 -> Farm::get_y();
			double xdiff = (f1x - f2x);
			double ydiff = (f1y - f2y);
			double distBWfarmssq = xdiff*xdiff + ydiff*ydiff;
			double kernelBWfarms = kernel->atDistSq(distBWfarmssq); // kernelsq calculates kernel based on distance squared
			double compSus = f2->Farm::get_sus(); // susceptible farm in comparison cell

			// calculate probability between these specific farms
			double ptrue = oneMinusExp(-focalInf * compSus * kernelBWfarms); // prob tx between this farm pair
if(verbose>1){std::cout<<"Inf: "<<focalInf<<", sus: "<<compSus<<", kernel: "<<kernelBWfarms<<", Ptrue "<<ptrue<<std::endl;}
			double random = unif_rand();
			if (random <= ptrue/pmax){ // actual infection
if(verbose>1){std::cout << "Infection @ distance: "<< std::sqrt(distBWfarmssq)/1000 << " km, prob "<<ptrue<<std::endl;}
				fcexp.emplace_back(f2);
			}
		 } // end "for each hypothetically exposed farm"
		} // end "if any hypothetically exposed farms"
	fcexp.swap(output);
}

///	Calculates and evaluates probability of cell entry, then steps through each premises
///	in cell and evaluates adjusted probabilities (Keeling's method)
/// \param[in]	f1	Infectious farm from which to evaluate transmission
///	\param[in]	fc	Focal cell containing infectious premises
///	\param[in]	c2	Comparison cell containing susceptible premises (can be same as fc)
///	\param[in]	ccID	ID of comparison cell
///	\param[out]	output	Vector of Farm*s exposed by this infectious premises
void Grid_checker::countdownEval(Farm* f1, grid_cell* fc, grid_cell* c2, int ccID,
	std::vector<Farm*>& output)
{
	double focalInf = f1->Farm::get_inf();
	double kern = fc->grid_cell::kernelTo(ccID);
	double pmax = oneMinusExp(-focalInf * kern); // Overestimated probability for any single premises, "prob6" in MT's Fortran code:
	double N = c2->grid_cell::get_num_farms();
	double pcell = oneMinusExp(-focalInf * kern * N); // Probability of cell entry
	
	std::vector<Farm*> fcexp; fcexp.reserve(N); // fcexp = "focal-comparison exposures"

	double s = 1; // on/off switch, 1 = on (single hypothetical infection hasn't happened yet)
	double random1 = unif_rand();
// Grid checkpoint A
	if (random1 <= pcell){ // if farm to cell succeeds
 		int f2count = 1; // how many farms in comparison cell have been checked
 		std::vector<Farm*> compFarms = c2->get_farms();
		for (auto& f2:compFarms){
			double oneMinusExpA = oneMinusExp(-focalInf * kern * (N+1-f2count)); // 1 - exp(A)
			double pcellAdj = 1 - s + s*oneMinusExpA; // equivalent to 1 - s*exp(A)
			double random2 = unif_rand(); // "prob4" in MT's Fortran code
// Grid checkpoint B
			if (random2 <= pmax/pcellAdj){
			// if (one max susceptible)/(entrance prob accounting for # of farms checked) succeeds
			s = 0; // remainingFarmProb recalculates to 1 for remainder of loop
			// get actual distances between farms
			double f1x = f1 -> Farm::get_x();
			double f1y = f1 -> Farm::get_y();
			double f2x = f2 -> Farm::get_x();
			double f2y = f2 -> Farm::get_y();
			double xdiff = (f1x - f2x);
			double ydiff = (f1y - f2y);
			double distBWfarmssq = xdiff*xdiff + ydiff*ydiff;
			double kernelBWfarms = kernel->atDistSq(distBWfarmssq); // kernelsq calculates kernel based on distance squared
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
				fcexp.emplace_back(f2);
			}
		 } // end "if farm hypothetically exposed"
		 f2count++;
		} // end "for each comparison farm"
	} // end "if >1 hypothetical infection"
	fcexp.swap(output);
}

/// Calculates filtered pairwise transmission: only makes pairwise calculations if random 
/// number passes pmax filter
/// \param[in]	f1	Infectious farm from which to evaluate transmission
///	\param[in]	fc	Focal cell containing infectious premises
///	\param[in]	c2	Comparison cell containing susceptible premises (can be same as fc)
///	\param[in]	ccID	ID of comparison cell
///	\param[out]	output	Vector of Farm*s exposed by this infectious premises
void Grid_checker::pairwise(Farm* f1, grid_cell* fc, grid_cell* c2, int ccID,
	std::vector<Farm*>& output)
{
	double focalInf = f1->Farm::get_inf();
	double kern = fc->grid_cell::kernelTo(ccID);
	double pmax = oneMinusExp(-focalInf * kern); // Overestimate of p for all farms in this cell
	std::vector<Farm*> cFarms = c2->grid_cell::get_farms();

	std::vector<Farm*> fcexp; fcexp.reserve(cFarms.size()); // For output

	for (auto& cf:cFarms){
		double random = unif_rand();
		if (random <= pmax){
			double f1x = f1 -> Farm::get_x();
			double f1y = f1 -> Farm::get_y();
			double f2x = cf -> Farm::get_x();
			double f2y = cf -> Farm::get_y();
			double xdiff = (f1x - f2x);
			double ydiff = (f1y - f2y);
			double distBWfarmssq = xdiff*xdiff + ydiff*ydiff;
			double kernelBWfarms = kernel->atDistSq(distBWfarmssq); // kernelsq calculates kernel based on distance squared
			double compSus = cf->Farm::get_sus(); // susceptible farm in comparison cell

			// calculate probability between these specific farms
			double ptrue = oneMinusExp(-focalInf * compSus * kernelBWfarms); // prob tx between this farm pair
			if (random <= ptrue){ // actual infection
if(verbose>0){std::cout << "Infection @ distance: "<< std::sqrt(distBWfarmssq)/1000 << " km, prob "<<ptrue<<std::endl;}
					fcexp.emplace_back(cf);
			}
		}
	}
	fcexp.swap(output);
}