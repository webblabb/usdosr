/*
Pairwise objects store values calculated for every farm in a given cell from a focal farm
- Random numbers to use in actual probability evaluation and step 2 of gridding
- Distances calculated from focal farm to each farm in cell
- Kernel values calculated
- Time to make all pairwise computations

Print layout:
Source ID, recipient ID, distance, gridding positive, pairwise positive, out of # comparisons
*/

#ifndef PAIRWISE_H
#define PAIRWISE_H

#include <ctime> // for timing processes
#include <set>

#include "Farm.h"
#include "shared_functions.h"
// shared_functions includes iostream, sstream, string, vector

extern int verboseLevel;

class pairwise
{
	private:
		int verbose;
		std::unordered_map<Farm*, double> randomDraws;
		double pwTimeMS;
		std::unordered_map<Farm*, std::vector<Farm*>> infPrems;
		std::set<Farm*> infected; // same as infPrems but as a set for compare func
		int farmID;
		int cellID;
	
	public:
		pairwise(Farm*,std::vector<Farm*>&, double);
		~pairwise();
		double get_draw(Farm*) const; // inlined
// 		bool get_infBool(Farm*) const; // inlined
		double get_pwTime() const; // inlined
		std::unordered_map<Farm*, std::vector<Farm*>> get_infPrems() const; // inlined
		std::string compare(std::vector<Farm*>&);
		// string contains farmID, cellID, # agreements, # grid only, # pw only
};

inline double pairwise::get_draw(Farm* c) const
{
	return randomDraws.at(c);
}

// inline bool pairwise::get_infBool(Farm* c) const
// {
// 	return infected.at(c);
// }

inline double pairwise::get_pwTime() const
{
	return pwTimeMS;
}

inline std::unordered_map<Farm*, std::vector<Farm*>> pairwise::get_infPrems() const
{
	return infPrems;
}

#endif //PAIRWISE_H