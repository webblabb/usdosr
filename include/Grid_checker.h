// Grid_checker.h

#ifndef Grid_checker_h
#define Grid_checker_h

#include "grid_cell.h"
#include "shared_functions.h"

extern int verboseLevel;

class Grid_checker
{
	private:
		int verbose;
		// own copies of cells with farms of disease status
		std::vector<grid_cell*> susceptible;
		std::vector<Farm*> exposed; // called from main each timestep and cleared
		// pointer to Grid_manager object
		const std::unordered_map<int, grid_cell*>* allCells; // used for reference for infectious
		// pointer to Status_manager object
		std::unordered_map< Farm*, std::vector< std::tuple<Farm*,int> >>* sources;
		double k1, k2, k3, k2tok3; // kernel parameters
		double kernelsq(double);
		void binomialEval(Farm*,grid_cell*,grid_cell*,int,std::vector<Farm*>&);
		// arguments: focal farm, focal cell, comp cell, comp cell ID, output

		
	public:
		Grid_checker(const std::unordered_map<int, grid_cell*>*,  // allCells
			std::unordered_map< Farm*, std::vector< std::tuple<Farm*,int> >>*, //sources to write to
			std::vector<double>&); //sources
		~Grid_checker();
		
		void stepThroughCells(
			std::vector<Farm*>&, // infectious
			std::vector<Farm*>&); //non-susceptible
			
		void take_exposed(std::vector<Farm*>&); //inlined - called from main and cleared each timestep
};

inline void Grid_checker::take_exposed(std::vector<Farm*>& output){
	exposed.swap(output);}

template<typename T> 
struct isInList // used in grid checker for grid_cell*
{
	isInList(const std::vector<T> in_list) : itemList(in_list) {} // constructor
	bool operator() (const T item){ // overload operator function
		return (isWithin(item,itemList));
	}	
	private:
		std::vector<T> itemList; // member
};

#endif