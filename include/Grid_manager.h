// Grid_manager.h

//
//  Creates a map of grid_cell objects, determined by local farm density
//	Evaluates infection spread via grid cells
//
// 7 Apr 2014

#ifndef Grid_manager_h
#define Grid_manager_h

#include "grid_cell.h"
#include "shared_functions.h"
#include "pairwise.h"
#include "file_manager.h"

#include <algorithm> // std::sort, std::any_of
#include <queue> // for checking neighbor cells
#include <stack>
#include <tuple>
#include <unordered_map>
#include <utility> // std::pair
#include <vector>


class County;
class State;

extern int verboseLevel;

class Grid_manager
{
	private:
		int verbose;
		// variables for grid creation
		unsigned int maxFarms;
		std::unordered_map<int, grid_cell*>
			allCells; // map of cells in grid
		std::unordered_map<int, Farm*>
			farm_map; // Contains all farm objects. Id as key.
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
			xylimits; // [0]x min, [1]x max, [2]y min, [3]y max
		// variables for infection evaluation
		std::unordered_map<int, std::unordered_map<int, double>>
			storedDists; // freq-used distances between cell pairs, used in shortestCellDist - use IDs to sort
		bool xyswitch;
		std::vector<std::string> speciesOnPrems; // list of species on all farms provided in prem file
		std::vector<double> speciesSus, speciesInf, speciesSusC, speciesInfC; // species specific susceptibility and infectiousness exponents and constants, in same order as speciesOnAllFarms
		std::unordered_map<std::string,std::vector<int>> herdSizes;
		std::unordered_map<std::string,double> xiP, xiQ;
		unsigned int committedFarms;
		std::unordered_map<std::string, Farm_type*> farm_types;

		double k1, k2, k3, k2tok3; // local kernel parameters
		const Parameters* parameters;

		// functions
		std::vector<double> read_replicate_file(std::string fname); //This is temporary until data is read from a database.
		void getReplicateData(); //Gets replicate-specific data (a, b & state shipments/t)
		void readStates(std::string state_file); //Reads states & flows from file.
		void readFips_and_states(); //Reads counties from file.
		void readFarms(std::string& farm_fname); //Does all the reading of premises-file
		void initFips(); //Removes counties without farms and sets shipping probs.
		void initStates(); //Initiates all farm shipment probs in the states.
		void set_maxFarms(unsigned int in_maxFarms); //inlined
		std::string to_string(grid_cell&) const;
		std::vector<Farm*> getFarms(
			std::tuple<int,double,double,double> cellSpecs,
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
 		double kernelsq(double);
		double shortestCellDist2(
			grid_cell* cell1,
			grid_cell* cell2); // calculates shortest distance^2 between two cells
		void makeCellRefs(); // make reference matrices for distance and kernel
		// functions for infection evaluation
		std::vector<grid_cell*>
			IDsToCells(std::vector<int>); // convert vector of IDs to cell pointers
		grid_cell*
			IDsToCells(int);  // overloaded to accept single ID also
		void set_FarmSus(Farm*); // used in precalculation and stored with Farm
		void set_FarmInf(Farm*); // used in precalculation and stored with Farm
		Farm_type* get_farm_type(std::string herd); //Returns a number based on what species are present on a farm.

	public:
		Grid_manager( // constructor loads premises
			std::string &fname, // filename of premises
			bool xyswitch,  // switch x/y columns
			std::vector<std::string>&, // list of species populations provided
			std::vector<double>&,  // list of species-specific susceptibility exponents
			std::vector<double>&, // list of species-specific infectiousness exponents
			std::vector<double>&,  // list of species-specific susceptibility constants
			std::vector<double>&, // list of species-specific infectiousness constants
			std::vector<double>&, // kernel parameters
			const Parameters* parameters); //Parameter struct.

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

		void printCells(
			std::string& pfile) const;

		const std::unordered_map<int, grid_cell*>*
			get_allCells(); //inlined

		const std::unordered_map<int, Farm*>*
			get_allFarms(); //inlined

		const std::unordered_map<std::string, County*>*
			get_FIPSmap(); //inlined

		const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Farm*> >>*
			get_fipsSpeciesMap(); //inlined

};

/////////// for grid creation ///////////
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
