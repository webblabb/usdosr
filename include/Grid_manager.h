// Grid_manager.h

//
//  Creates a map of grid_cell objects, determined by local farm density
//	Evaluates infection spread via grid cells
//
// 7 Apr 2014

#ifndef Grid_manager_h
#define Grid_manager_h

#include "grid_cell.h"
#include "farm.h"
#include "shared_functions.h"
#include "pairwise.h"

#include <algorithm> // std::sort, std::any_of
#include <queue> // for checking neighbor cells
#include <stack>
#include <tuple>
#include <unordered_map>
#include <utility> // std::pair
#include <vector>

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
		std::unordered_map<std::string, std::vector<Farm*>>
			FIPSmap; // key is fips code, value is vector of farms within
		std::unordered_map<std::string, 
			std::unordered_map< std::string, std::vector<Farm*> >> fipsSpeciesMap;
			// key is fips code, then species name, then sorted by population size
			
		// for modified gridding using stored 2^n map
		unsigned int maxFarmsInACell;
		std::unordered_map<int,double> twoPower;
		
 		std::vector<Farm*> 
 			farmList; // vector of pointers to all farms (deleted in chunks as grid is created)
		std::tuple<double,double,double,double> 
			xylimits; // [0]x min, [1]x max, [2]y min, [3]y max
		std::unordered_map<int, std::unordered_map<int, double>> 
			gridCellKernel; // kernel values between cell pairs - use IDs to sort
		std::unordered_map<int, std::unordered_map<int, double>> 
			storedDists; // freq-used distances between cell pairs, used in shortestCellDist - use IDs to sort
		std::unordered_map<int, std::vector<grid_cell*>> 
			kernelNeighbors; // each cell's susceptible neighbors with p>0
		// variables for infection evaluation
		bool infectOut; // if true, looks at transmission TO other cells, otherwise FROM other cells
		std::vector<std::string> speciesOnPrems; // list of species on all farms provided in prem file
		std::vector<double> speciesSus, speciesInf; // species specific susceptibility and infectiousness, in same order as speciesOnAllFarms
		// counting variables to keep track of infection/efficiency
		// per timestep variables
		int farmtocellskips = 0;
		int farmsinskippedcells = 0;
		std::unordered_map<int, std::vector<int>> infectedFarms; // ids (key) & counts of farms infected in a timestep - counts > 1 are post-infection exposures

		// functions		
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
		double shortestCellDist2(
			grid_cell* cell1, 
			grid_cell* cell2); // calculates shortest distance^2 between two cells
		void makeCellRefs(); // make reference matrices for distance and kernel		
		// functions for infection evaluation
		std::vector<grid_cell*> 
			IDsToCells(std::vector<int>); // convert vector of IDs to cell pointers
		grid_cell* 				
			IDsToCells(int);  // overloaded to accept single ID also
		double getFarmSus(Farm*); // used in precalculation and stored with Farm
		double getFarmInf(Farm*); // used in precalculation and stored with Farm
		std::string formatPWOutput(std::tuple<double,double,int,int,int,int>&);
		std::string formatAnomaly(std::tuple<int,int,bool,bool,std::string,double>&);
		
	public:
		/////////// for grid creation ///////////
		Grid_manager( // constructor loads premises
			std::string &fname, // filename of premises
			bool xyswitch,  // switch x/y columns
			std::vector<std::string>&, // list of species populations provided
			std::vector<double>&,  // list of species-specific susceptibility values
			std::vector<double>&); // list of species-specific infectiousness values
			
		~Grid_manager();
		
		// main function that splits/commits cells:
		// 1st way to initiate a grid: specify maximum farms per cell, min size determined by kernel radius
		void initiateGrid(
			const unsigned int maxFarms, 
			const int kernelRadius); 
		
		// 2nd way to initiate a grid: specify file containing cell specs
		void initiateGrid(
			std::string &cname);
		
		// 3rd way to initiate a grid: specify side length (same units as x/y) for uniform cells
		void initiateGrid(
			double cellSide);	
		
		void printCells(
			std::string& pfile) const;
			
		void printGridKernel() const;
		
// 		void printVector(
// 			std::vector<Farm*>&, std::string&) const;
			
		std::unordered_map<int, grid_cell*> 
			get_allCells() const; //inlined	
			
		std::unordered_map<int, std::unordered_map<int, double>> 
			get_gridCellKernel() const; // inlined
			
		std::unordered_map<int, Farm*> 
			get_allFarms() const; //inlined
			
		std::unordered_map<std::string, std::vector<Farm*>> 
			get_FIPSmap() const; //inlined
			
		std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Farm*> >> 
			get_fipsSpeciesMap() const; //inlined
	
		std::vector<Farm*>
			get_infectedFarms() const; // called from main, cleared at each timestep
			
		/////////// for infection evaluation ///////////
		// per timestep function
			
		void stepThroughCells(
			std::vector<Farm*>&, std::vector<Farm*>&);

//		void stepThroughCellsFilter(
//			std::vector<Farm*>&, std::vector<Farm*>&);

		void stepThroughCellsBinom(
			std::vector<Farm*>&, std::vector<Farm*>&);
			
		void stepThroughCellsPW(std::vector<Farm*>&, std::vector<Farm*>&);
		// calcs pw prob for each farm for comparison to gridding loops
		
// 		void setInfectOut(bool); //inlined

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

inline std::unordered_map<int, grid_cell*> 
	Grid_manager::get_allCells() const
{
	return (allCells);
}

inline std::unordered_map<int, std::unordered_map<int, double>> 
	Grid_manager::get_gridCellKernel() const
{
	return(gridCellKernel);
}

inline std::unordered_map<int, Farm*> 
	Grid_manager::get_allFarms() const
{
	return(farm_map);
}

inline std::unordered_map<std::string, std::vector<Farm*>> 
	Grid_manager::get_FIPSmap() const
{
	return(FIPSmap); 
}

inline std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Farm*> >> 
	Grid_manager::get_fipsSpeciesMap() const
{
	return(fipsSpeciesMap); 
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

/////////// for infection evaluation ///////////
// inline void Grid_manager::setInfectOut(bool io)
// {
// 	infectOut = io;
// }

#endif
