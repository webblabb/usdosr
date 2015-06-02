// Grid_checker.h

#ifndef Grid_checker_h
#define Grid_checker_h

#include "grid_cell.h"
#include "shared_functions.h"
#include "Local_spread.h"

extern int verboseLevel;

class Grid_checker
{
	private:
		int verbose;
		std::vector<grid_cell*> susceptible; ///> Local copy of cells, with vectors of susceptible farms within
		std::vector<Farm*> exposed; ///> List of farms most recently exposed
		const std::unordered_map<int, grid_cell*>* allCells; ///> Pointer to Grid_manager object, referenced in infection evaluation among cells
		std::unordered_map< Farm*, std::vector< std::tuple<Farm*,int> >>* sources; ///> Pointer to Status_manager object
		Local_spread* kernel;
		
		void binomialEval(Farm*,grid_cell*,grid_cell*,int,std::vector<Farm*>&); ///> Evaluates transmission from a focal farm to all susceptible farms in a cell
		void pairwise(Farm*,grid_cell*,grid_cell*,int,std::vector<Farm*>&); ///> Evaluates transmission from a focal farm to all susceptible farms in a cell

	public:
		///> Makes local copy of all grid_cells, initially set as susceptible to check local spread against
		Grid_checker(const std::unordered_map<int, grid_cell*>*,
			std::unordered_map< Farm*, std::vector< std::tuple<Farm*,int> >>*,
			Local_spread*);
		~Grid_checker();
		
		void stepThroughCells(
			std::vector<Farm*>&, // infectious
			std::vector<Farm*>&); //non-susceptible
			
		void take_exposed(std::vector<Farm*>&); // inlined - called from main and cleared each timestep
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