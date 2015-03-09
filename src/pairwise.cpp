#include "pairwise.h"

pairwise::pairwise(Farm* f,std::vector<Farm*>& comp, double maxProb)
{
	verbose = verboseLevel;
	farmID = f->Farm::get_id();
	cellID = comp.front()->get_cellID();
	if (verbose>1){std::cout<<"Computing pairwise comparisions from focal farm "<<farmID
		<<" to "<<comp.size()<<" other prems in cell "<<cellID<<std::endl;}
	std::clock_t pw_start = std::clock();
	double fx = f->Farm::get_x(); // get focal x coordinate
	double fy = f->Farm::get_y(); // get focal y coordinate
	double fInf = f->Farm::get_inf(); // get focal infectiousness
	
	for (auto& c:comp){
		double random = unif_rand();
			randomDraws[c] = random;	
		if (random <= maxProb){ // is this random number small enough to calculate the distance for?	
			double cx = c -> Farm::get_x(); // get comp x coordinate
			double cy = c -> Farm::get_y(); // get comp y coordinate
	
			double xdiff = fx - cx;
			double ydiff = fy - cy;
			double distsq = xdiff*xdiff + ydiff*ydiff;
			double kernel = kernelsq(distsq);
			// get susceptibility value
			double cSus = c->Farm::get_sus();

			double prob = 1-exp(-fInf * cSus * kernel); // prob tx between this farm pair

			if (random <= prob){
				// success... infect
				infPrems[c].emplace_back(f);
				infected.emplace(c); // save in a set for compare function
			} 
		} 
	}
	std::clock_t pw_end = std::clock();
	pwTimeMS = 1000.0 * (pw_end - pw_start) / CLOCKS_PER_SEC;
	if (verbose>1){
		std::cout<<"Time for pw comparisons (ms): "<<pwTimeMS<<std::endl;
		std::cout<<"Total infections (pairwise): "<<infPrems.size()<<std::endl;
		}
}

pairwise::~pairwise()
{
}

std::string pairwise::compare(std::vector<Farm*>& gridInf)
// compares gridding results to pairwise and formats to string for printing:
// total # of agreed upon infections, # of gridding only infections, # of pw only infections
{
	int gridOnly = 0;
	int pwOnly = 0;
	int agree = 0;
		
	if (gridInf.size()>0 && infected.size()==0){ // only grid infections
		gridOnly = gridInf.size();
		std::cout<<"All grid infections."<<std::endl;
	} else if (gridInf.size()==0 && infected.size()>0){ // only pairwise infections
		pwOnly = infected.size();
		std::cout<<"All pairwise infections."<<std::endl;
	} else if (gridInf.size()>0){ // infections in both
		// copy infected list
		std::set<Farm*> pwSet = infected;
		// step through each grid infection
		for (auto& g:gridInf){
			if (pwSet.count(g)>0){ // present in pairwise list
				agree++; // agreed upon
				pwSet.erase(pwSet.find(g));
			} else {
				gridOnly++; // gridding only
			}
		}
		pwOnly = pwSet.size(); // what's left in pwSet is pw only
	} // if infection in neither, all counts stay as 0

	std::string countString;
	if (gridOnly!=0 || pwOnly!=0){ // if they don't agree on everything, print
		char temp[5];
	
		sprintf(temp, "%d\t", farmID); // farm ID, then tab
		countString	+= temp;
		sprintf(temp, "%d\t", cellID); // cell ID, then tab
		countString	+= temp;
		sprintf(temp, "%d\t", agree); // # in agreement, then tab
		countString	+= temp;
		sprintf(temp, "%d\t", gridOnly); // # gridding only, then tab
		countString	+= temp;
		sprintf(temp, "%d", pwOnly); // # pairwise only
		countString	+= temp;
		countString.replace(countString.end()-1, countString.end(), "\n"); // add line break at end
	}
	return countString;
}