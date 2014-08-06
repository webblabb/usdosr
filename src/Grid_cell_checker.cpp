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
#include <string>
#include <cmath> // std::sqrt
#include <queue>
#include "Grid_cell_checker.h"
		
// Constructor: uses list of cells, grid kernel values, and each cell's immediate neighbors
Grid_cell_checker::Grid_cell_checker(const std::unordered_map<double, grid_cell*>& in_allCells, 
			const std::unordered_map<double, std::unordered_map<double, double>>& in_gridCellKernel,
			const std::unordered_map<double, std::vector<double>>& in_neighbors,
			const bool v, const bool io): 
			allCells(in_allCells), gridCellKernel(in_gridCellKernel), kernelNeighbors(in_neighbors) // initialize with these values
{
	setVerbose(v);
	setInfectOut(io);
	std::cout << "Grid cell checker initialized.";
 	setFocalCell(allCells);
 }

Grid_cell_checker::~Grid_cell_checker()
{
}

// functions for gridding algorithm

// convert vector of IDs to cell pointers
std::vector<grid_cell*> Grid_cell_checker::IDsToCells(std::vector<double>& cellIDs) const
{
	std::vector<grid_cell*> neighborCells;
	for (auto i:cellIDs)
	{
		neighborCells.emplace_back(allCells.at(i));
	}
	return neighborCells;
}

//overloaded to accept single ID also
grid_cell* Grid_cell_checker::IDsToCells(double& cellID) const
{
	return allCells.at(cellID);
}

std::vector<double> Grid_cell_checker::orderIDs(double cellID1, double cellID2)
{
	std::vector<double> ordered;
	ordered.emplace_back(cellID1);
	if (cellID2 < cellID1){
		ordered.insert(ordered.begin(),cellID2);
	} else {
		ordered.emplace_back(cellID2); // if cellID2 is larger or equal to cellID1
	}
	return ordered;
}

std::vector<grid_cell*> Grid_cell_checker::posKernelNeighborsOf(double cellID)
{
	return IDsToCells(kernelNeighbors.at(cellID));
}

// outer nested loop: determine first of cell pairs, send to inner nested loop (stepThroughCells)

void Grid_cell_checker::setFocalCell(std::unordered_map<double, grid_cell*>& cellsToCheck)
{
	if (verbose){std::cout << "Setting focal cell... ";}
	for (auto c1:cellsToCheck)
	{ // loop through each cell (c1 as in "cell 1")
		grid_cell* currentCell = c1.second; // set current cell
		if (verbose){std::cout << std::endl << "Focal cell set to " << c1.first;}
		std::vector<grid_cell*> neighborsOfFocal = posKernelNeighborsOf(c1.first);
		if (verbose){std::cout << ", with " << neighborsOfFocal.size() << " neighbor cells. " ;}
		stepThroughCells(currentCell, neighborsOfFocal); // send to next loop for comparison
		// include comparison to self to include other farms in same cell
	}
	std::cout << std::endl
		<< "Farm to cell skips: " << farmtocellskips << " (avoided " << farmsinskippedcells << " comparisons)" << std::endl
		<< "Total infections (gridding): " << totalinfections << std::endl;
}

// focalCell is the grid_cell to which all others are compared
void Grid_cell_checker::stepThroughCells(grid_cell* focalCell, std::vector<grid_cell*>& cellsToCheck)
{
	double focalNumFarms = focalCell->get_num_farms(); // how many farms in focal cell
	if (verbose){std::cout << "Focal cell: " << focalNumFarms << " farms. ";}

	for (auto c2:cellsToCheck){	// loop through each cell (c2 as in "cell 2")
		// identify which cell ID is smaller/larger for grid value lookup, min comes first
		std::vector<double> ids = orderIDs(focalCell->get_id(),c2->get_id());
		double gridKernValue = gridCellKernel.at(ids[0]).at(ids[1]);
		if (gridKernValue > 0){ // only proceed if kernel > 0
		
		if (verbose){std::cout << std::endl << "Kernel: " << gridKernValue;}

		double compNumFarms = c2->get_num_farms(); // how many farms in comparison cell
		if (verbose){std::cout << " Comparison cell " << c2->get_id() <<": " << compNumFarms << " farms.";}

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
						s = 0; // remainingFarmProb recalculates to 1 for remainder of loop
						// get actual distances between farms
						f1x = f1 -> get_x();
						f1y = f1 -> get_y();
						f2x = f2 -> get_x();
						f2y = f2 -> get_y();
						xdiff = (f1x - f2x);
						ydiff = (f1y - f2y);
						distBWfarms = sqrt(xdiff*xdiff + ydiff*ydiff);
						// add kernel choice option
						kernelBWfarms = kernel(distBWfarms);
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
				} else { // otherwise if p(farm->cell) fails
					farmtocellskips++;
					farmsinskippedcells += compNumFarms;
				} // end if p(farm->cell) fails
			} // end for each focal cell farm			
			} // end "if gridkernel value > 0"
		} // end for loop through comparison cells
}

