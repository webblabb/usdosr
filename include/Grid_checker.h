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

		void binomialEval(Farm*,grid_cell*,grid_cell*,int,std::vector<Farm*>&);
		// arguments: focal farm, focal cell, comp cell, comp cell ID, output
		
	public:
		Grid_checker(const std::unordered_map<int, grid_cell*>*,  // allCells
			std::unordered_map< Farm*, std::vector< std::tuple<Farm*,int> >>*); //sources
		~Grid_checker();
		
		void stepThroughCells(
			std::vector<Farm*>&, // infectious
			std::vector<Farm*>&); //non-susceptible
			
		void get_exposed(std::vector<Farm*>&); //inlined
};

inline void Grid_checker::get_exposed(std::vector<Farm*>& output){
	exposed.swap(output);}

#endif