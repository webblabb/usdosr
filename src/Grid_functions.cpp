// Functions for within-simulation gridding algorithm

// input a list of all cells
// run nested loops to compare pairs of cells (in both transmission directions)
// draw random number
// ...get distance of nearest cell corner to focal farm
// get max sus or max inf of cells
// calculate prob for cell
// if random number <= prob, step in
// else move on to next cell

#include <random> // random number generator
#include <vector>
#include <cmath> // std::abs, std::sqrt
#include Grid_functions.h
#include grid_cell.h

double unif_rand()
{
	static std::uniform_real_distribution<double> unif_dist(0.0, 1.0);
	static unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
	static std::mt19937 generator(seed); //Mersenne Twister pseudo-random number generator. Generally considered research-grade.
	return unif_dist(generator);
}


// main functions for gridding algorithm

// outer nested loop: determine first of cell pairs, send to inner nested loop (stepThroughCells)
// bool infectOut (if true, looks at transmission TO other cells, otherwise FROM other cells)

void setFocalCell(std::vector<grid_cell*> cellsToCheck, bool infectOut)
{
	for (auto c1:cellsToCheck){ // loop through each cell (c1 as in "cell 1")
		grid_cell* currentCell = c1; // set current cell
		std::vector<grid_cell*> cellsToCompare = cellsToCheck; // copy the cell list
		cellsToCompare.erase(c1); // remove the current cell from cellsToCompare: now contains all other cells
		stepThroughCells(currentCell, cellsToCompare, infectOut); // send to next loop for comparison
	}
}

// focalCell is the grid_cell to which all others are compared
void stepThroughCells(grid_cell* focalCell, std::vector<grid_cell*> cellsToCheck, bool infectOut)
{
	double maxProbFrom, maxProbTo;
	for (auto c2:cellsToCheck){	// loop through each cell (c2 as in "cell 2")
		std::vector<Farm*> farmsInCell = c2->get_farms(); // make vector of farms within cell
		double numFarms = farmsInCell.size();

		if(infectOut) // if calculating infectious spread from focal cell
			{
			maxProbFrom = focalCell->get_maxInf();
			maxProbTo = c2->get_maxSus();
			}
		else // if calculating spread to focal cell
			{
			maxProbFrom = focalCell->get_maxSus();
			maxProbTo = c2->get_maxInf();
			}
		
		// maximum probability any transmission occurs between cells
		double maxProbCell = 1 - exp(-numFarms * maxProbFrom * MaxProbTo * gridKernel(focalCell,c2))
			if (verbose){cout << "p(transmission between cells) = " << maxProbCell;}
		double random1 = unif_rand();
			if (verbose){cout << "random1 = " << random1;}
		// loop through 
		// divide individual farm probabilities by p_enterCell
		double p_enterCell = 1 - exp(-maxProb * trans * kernelValue(distToCell))
		
		//double r = unif_rand();
	}
}

// function that checks individual farms in a cell and "do (inverse) diffuse infections"
void enterCell(double p_enterCell)
{
// get all farms in cell
}