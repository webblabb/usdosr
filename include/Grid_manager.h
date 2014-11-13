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

#include <algorithm> // std::sort, std::any_of
#include <queue> // for checking neighbor cells
#include <stack>
#include <unordered_map>
#include <vector>

class Grid_manager
{
	private:
		bool verbose; // if true, outputs processing details
		// variables for grid creation
		unsigned int maxFarms;
		std::unordered_map<double, grid_cell*> 
			allCells; // map of cells in grid
		std::unordered_map<int, Farm*> 
			farm_map; // Contains all farm objects. Id as key.
		std::unordered_map<std::string, std::vector<Farm*>>
			FIPSmap; // key is fips code, value is vector of farms within
 		std::vector<Farm*> 
 			farmList; // vector of pointers to all farms (deleted in chunks as grid is created)
		std::vector<double> 
			xylimits; // [0]x min, [1]x max, [2]y min, [3]y max
		std::unordered_map<double, std::unordered_map<double, double>> 
			gridCellKernel; // kernel values between cell pairs
		std::unordered_map<double, std::unordered_map<double, double>> 
			storedDists; // freq-used distances between cell pairs, used in shortestCellDist
		std::unordered_map<double, std::vector<grid_cell*>> 
			kernelNeighbors; // each cell's susceptible neighbors with p>0
		// variables for infection evaluation
		bool infectOut; // if true, looks at transmission TO other cells, otherwise FROM other cells
		// counting variables to keep track of infection/efficiency
		// per timestep variables
		int farmtocellskips = 0;
		int farmsinskippedcells = 0;
		std::unordered_map<double, std::vector<double>> infectedFarms; // ids & counts of farms infected in a timestep

		// functions
		void setVerbose(bool); // inlined
		
		void set_maxFarms(unsigned int in_maxFarms); //inlined
		std::string to_string(grid_cell&) const;
		std::vector<Farm*> getFarms(
			std::vector<double> cellSpecs,
			const unsigned int maxFarms=0); // makes list of farms in a cell (quits early if over max)
		void removeParent(
			std::stack< std::vector<double> >& queue);// removes 1st vector in queue
		void addOffspring(
			std::vector<double> cellSpecs, 
			std::stack< std::vector<double> >& queue); // adds subdivided cells to queue
		void commitCell(
			std::vector<double> cellSpecs, 
			std::vector<Farm*>& farmsInCell); // adds cell as type GridCell to set allCells
		void splitCell(
			std::vector<double>& cellSpecs, 
			std::stack< std::vector<double> >& queue); // replaces parent cell with subdivided offspring quadrants
 		void assignCellIDtoFarms(double cellID, std::vector<Farm*>& farmsInCell);
		double shortestCellDist(
			grid_cell* cell1, 
			grid_cell* cell2); // calculates shortest distance between two cells
		void makeCellRefs(); // make reference matrices for distance and kernel		
		// functions for infection evaluation
		std::vector<grid_cell*> 
			IDsToCells(std::vector<double>); // convert vector of IDs to cell pointers
		grid_cell* 				
			IDsToCells(double);  // overloaded to accept single ID also
		
	public:
		/////////// for grid creation ///////////
		Grid_manager( // constructor loads premises
			std::string &fname, // filename of premises
			bool xyswitch = 0, // switch x/y columns
			bool v = 0); 
			
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
		
		void printVector(
			std::vector<Farm*>&, std::string&) const;
			
		std::unordered_map<double, grid_cell*> 
			get_allCells() const; //inlined	
			
		std::unordered_map<double, std::unordered_map<double, double>> 
			get_gridCellKernel() const; // inlined
			
		std::unordered_map<int, Farm*> 
			get_allFarms() const; //inlined
			
		std::unordered_map<std::string, std::vector<Farm*>> 
			get_FIPSmap() const; //inlined
			
		void removeFarmSubset(std::vector<Farm*>&, std::vector<Farm*>&); // removes farms in 1st vector from 2nd (ie recently committed cell from main list)
		/////////// for infection evaluation ///////////
		// per timestep functions
		std::vector <std::vector<Farm*>> 
			setFarmStatuses(double);
			
		void stepThroughCells(
			std::vector<Farm*>&, std::vector<Farm*>&);
		// calcs pw prob for each farm for comparison to gridding loops
		
		void setInfectOut(bool); //inlined
		
		std::unordered_map<double, std::vector<double>> 
			getTotalInfections() const; //inlined
			
		std::vector<Farm*> 
			getInfVec() const;

};

/////////// for grid creation ///////////
inline bool sortByX(const Farm* farm1, const Farm* farm2)
// "compare" function to sort farms by x-coordinate
// used when assigning farms to uniform cell grid
// must be defined outside of class, or else sort doesn't work
{
	return (farm1 -> get_x()) < (farm2 -> get_x());
}
inline bool sortByID(const Farm* farm1, const Farm* farm2)
// "compare" function to sort farms by ID
// must be defined outside of class, or else sort doesn't work
{
	return (farm1 -> get_id()) < (farm2 -> get_id());
}
inline bool sortByPop(const Farm* farm1, const Farm* farm2)
// "compare" function to sort farms by ID
// must be defined outside of class, or else sort doesn't work
{
	return (farm1 -> get_size()) < (farm2 -> get_size());
}

inline void Grid_manager::setVerbose(bool v)
{
	verbose = v;
}

inline void Grid_manager::set_maxFarms(unsigned int in_maxFarms)
{
	maxFarms = in_maxFarms;
}

inline std::unordered_map<double, grid_cell*> 
	Grid_manager::get_allCells() const
{
	return (allCells);
}

inline std::unordered_map<double, std::unordered_map<double, double>> 
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

// inline std::unordered_map<double, std::vector<double>> 
// 	Grid_manager::get_kernelNeighbors() const
// {
// 	return(kernelNeighbors);
// }

/////////// for infection evaluation ///////////
inline void Grid_manager::setInfectOut(bool io)
{
	infectOut = io;
}

inline std::unordered_map<double, std::vector<double>> Grid_manager::getTotalInfections() const
{
	return infectedFarms;
}

#endif
