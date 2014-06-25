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
		std::vector<grid_cell*> allCells;
		std::unordered_map<double, std::unordered_map<double, double>> gridCellKernel;
		
		// functions for calculating reference distance/kernel matrices from grid
		void setVerbose(bool); // inlined
		void setInfectOut(bool); //inlined
		
		// functions for main gridding algorithm
		void setFocalCell(std::vector<grid_cell*> cellsToCheck);
		void stepThroughCells(grid_cell* focalCell, std::vector<grid_cell*> cellsToCheck);
		void enterCell(double p_enterCell);
		
	public:
		// Constructor:
		Grid_cell_checker(std::vector<grid_cell*> allCells, 
			std::unordered_map<double, std::unordered_map<double, double>> gridCellKernel, 
			bool v, bool io);
		~Grid_cell_checker();
		

};

inline void Grid_cell_checker::setVerbose(bool v)
{
	verbose = v;
}

inline void Grid_cell_checker::setInfectOut(bool io)
{
	infectOut = io;
}

#endif
