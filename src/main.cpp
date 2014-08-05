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
#include <fstream>
#include <vector>

// for slow calc
#include <cmath> // std::sqrt
#include <unordered_map>

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
	
// 	std::vector <unsigned int> maxFarms;
// 	maxFarms.emplace_back(500);
// 	maxFarms.emplace_back(400);
// 	maxFarms.emplace_back(300);
// 	maxFarms.emplace_back(200);
// 	maxFarms.emplace_back(100);
// 	maxFarms.emplace_back(50);
//	maxFarms.emplace_back(25);
// 	maxFarms.emplace_back(15);
// 	maxFarms.emplace_back(10);
// 	maxFarms.emplace_back(5);
// 		
//  for (auto j:maxFarms)	// for each value of maxFarms to run
//  {
//  	std::string allLinesToPrint;
// 	for (auto i=0; i!=100; i++) // replicates per value
// 	{
		// generate map of farms and xylimits
	 	std::clock_t loading_start = std::clock();
		Grid_Creator G(pfile,0); // 1 turns on verbose option
		std::clock_t loading_end = std::clock();
	
 		std::cout << std::endl << "CPU time for loading premises: "
 			<< 1000.0 * (loading_end - loading_start) / CLOCKS_PER_SEC
 			<< "ms." << std::endl;
		
		// allocate farms to grid cells by density		  
 		std::clock_t grid_start = std::clock();	
// 		G.initiateGrid(15,50);
		std::string cellfile = "cellList_7772cells.txt";
		G.initiateGrid(cellfile); // max farms in cell, kernel radius OR filename with cells
 		std::cout << "Grid created." << std::endl;
 		
 		std::clock_t grid_end = std::clock();
// 		double gridGenTimeMS = 1000.0 * (grid_end - grid_start) / CLOCKS_PER_SEC;
		
 		// step through as if all farms are infectious and susceptible
 		std::clock_t gridcheck_start = std::clock();	  
 		// copy cell list and kernel values for generated grid G
//  		std::unordered_map<double, grid_cell*> allCells = G.get_allCells();
//  		std::unordered_map<double, std::unordered_map<double, double>> gridCellKernel = G.get_gridCellKernel();
//  		std::unordered_map<double, std::vector<double>> neighbors = G.get_neighbors();
 		// feed into cell checker
  		Grid_cell_checker gridder(
  			G.get_allCells(), 
  			G.get_gridCellKernel(), 
  			G.get_neighbors(), 
  			1,// verbose
 			1);  // infectOut
 		std::clock_t gridcheck_end = std::clock();
//		double gridCheckTimeMS = 1000.0 * (gridcheck_end - gridcheck_start) / CLOCKS_PER_SEC;
		
		std::cout << "CPU time for generating grid: "
				  << 1000.0 * (grid_end - grid_start) / CLOCKS_PER_SEC
				  << "ms." << std::endl;

		std::cout << "CPU time for checking grid: "
				  << 1000.0 * (gridcheck_end - gridcheck_start) / CLOCKS_PER_SEC
				  << "ms." << std::endl;			  
		
// 		std::string oneLine;
// 			oneLine.reserve(30);
// 		char temp[10];
// 		sprintf(temp, "%f\t", gridGenTimeMS); // convert times to characters
// 		oneLine += temp;
// 		sprintf(temp, "%f\t", gridCheckTimeMS);
// 		oneLine += temp;
// 		sprintf(temp, "%d\t", gridder.getTotalInfections());
// 		oneLine += temp;
// 		oneLine.replace(oneLine.end()-1, oneLine.end(), "\n");
// 		
// 		allLinesToPrint += oneLine;
// 		}
// 	// end for each value of maxFarms loop
// 	
// 	std::string ofilename = "gridResults";
// 	char temp[10];
// 	sprintf(temp, "%d", j);
// 	ofilename += temp;
// 	ofilename += "farms.txt";
// 	std::ofstream f(ofilename);
// 	if(f.is_open())
// 	{
// 		f << allLinesToPrint;
// 		f.close();
// 	}
//  }
/*	// replicating the pairwise comparisons
	bool pairwise = 0;
for (auto i=0; i!=1000; i++){
	std::cout << "Test #" << i << ": ";
	std::clock_t slow_start = std::clock();
	if (pairwise){
	// run this farm by farm (no gridding) for comparison
		int totalinfections = 0;
		int totalcomparisons = 0;
		std::unordered_map<int, Farm*> allFarms = G.get_allFarms();
	
		for (auto f1:allFarms)
		{
			for (auto f2:allFarms)
			{
				totalcomparisons++;
				double f1x = f1.second -> get_x(); // get farm 1 x coordinate
				double f1y = f1.second -> get_y(); // get farm 1 y coordinate
				double f2x = f2.second -> get_x(); // get farm 2 x coordinate
				double f2y = f2.second -> get_y(); // get farm 2 y coordinate
				double xdiff = (f1x - f2x);
				double ydiff = (f1y - f2y);
				double distBWfarms = sqrt(xdiff*xdiff + ydiff*ydiff);
				double kernelBWfarms = linearDist(distBWfarms);
				// get farm infectiousness/susceptibility values (assumes infectOut is true)
				double farmInf = getFarmInf(f1.second);	
				double farmSus = getFarmSus(f2.second);

				// calculate probability between these specific farms
				double betweenFarmsProb = 1-exp(-farmSus * farmInf * kernelBWfarms); // prob tx between this farm pair
				// "prob3" in MT's Fortran code
				double random3 = unif_rand();
				if (random3 < betweenFarmsProb){
					// success... infect
					totalinfections++;
					}
			}
		}
			inf.emplace_back(totalinfections);
			
		std::cout << "Total infections (pairwise): " << totalinfections << std::endl;
//		 "  Total comparisons (pairwise): " << totalcomparisons << std::endl;
	}
	std::clock_t slow_end = std::clock();
	
	if (pairwise){
	std::cout << "CPU time for checking pairwise: "
				  << 1000.0 * (slow_end - slow_start) / CLOCKS_PER_SEC
				  << "ms." << std::endl << std::endl;
	}
}

char temp[10];
for(auto it = inf.begin(); it != inf.end(); it++)
	{
		sprintf(temp, "%d\t", *it);
		toPrint += temp;
	}
	toPrint.replace(toPrint.end()-1, toPrint.end(), "\n");
	
	std::string ofilename = "numPairwiseInf.txt";
	std::ofstream f(ofilename);
	if(f.is_open())
	{
		f << toPrint;
		f.close();
	}
*/
	
return 0;
}