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
#include <cmath> // std::sqrt
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

void Grid_cell_checker::setFocalCell(std::vector<grid_cell*> cellsToCheck)
{
	std::vector<grid_cell*> fullList = cellsToCheck;
	for (auto c1=cellsToCheck.begin(); c1!=cellsToCheck.end(); c1++)
	{ // loop through each cell (c1 as in "cell 1")
		grid_cell* currentCell = *c1; // set current cell
		if (verbose){std::cout << std::endl << "Focal cell set to " << (*c1)->get_id();}
		stepThroughCells(currentCell, cellsToCheck); // send to next loop for comparison
		// include comparison to self to include closest neighbors
	}
	std::cout << std::endl << std::endl //<< "Cell to cell skips: " << celltocellskips << std::endl
		<< "Farm to cell skips: " << farmtocellskips << " (avoided " << farmsinskippedcells << " comparisons)" << std::endl
		<< "Total infections (gridding): " << totalinfections << std::endl;
}

// focalCell is the grid_cell to which all others are compared
void Grid_cell_checker::stepThroughCells(grid_cell* focalCell, std::vector<grid_cell*> cellsToCheck)
{
	double focalNumFarms = focalCell->get_num_farms(); // how many farms in focal cell
if (verbose){std::cout << ": " << focalNumFarms << " farms. ";}

	for (auto c2:cellsToCheck){	// loop through each cell (c2 as in "cell 2")
	
		double gridKernValue = gridCellKernel[focalCell->get_id()][c2->get_id()];
		if (gridKernValue > 0){ // only proceed if kernel > 0
		
		if (verbose){std::cout << " Kernel: " << gridKernValue;}

		double compNumFarms = c2->get_num_farms(); // how many farms in comparison cell
		if (verbose){std::cout << std::endl << "Comparison cell " << c2->get_id() <<": " << compNumFarms << " farms. ";}

		// maximum transmission to/from, depends on farm sizes
		// initialized assuming infectOut is true
		double maxInf = focalCell->get_maxInf(); // max infectiousness of any farm in focalCell
		double maxSus = c2->get_maxSus(); // max susceptibility of any farm in comparison cell
		
		if (!infectOut)// if calculating spread TO focal cell, reassign values
			{
			maxSus = focalCell->get_maxSus();
			maxInf = c2->get_maxInf();
			}
	
		double s = 1; // on/off switch, 1 = on
		// get farms in each cell
		std::vector<Farm*> focalFarmList = focalCell -> get_farms();
		std::vector<Farm*> compFarmList = c2 -> get_farms();
		
		int f2count = 0; // how many farms in comparison cell have been checked
		double farmSus = 0;
		double farmInf = 0;
		double farmToCellProb = 0; // 1st "prob5" in MT's Fortran code, not divided out
		double indivFarmMaxProb = 0; // "prob6" in MT's Fortran code
		double remainingFarmsMaxProb = 0; // 2nd "prob5" in MT's Fortran code - replaces farmToCell while stepping through

		double f1x, f1y, f2x, f2y, xdiff, ydiff, distBWfarms, kernelBWfarms; // vars for farm pairs if evaluated
		double betweenFarmsProb = 0; // "prob3" in MT's Fortran code

		for (auto f1:focalFarmList){
		
			if (infectOut){
				farmInf = getFarmInf(f1); // infectiousness value for farm in focal cell
				farmToCellProb = 1 - exp(-farmInf * compNumFarms*maxSus * gridKernValue);
			} else if (!infectOut){
				farmSus = getFarmSus(f1); // susceptibility value for farm in focal cell
				farmToCellProb = 1 - exp(-farmSus * compNumFarms*maxInf * gridKernValue);		
			}
								
			double random1 = unif_rand();
			if (random1 < farmToCellProb){ // if farm to cell succeeds
				if (infectOut){
					indivFarmMaxProb = 1 - exp(-farmInf * maxSus * gridKernValue); 
					// infectiousness of focal farm * max susceptibility in comp cell * grid kernel
				} else if (!infectOut){
					indivFarmMaxProb = 1 - exp(-farmSus * maxInf * gridKernValue); 		
					// susceptibility of focal farm * max infectiousness in comp cell * grid kernel
				}
			
			if (indivFarmMaxProb > 0){ // only continue if indivMax > 0 
				for (auto f2:compFarmList){
					f2count++;
					if (infectOut){			
						remainingFarmsMaxProb =1-(s*exp(-farmInf * (compNumFarms+1-f2count)*maxSus * gridKernValue));
					} else if (!infectOut){	
						remainingFarmsMaxProb =1-(s*exp(-farmSus * (compNumFarms+1-f2count)*maxInf * gridKernValue));
					}
					// # farms left in cell * farm(a) infectiousness * farm(b) susceptibility * grid kernel

					//if(verbose && f2count==1){std::cout << " remMaxProb: " << remainingFarmsMaxProb << " ";}
					double random2 = unif_rand(); // "prob4" in MT's Fortran code
					if (random2 < indivFarmMaxProb/remainingFarmsMaxProb){	
					//if(verbose){std::cout << random2 << " < " << indivFarmMaxProb/remainingFarmsMaxProb;}

					// if (one max susceptible)/(number of farms using specific sus-inf values) succeeds
						s = 0; // remainingFarmProb would recalculate to 1, but isn't used
						// get actual distances between farms
						f1x = f1 -> get_x();
						f1y = f1 -> get_y();
						f2x = f2 -> get_x();
						f2y = f2 -> get_y();
						xdiff = (f1x - f2x);
						ydiff = (f1y - f2y);
						distBWfarms = sqrt(xdiff*xdiff + ydiff*ydiff);
						// add kernel choice option
						kernelBWfarms = linearDist(distBWfarms);
						// get individual infectiousness/susceptibility values
						if (infectOut){			
							farmSus = getFarmSus(f2); // susceptible farm in comparison cell (farmInf already defined from focal cell)
						} else if (!infectOut){	
							farmInf = getFarmInf(f2); // infectious farm in comparison cell (farmSus already defined from focal cell)
	 					}
						// calculate probability between these specific farms
						betweenFarmsProb = 1-exp(-farmSus * farmInf * kernelBWfarms); // prob tx between this farm pair
						// "prob3" in MT's Fortran code
						if (random2 < betweenFarmsProb/remainingFarmsMaxProb){
						// why don't we account for the indivFarmMaxProb? b/c we use the same random number?
							// infect
							totalinfections++;
							if (verbose){std::cout << " Farm infected. ";}
							}
						} // end "if trans between indiv farms at max"
					} // end for each comparison cell farm
			  		f2count = 0;
					} // end if indivMaxProb > 0
				} else { // end if p(farm->cell) succeeds
					farmtocellskips++;
					farmsinskippedcells += compNumFarms;
					if (verbose){std::cout << "Cell skipped." << std::endl;}
				} // end if p(farm->cell) fails
			} // end for each focal cell farm			
			} // end "if gridkernel value > 0"
		} // end for loop through comparison cells
}

