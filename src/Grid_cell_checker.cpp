// Functions for within-simulation gridding algorithm

// input a list of all cells
// run nested loops to compare pairs of cells (in both transmission directions)
// draw random number
// ...get distance of nearest cell corner to focal farm
// get max sus or max inf of cells
// calculate prob for cell
// if random number <= prob, step in
// else move on to next cell

#include <iostream>
#include <vector>
#include <string>
// #include <cmath> // std::abs, std::sqrt
#include "Grid_cell_checker.h"
		
// Constructor: reads in file of cells
Grid_cell_checker::Grid_cell_checker(std::vector<grid_cell*> in_allCells, 
			std::unordered_map<double, std::unordered_map<double, double>> in_gridCellKernel,
			bool v, bool io): 
			allCells(in_allCells), gridCellKernel(in_gridCellKernel) // initialize with these values
{
	setVerbose(v);
	setInfectOut(io);
	std::cout << "Grid cell checker initialized." << std::endl;
	setFocalCell(allCells);
}

Grid_cell_checker::~Grid_cell_checker()
{
}

// main functions for gridding algorithm

// outer nested loop: determine first of cell pairs, send to inner nested loop (stepThroughCells)

// looping through in weird order. checked: cell id assignments ok.

void Grid_cell_checker::setFocalCell(std::vector<grid_cell*> cellsToCheck)
{
	std::vector<grid_cell*> fullList = cellsToCheck;
	for (auto c1=cellsToCheck.begin(); c1!=cellsToCheck.end(); c1++)
	{ // loop through each cell (c1 as in "cell 1")
		grid_cell* currentCell = *c1; // set current cell
		if (verbose){std::cout << std::endl << "Focal cell set to " << (*c1)->get_id() << std::endl;}

		std::vector<grid_cell*> cellsToCompare = fullList; // reset copy of cell list
		cellsToCompare.erase(c1); // remove the current cell from cellsToCompare: now contains all other cells
		stepThroughCells(currentCell, cellsToCompare); // send to next loop for comparison
	}
}

// focalCell is the grid_cell to which all others are compared
void Grid_cell_checker::stepThroughCells(grid_cell* focalCell, std::vector<grid_cell*> cellsToCheck)
{
	double focalNumFarms = focalCell->get_num_farms(); // how many farms in focal cell
	if (verbose){std::cout << "Focal cell: " << focalNumFarms << " farms. ";}

	for (auto c2:cellsToCheck){	// loop through each cell (c2 as in "cell 2")
		double compNumFarms = c2->get_num_farms(); // how many farms in comparison cell
		if (verbose){std::cout << "Comparison cell " << c2->get_id() <<": " << compNumFarms << " farms. ";}

		// maximum transmission to/from, depends on farm size
		// initialized assuming infectOut is true - avoids warning about being uninitialized
		double maxTxFrom = focalCell->get_maxInf(); // max infectiousness of any farm in focalCell
		double maxTxTo = c2->get_maxSus(); // max susceptibility of any farm in comparison cell
			
		if (!infectOut)// if calculating spread to focal cell
			{
			maxTxFrom = focalCell->get_maxSus();
			maxTxTo = c2->get_maxInf();
			}
		
		if (verbose){std::cout << "Max tx from: " << maxTxFrom;
		std::cout << " Max tx to: " << maxTxTo << std::endl;}

		// maximum probability any transmission occurs between cells
		double CellToCell = 1 - exp(-focalNumFarms * compNumFarms * maxTxFrom * maxTxTo * gridCellKernel[focalCell->get_id()][c2->get_id()]);
		if (verbose){std::cout << " Kernel: " << gridCellKernel[focalCell->get_id()][c2->get_id()];}
			if (verbose){std::cout << " p(cell-cell tx) = " << CellToCell;}
		double random1 = unif_rand();
			if (verbose){std::cout << ", random1 = " << random1 << std::endl;}
// 			
// 		if (CellToCell < random1)
// 		{
// 			double indivFarmMaxProb = 
// 			double random2 = unif_rand();
// 		
// 		}
		// loop through 
		// divide individual farm probabilities by p_enterCell
// 		double p_enterCell = 1 - exp(-maxProb * trans * kernelValue(distToCell))
		
	} // end for loop through comparison cells
}

// // function that checks individual farms in a cell and "do diffuse infections"
// void Grid_cell_checker::enterCell(double p_enterCell)
// {
// // get all farms in cell
// }