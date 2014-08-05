//
//  Grid_Creator.h
// 
//  Creates a vector of grid_cell objects, determined by local farm density
//

#ifndef Grid_cell_checker_h
#define Grid_cell_checker_h

#include "grid_cell.h"
#include "farm.h"
#include "shared_functions.h"

#include <vector>
#include <stack>
#include <unordered_map>

class Grid_cell_checker
{
	private:
		bool verbose; // if true, outputs processing details
		bool infectOut; // if true, looks at transmission TO other cells, otherwise FROM other cells
		std::unordered_map<double, grid_cell*> allCells;
		std::unordered_map<double, std::unordered_map<double, double>> gridCellKernel;
		std::unordered_map<double, std::vector<double>> neighbors;
		
		// counter variables to keep track of infection/efficiency
		int farmtocellskips = 0;
		int farmsinskippedcells = 0;
		int totalinfections = 0;
		
		// functions for calculating reference distance/kernel matrices from grid
		void setVerbose(bool); // inlined
		void setInfectOut(bool); //inlined

		// functions for main gridding algorithm
		std::vector<grid_cell*> IDsToCells(std::vector<double>&) const; // convert vector of IDs to cell pointers
		grid_cell* 				IDsToCells(double&) const;  // overloaded to accept single ID also
		std::vector<double> orderIDs(double cellID1, double cellID2); // orders smaller before larger
		std::vector<grid_cell*> neighborsOf(double& cellID); // returns immediate neighbors of cellID
		std::vector<grid_cell*> posKernelNeighborsOf(double cellID);
		void 					setFocalCell(std::unordered_map<double, grid_cell*>& cellsToCheck);
		void 					stepThroughCells(grid_cell* focalCell, std::vector<grid_cell*>& cellsToCheck);
		void 					enterCell(double p_enterCell);
		
	public:
		// Constructor:
		Grid_cell_checker(const std::unordered_map<double, grid_cell*>&, 
			const std::unordered_map<double, std::unordered_map<double, double>>&, 
			const std::unordered_map<double, std::vector<double>>&,
			const bool v, const bool io);
		~Grid_cell_checker();
		int getTotalInfections() const; //inlined
};

inline void Grid_cell_checker::setVerbose(bool v)
{
	verbose = v;
}

inline void Grid_cell_checker::setInfectOut(bool io)
{
	infectOut = io;
}

inline int Grid_cell_checker::getTotalInfections() const
{
	return totalinfections;
}

#endif
