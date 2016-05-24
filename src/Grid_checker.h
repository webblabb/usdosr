#ifndef Grid_checker_h
#define Grid_checker_h

#include "grid_cell.h"
#include "shared_functions.h"
#include "Local_spread.h"

extern int verboseLevel;

///> Makes comparisons between a focal farm and comparison farms in a cell
class Grid_checker
{
	private:
		int verbose; ///< Can be set to override global setting for console output
		std::vector<grid_cell*> susceptible; ///< Local copy of cells, with vectors of susceptible farms within
		std::vector<Farm*> exposed; ///< List of farms most recently exposed
		const std::unordered_map<int, grid_cell*>* allCells; ///< Pointer to Grid_manager instance for this premises file, referenced in infection evaluation among cells
		std::unordered_map< Farm*, std::vector< std::tuple<Farm*,int> >>* sources; ///< Pointer to member of Status_manager instance for this replicate
		Local_spread* kernel;
		
		void binomialEval(Farm*,grid_cell*,grid_cell*,int,std::vector<Farm*>&); ///< Evaluates transmission from a focal farm to all susceptible farms in a cell via binomial method
		void countdownEval(Farm*,grid_cell*,grid_cell*,int,std::vector<Farm*>&); ///< Evaluates transmission from a focal farm to all susceptible farms in a cell via Keeling's "countdown" method
		void pairwise(Farm*,grid_cell*,grid_cell*,int,std::vector<Farm*>&); ///< Evaluates transmission from a focal farm to all susceptible farms in a cell pairwise

	public:
		///> Makes local copy of all grid_cells, initially set as susceptible to check local spread against
		Grid_checker(const std::unordered_map<int, grid_cell*>*,
			std::unordered_map< Farm*, std::vector< std::tuple<Farm*,int> >>*,
			Local_spread*);
		~Grid_checker();
		
		///> Function that handles actual comparisons between focal and susceptible premises.
		void stepThroughCells(
			std::vector<Farm*>&, // infectious
			std::vector<Farm*>&); //non-susceptible
		
		///> Returns list of premises exposed in the last timestep
		void take_exposed(std::vector<Farm*>&); // inlined
};

/// Get most recently exposed premises and clear list, via swap with empty vector. Called from main().
inline void Grid_checker::take_exposed(std::vector<Farm*>& output){
	exposed.swap(output);}

///> Determines if an object is present in a vector of objects. 
/// Used to remove any grid_cells that no longer contain any susceptible farms.
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