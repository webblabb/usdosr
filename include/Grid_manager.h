#ifndef Grid_manager_h
#define Grid_manager_h

#include "file_manager.h" // for parameter struct
#include "grid_cell.h"
//#include "pairwise.h"

#include <algorithm> // std::sort, std::any_of
#include <queue> // for checking neighbor cells
#include <stack>
#include <tuple>
#include <unordered_map>
#include <utility> // std::pair

extern int verboseLevel;

///>  Creates grid cells determined by local farm density, stores relevant values with Farms
class Grid_manager
{
	private:
		int verbose; ///< Can be set to override global setting for console output
		
		// variables for grid creation
		unsigned int maxFarms; ///< Threshold number of premises per cell (cell size takes precedence)
		std::unordered_map<int, grid_cell*> 
			allCells; ///< Unordered_map of all cells in grid
		std::unordered_map<int, Farm*> 
			farm_map; ///< Unordered_map of all premises objects
		std::unordered_map<std::string, std::vector<Farm*>>
			FIPSmap; ///< Key is fips code, value is vector of farms within
		std::unordered_map<std::string, 
			std::unordered_map< std::string, std::vector<Farm*> >> fipsSpeciesMap;
			///< key is fips code, then species name, then sorted by population size
 		std::vector<Farm*> 
 			farmList; ///< vector of pointers to all farms (deleted in chunks as grid is created)
		std::tuple<double,double,double,double> 
			xylimits; ///< Ranges of premises coordinates: [0]x min, [1]x max, [2]y min, [3]y max

		// variables for infection evaluation			
		std::vector<std::string> speciesOnPrems; ///< List of species on all farms provided in premises file
		std::unordered_map<std::string,double> susExponents; ///< Species-specific susceptibility exponents, in same order as speciesOnAllFarms
		std::unordered_map<std::string,double> infExponents; ///< Species-specific infectiousness exponents, in same order as speciesOnAllFarms
		std::unordered_map<std::string,double> susValues; ///< Species-specific susceptibility values, in same order as speciesOnAllFarms
		std::unordered_map<std::string,double> infValues; ///< Species-specific infectiousness values, in same order as speciesOnAllFarms
		std::unordered_map<std::string,double> normInf; ///< Normalized species-specific infectiousness values, in same order as speciesOnAllFarms
		std::unordered_map<std::string,double> normSus; ///< Normalized species-specific susceptibility values, in same order as speciesOnAllFarms
		Local_spread* kernel;

		unsigned int committedFarms; ///< Used to double-check that all loaded premises were committed to a cell
		int printCellFile;
		std::string batch; ///< Cells printed to file with name: [batch]_cells.txt
		
		// functions		
		void set_maxFarms(unsigned int in_maxFarms); //inlined
		std::string to_string(grid_cell&) const;
		std::vector<Farm*> getFarms(
			std::tuple<int,double,double,double>& cellSpecs,
			const unsigned int maxFarms=0); ///< Makes list of farms in a cell (quits early if over max)
		void removeParent(
			std::stack< std::tuple<int,double,double,double> >& queue);///< Removes 1st item in queue
		void addOffspring(
			std::tuple<int,double,double,double> cellSpecs, 
			std::stack< std::tuple<int,double,double,double> >& queue);
		void commitCell(
			std::tuple<int,double,double,double> cellSpecs, 
			std::vector<Farm*>& farmsInCell); ///< Adds grid_cell to allCells
		void splitCell(
			std::tuple<int,double,double,double>& cellSpecs, 
			std::stack< std::tuple<int,double,double,double> >& queue); ///< Replaces parent cell with subdivided offspring quadrants
 		void assignCellIDtoFarms(int cellID, std::vector<Farm*>& farmsInCell);
 		void removeFarmSubset(std::vector<Farm*>&, std::vector<Farm*>&); ///< Remove farms in first vector from second vector

		// functions for infection evaluation
		double shortestCellDist2(
			grid_cell* cell1, 
			grid_cell* cell2); ///< Calculates (shortest distance between two cells)^2
		void makeCellRefs(); ///< Calculates and stores kernel values and other pre-processing tasks
		void set_FarmSus(Farm*); ///< Calculates premises susceptibility and stores in Farm
		void set_FarmInf(Farm*); ///< Calculates premises infectiousness and stores in Farm
		
	public:
		Grid_manager(const parameters*);
			
		~Grid_manager();
		
		// main function that splits/commits cells:
		// 1st way to initiate a grid: specify maximum farms per cell, min size
		void initiateGrid(
			const unsigned int, 
			const int); 
		
		// 2nd way to initiate a grid: specify file containing cell specs
		void initiateGrid(
			std::string &cname);
		
		// 3rd way to initiate a grid: specify side length (same units as x/y) for uniform cells
		void initiateGrid(
			double cellSide);	

		const std::unordered_map<int, grid_cell*>*
			get_allCells(); //inlined	
			
		const std::unordered_map<int, Farm*>* 
			get_allFarms(); //inlined
			
		const std::unordered_map<std::string, std::vector<Farm*>>* 
			get_FIPSmap(); //inlined
			
		const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Farm*> >>* 
			get_fipsSpeciesMap(); //inlined
			
		void printCells();

			
};

/// "Compare" function to sort farms by x-coordinate,
/// used when assigning farms to uniform cell grid
inline bool sortByX(const Farm* farm1, const Farm* farm2)
{
	return (farm1 -> get_x()) < (farm2 -> get_x());
}

inline void Grid_manager::set_maxFarms(unsigned int in_maxFarms)
{
	maxFarms = in_maxFarms;
}

inline const std::unordered_map<int, grid_cell*>* 
	Grid_manager::get_allCells()
{
	return &allCells;
}

inline const std::unordered_map<int, Farm*>*
	Grid_manager::get_allFarms()
{
	return &farm_map;
}

inline const std::unordered_map<std::string, std::vector<Farm*>>* 
	Grid_manager::get_FIPSmap()
{
	return &FIPSmap; 
}

inline const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Farm*> >>* 
	Grid_manager::get_fipsSpeciesMap()
{
	return &fipsSpeciesMap; 
}

// used to look up re-used cell distances
template<typename T> std::vector<T> orderNumbers(T& number1, T& number2)
// order number1 and number2 from lowest to highest
{
	std::vector<T> ordered;
	ordered.emplace_back(number1);
	if (number2 < number1){
		ordered.insert(ordered.begin(),number2);
	} else {
		ordered.emplace_back(number2); // if number2 is larger or equal to number1
	}
	return ordered;
}

/// Sorts farms by population for a given species/type
struct comparePop
{
	comparePop(std::string in_species) : species(in_species) {}
	bool operator() (const Farm* farm1, const Farm* farm2){
		return (farm1->Farm::get_size(species)) < (farm2->Farm::get_size(species));
	}	
	private:
		std::string species;
};


#endif
