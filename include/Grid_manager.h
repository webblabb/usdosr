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

	/////////// for grid creation ///////////
		// variables
		unsigned int maxFarms;
		std::unordered_map<double, grid_cell*> 
			allCells; // map of cells in grid
		std::unordered_map<int, Farm*> 
			farm_map; // Contains all farm objects. Id as key.
 		std::vector<Farm*> 
 			farmList; // vector of pointers to all farms (deleted in chunks as grid is created)
		std::vector<double> 
			xylimits; // [0]x min, [1]x max, [2]y min, [3]y max
		std::unordered_map<double, std::unordered_map<double, double>> 
			gridCellKernel; // kernel values between cell pairs
		std::unordered_map<double, std::vector<double>> 
			kernelNeighbors; // each cell's neighbors with positive kernel values
		// functions
		void setVerbose(bool); // inlined
		
		void set_maxFarms(unsigned int in_maxFarms); //inlined
		std::string to_string(grid_cell&) const;
		void updateFarmList(
			std::vector<Farm*>& farmsInCell); // removes farms from recently committed cell from main list
		std::vector<Farm*> getFarms(
			std::vector<double> cellSpecs,
			const unsigned int maxFarms); // makes list of farms in a cell (quits early if over max)
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
 		std::vector<grid_cell*> 
 			posKernelNeighborsOf(double cellID);
		double shortestCellDist(
			grid_cell* cell1, 
			grid_cell* cell2) const; // calculates shortest distance between two cells
		void makeCellRefs(); // make reference matrices for distance and kernel
		
		/////////// for infection evaluation ///////////
		// variables
		bool infectOut; // if true, looks at transmission TO other cells, otherwise FROM other cells
		// counting variables to keep track of infection/efficiency
		// per timestep variables
		int farmtocellskips = 0;
		int farmsinskippedcells = 0;
		int totalinfections = 0;
		// functions
		std::vector<grid_cell*> 
			IDsToCells(std::vector<double>); // convert vector of IDs to cell pointers
		grid_cell* 				
			IDsToCells(double);  // overloaded to accept single ID also
		std::vector<double> 
			orderIDs(double cellID1, double cellID2); // orders smaller before larger
		
	public:
		/////////// for grid creation ///////////
		Grid_manager( // constructor loads premises
			std::string &fname, // filename of premises
			bool xyswitch, // switch x/y columns
			bool v); 
			
		~Grid_manager();
		
		void initiateGrid(
			const unsigned int maxFarms, 
			const int kernelRadius); // main function that splits/commits cells
			
		void initiateGrid(
			std::string &cname); // overloaded version accepts file of cell specs
			
		void initiateGrid(
			double cellSide); // overloaded version accepts length of all (uniform) cell sides, same units as x/y	
		
		void printCells(std::string& pfile) const;
		void printGridKernel() const;
		std::unordered_map<double, grid_cell*> 
			get_allCells() const; //inlined	
		std::unordered_map<double, std::unordered_map<double, double>> 
			get_gridCellKernel() const; // inlined
		std::unordered_map<int, Farm*> 
			get_allFarms() const; //inlined
		std::unordered_map<double, std::vector<double>> 
			get_kernelNeighbors() const; //inlined
			
		/////////// for infection evaluation ///////////
		// per timestep functions
		//void setFocalCells(std::vector<Farm*>& focalFarms); // gets cells of focal farms and sends them to stepThroughCells
		std::vector <std::vector<Farm*>> fakeFarmStatuses(double);
		std::vector <std::vector<Farm*>> fakeFarmStatuses(double, double);
		void stepThroughCells(std::vector<Farm*>&, std::vector<Farm*>&);
		void setInfectOut(bool); //inlined
		int getTotalInfections() const; //inlined

};

/////////// for grid creation ///////////
inline bool sortByX(const Farm* farm1, const Farm* farm2)
// "compare" function to sort farms by x-coordinate
// must be defined outside of class, or else sort doesn't work
{
	return (farm1 -> get_x()) < (farm2 -> get_x());
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

inline std::unordered_map<double, std::vector<double>> 
	Grid_manager::get_kernelNeighbors() const
{
	return(kernelNeighbors);
}

/////////// for infection evaluation ///////////
inline void Grid_manager::setInfectOut(bool io)
{
	infectOut = io;
}

inline int Grid_manager::getTotalInfections() const
{
	return totalinfections;
}

#endif
