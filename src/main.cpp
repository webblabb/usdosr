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

#include "Grid_manager.h"

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
	
// 	std::vector <double> sideLengths;
//  	sideLengths.emplace_back(30000.0);
	//sideLengths.emplace_back(10000.0);

//   for (auto j:sideLengths)	// for each value of j to run
//   {
  		// generate map of farms and xylimits
	 	std::clock_t loading_start = std::clock();
		Grid_manager G(pfile,0,0); // reverse x/y on/off, verbose on/off
		std::clock_t loading_end = std::clock();
	
 		std::cout << std::endl << "CPU time for loading premises: "
 			<< 1000.0 * (loading_end - loading_start) / CLOCKS_PER_SEC
 			<< "ms." << std::endl;
	 	std::clock_t grid_start = std::clock();		
// 		std::string cellfile="max250f_7328c_USprems.txt"; G.initiateGrid(cellfile);  // filename with cells
//		G.initiateGrid(400,50000); // max farms in cell, kernel radius
		G.initiateGrid(1000); // length of cell side
 //		G.printCells(pfile); // option to print cells, based on specified prem file
//		G.printGridValues();
 		
// 		std::clock_t grid_end = std::clock();
// 		double gridGenTimeMS = 1000.0 * (grid_end - grid_start) / CLOCKS_PER_SEC;
// 		std::cout << "CPU time for generating grid: " << gridGenTimeMS << "ms." << std::endl;
// 
// 		// use first x focal and comparison farms (same list for all reps)
// 		std::vector <std::vector<Farm*>> f_c_farms = G.fakeFarmStatuses(0.05);
// 		std::vector<Farm*> focalFarms = f_c_farms[0];
// 		std::vector<Farm*> compFarms = f_c_farms[1];
		// std::unordered_map<int, Farm*> af = G.get_allFarms();
// 		std::vector<Farm*> focalFarms;
// 		for (auto f:af){focalFarms.emplace_back(f.second);}
// 		std::vector<Farm*> compFarms = focalFarms;

//  	std::string allLinesToPrint;
//    	   for (auto i=0; i!=1; i++) // replicates per value
//    		{	
// 		// at one timestep:
// 		 std::cout << "Starting grid check: " << std::endl;
//   		 std::clock_t gridcheck_start = std::clock();	  
// 	   	 G.stepThroughCells(focalFarms,compFarms);
//   		 std::clock_t gridcheck_end = std::clock();
//   		
//  		 double gridCheckTimeMS = 1000.0 * (gridcheck_end - gridcheck_start) / CLOCKS_PER_SEC;
// 		 std::cout << "CPU time for checking grid: " << gridCheckTimeMS << "ms." << std::endl;			  
// 		
// 		std::string oneLine;
// 			oneLine.reserve(30);
// 		char temp[10];
// 		sprintf(temp, "%f\t", gridGenTimeMS); // convert times to characters
// 		oneLine += temp;
// 		sprintf(temp, "%f\t", gridCheckTimeMS);
// 		oneLine += temp;
// 		sprintf(temp, "%d\t", G.getTotalInfections());
// 		oneLine += temp;
// 		oneLine.replace(oneLine.end()-1, oneLine.end(), "\n");
// 		
// 		allLinesToPrint += oneLine;

// 	std::string ofilename = "gridResults";
// 	char temp[10];
// 	sprintf(temp, "%f", j);
// 	ofilename += temp;
// 	ofilename += "sidelength.txt";
// 	std::ofstream f(ofilename);
// 	if(f.is_open())
// 	{
// 		f << allLinesToPrint;
// 		f.close();
// 	}
//	} //end for each rep


	// replicating the pairwise comparisons
// 	std::vector<double> inf;
// for (auto i=0; i!=10; i++){
// 	std::cout << "Test #" << i << ": ";
// 	std::clock_t slow_start = std::clock();
// 	// run this farm by farm (no gridding) for comparison
// 		int totalinfections = 0;
// 		int totalcomparisons = 0;
// // 	
// 		for (auto f1:focalFarms)
// 		{
// 			for (auto f2:compFarms)
// 			{
// 				totalcomparisons++;
// 				double f1x = f1 -> get_x(); // get farm 1 x coordinate
// 				double f1y = f1 -> get_y(); // get farm 1 y coordinate
// 				double f2x = f2 -> get_x(); // get farm 2 x coordinate
// 				double f2y = f2 -> get_y(); // get farm 2 y coordinate
// 				double xdiff = (f1x - f2x);
// 				double ydiff = (f1y - f2y);
// 				double distBWfarms = sqrt(xdiff*xdiff + ydiff*ydiff);
// 				double kernelBWfarms = kernel(distBWfarms);
// 				// get farm infectiousness/susceptibility values (assumes infectOut is true)
// 				double farmInf = getFarmInf(f1);	
// 				double farmSus = getFarmSus(f2);
// 
// 				// calculate probability between these specific farms
// 				double betweenFarmsProb = 1-exp(-farmSus * farmInf * kernelBWfarms); // prob tx between this farm pair
// 				// "prob3" in MT's Fortran code
// 
// 				double random3 = unif_rand();
// 				if (random3 < betweenFarmsProb){
// 					// success... infect
// 					totalinfections++;
// 					}
// 			}
// 		}
// 		inf.emplace_back(totalinfections);
// 			
// 		std::cout << "Total infections (pairwise): " << totalinfections << std::endl <<
// 		 "  Total comparisons (pairwise): " << totalcomparisons << std::endl;
// 
// 	std::clock_t slow_end = std::clock();
// 	
// 	std::cout << "CPU time for checking pairwise: "
// 				  << 1000.0 * (slow_end - slow_start) / CLOCKS_PER_SEC
// 				  << "ms." << std::endl << std::endl;
//  	} // end for each i value
// 
// std::string toPrint;
// char temp[10];
// for(auto it = inf.begin(); it != inf.end(); it++)
// 	{
// 		sprintf(temp, "%f\n", *it);
// 		toPrint += temp;
// 	}
// 	
// 	std::string ofilename = "numPairwiseInf.txt";
// 	std::ofstream f(ofilename);
// 	if(f.is_open())
// 	{
// 		f << toPrint;
// 		f.close();
// 	}
// 


//	} // end for each j value
return 0;
}