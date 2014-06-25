// For testing gridding functions

// Input is artificial premises layout (run 'a prems.txt')
// - start with evenly spaced grid
// Check that cells are placed/oriented as expected
// Check that farms are assigned to cells as expected
// Check that distances between cells are as expected
// Check that kernel values are as expected
// Test range of cell sizes

#include <iostream>
#include <string>
#include <ctime>
#include <stdlib.h>

// #include "grid_cell.h"
#include "Grid_Creator.h"
#include "Grid_cell_checker.h"

int main(int argc, char* argv[])
{
	// read in file
	std::string pfile; // file containing premises
	if(argc > 1)
	{
		pfile = argv[1];
	}
	else
	{
		std::cout << "ERROR: No premises file specified." << std::endl <<
		"Exiting..." << std::endl;
		exit(EXIT_FAILURE);
	}
	
	// (timed) generate map of farms and xylimits
	std::clock_t loading_start = std::clock();
	Grid_Creator G(pfile,1); // 1 turns on verbose option
	std::clock_t loading_end = std::clock();
	std::cout << "CPU time for loading data: "
			  << 1000.0 * (loading_end - loading_start) / CLOCKS_PER_SEC
			  << "ms." << std::endl;
			  
	std::clock_t grid_start = std::clock();	  
 	G.initiateGrid(6000,40); // max farms in cell, kernel radius
	std::clock_t grid_end = std::clock();
	std::cout << "CPU time for generating grid: "
			  << 1000.0 * (grid_end - grid_start) / CLOCKS_PER_SEC
			  << "ms." << std::endl << std::endl;
	
	// get cell list and kernel values for generated grid G
	std::vector<grid_cell*> allCells = G.get_allCells();
	std::cout << "Cell IDs: ";
	for (auto c:allCells)
	{
	std::cout << c->get_id() << ", ";
	}
	std::cout << std::endl;
	
	std::unordered_map<double, std::unordered_map<double, double>> gridCellKernel = G.get_gridCellKernel();

	// feed into cell checker
	Grid_cell_checker gridder(allCells, gridCellKernel, 1,// verbose
		 1);  // infectOut
	
return 0;
}