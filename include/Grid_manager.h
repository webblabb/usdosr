// Grid_manager.h
//
// 7 Apr 2014

#ifndef Grid_manager_h
#define Grid_manager_h

#include "file_manager.h" // for parameter struct
#include "grid_cell.h"
//#include "pairwise.h"
#include "shared_functions.h" //Needed???

#include <algorithm> // std::sort, std::any_of
#include <queue> // for checking neighbor cells
#include <stack>
#include <tuple>
#include <unordered_map>
#include <utility> // std::pair


class County;
class State;

extern int verboseLevel;

///  Creates grid cells determined by local farm density, stores relevant values with Farms
class Grid_manager
{
	private:
		int verbose; ///> Global value of verbose level (unless overridden for this class)

		// variables for grid creation
		unsigned int maxFarms; ///> Threshold number of premises per cell (cell size takes precedence)
		std::unordered_map<int, grid_cell*>
			allCells; ///> Unordered_map of all cells in grid
		std::unordered_map<int, Farm*>
			farm_map; ///> Unordered_map of all premises objects
//		std::unordered_map<std::string, std::vector<Farm*>>
//			FIPSmap; // key is fips code, value is vector of farms within
		std::unordered_map<std::string, State*>
            state_map; //Contains states, name as key.
        std::map<std::string, std::vector<double>> a_map; //Stores a-parameters for states
        std::map<std::string, std::vector<double>> b_map; //Stores b-parameters for states
		std::map<std::string, std::vector<double>> shipment_volume_map; //Stores shipment volumes for the states.
		std::unordered_map<std::string, County*>
			FIPSmap; // key is fips code, value is county object
        std::vector<County*>
            FIPSvector;
		std::unordered_map<std::string,
			std::unordered_map< std::string, std::vector<Farm*> >> fipsSpeciesMap;
			// key is fips code, then species name, then sorted by population size
 		std::vector<Farm*>
 			farmList; // vector of pointers to all farms (deleted in chunks as grid is created)
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
		std::unordered_map<std::string, Farm_type*> farm_types;
		const Parameters* parameters;

		// functions
		std::vector<double> read_replicate_file(std::string fname); //This is temporary until data is read from a database.
		void getReplicateData(); //Gets replicate-specific data (a, b & state shipments/t)
		void readStates(std::string state_file); //Reads states & flows from file.
		void readFips_and_states(); //Reads counties from file.
		void readFarms(const std::string& farm_fname); //Does all the reading of premises-file
		void initFips(); //Removes counties without farms and sets shipping probs.
		void initStates(); //Initiates all farm shipment probs in the states.
		void set_maxFarms(unsigned int in_maxFarms); //inlined
		std::string to_string(grid_cell&) const;
		std::vector<Farm*> getFarms(
			std::tuple<int,double,double,double>& cellSpecs,
			const unsigned int maxFarms=0); // makes list of farms in a cell (quits early if over max)
		void removeParent(
			std::stack< std::tuple<int,double,double,double> >& queue);// removes 1st vector in queue
		void addOffspring(
			std::tuple<int,double,double,double> cellSpecs,
			std::stack< std::tuple<int,double,double,double> >& queue); // adds subdivided cells to queue
		void commitCell(
			std::tuple<int,double,double,double> cellSpecs,
			std::vector<Farm*>& farmsInCell); // adds cell as type GridCell to set allCells
		void splitCell(
			std::tuple<int,double,double,double>& cellSpecs,
			std::stack< std::tuple<int,double,double,double> >& queue); // replaces parent cell with subdivided offspring quadrants
 		void assignCellIDtoFarms(int cellID, std::vector<Farm*>& farmsInCell);
		double shortestCellDist2(
			grid_cell* cell1,
			grid_cell* cell2); ///> Calculates (shortest distance between two cells)^2
		void makeCellRefs(); ///> Calculates and stores kernel values and other pre-processing tasks
		// functions for infection evaluation
		void set_FarmSus(Farm*); ///> Calculates premises susceptibility and stores in Farm
		void set_FarmInf(Farm*); ///> Calculates premises infectiousness and stores in Farm
		Farm_type* get_farm_type(std::string herd); //Returns pointer to type based on what species are present on a farm.

	public:
		Grid_manager(const Parameters*);
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

		const std::unordered_map<std::string, County*>*
			get_FIPSmap(); //inlined

		const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Farm*> >>*
			get_fipsSpeciesMap(); //inlined

		void printCells();
};

inline bool sortByX(const Farm* farm1, const Farm* farm2)
// "compare" function to sort farms by x-coordinate
// used when assigning farms to uniform cell grid
// must be defined outside of class, or else sort doesn't work
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

inline const std::unordered_map<std::string, County*>*
	Grid_manager::get_FIPSmap()
{
	return &FIPSmap;
}

//inline const std::unordered_map<std::string, std::vector<Farm*>>*
//	Grid_manager::get_FIPSmap()
//{
//	return &FIPSmap;
//}

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

// used to sort farms by population for a given species/type
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
