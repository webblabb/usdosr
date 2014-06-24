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
 	G.initiateGrid(200,10); // max 200 farms in cell, kernel radius 10
	std::clock_t grid_end = std::clock();
	std::cout << "CPU time for generating grid: "
			  << 1000.0 * (grid_end - grid_start) / CLOCKS_PER_SEC
			  << "ms." << std::endl;
	//G.printCells(); // cellList.txt: columns are id, x, y, s, number of farms
	G.makeCellRefs();
	
return 0;

// to do:
// fill in kernel
}