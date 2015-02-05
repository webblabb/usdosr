#include "pairwise.h"

pairwise::pairwise(Farm* f,std::vector<Farm*>& comp)
{
	verbose = 1;
	if (verbose>1){std::cout<<"Computing pairwise comparisions from focal farm "<<f->Farm::get_id()
		<<" to "<<comp.size()<<" other prems."<<std::endl;}
	std::clock_t pw_start = std::clock();
	double fx = f->Farm::get_x(); // get focal x coordinate
	double fy = f->Farm::get_y(); // get focal y coordinate
	double fInf = f->Farm::get_inf(); // get focal infectiousness
	
	for (auto& c:comp){
		double cx = c -> Farm::get_x(); // get comp x coordinate
		double cy = c -> Farm::get_y(); // get comp y coordinate
		
		double xdiff = fx - cx;
		double ydiff = fy - cy;
		double distsq = xdiff*xdiff + ydiff*ydiff;
			distances[c] = distsq;
		double kernel = kernelsq(distsq);
//			k_values[c] = kernel;
		// get susceptibility value
		double cSus = c->Farm::get_sus();

		double prob = 1-exp(-fInf * cSus * kernel); // prob tx between this farm pair
//			probs[c] = prob;
		double random = unif_rand();
			randomDraws[c] = random;
		if (random <= prob){
			// success... infect
			infPrems.emplace_back(c);
			infected[c] = 1; // store bools separately for easy infection status access
		} else { // not infected
			infected[c] = 0;
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