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
	
  		// generate map of farms and xylimits
	 	std::clock_t loading_start = std::clock();
		Grid_manager G(pfile,0,0); // reverse x/y on/off, verbose on/off
		std::clock_t loading_end = std::clock();
	
 		std::cout << std::endl << "CPU time for loading premises: "
 			<< 1000.0 * (loading_end - loading_start) / CLOCKS_PER_SEC
 			<< "ms." << std::endl;
 			
		// set some proportion to be focal farms (same list for all reps)
		std::vector <std::vector<Farm*>> f_c_farms = G.fakeFarmStatuses(0.05);
		std::vector<Farm*> focalFarms = f_c_farms[0];
		std::vector<Farm*> compFarms = f_c_farms[1];
 			
 	bool griddingOn = 1;
if(griddingOn){
//  		std::vector <double> parList;
// 	  	parList.emplace_back(3000.0);
// 		parList.emplace_back(3500.0);
// 		parList.emplace_back(4000.0);
//    for (auto j:parList)	// for each value of j to run
//    {
	 	std::clock_t grid_start = std::clock();		
		G.initiateGrid(800,50000); // max farms in cell, kernel radius
//		G.initiateGrid(300000); // length of cell side
//		G.printCells(pfile); // option to print cells, based on specified prem file
 		
		std::clock_t grid_end = std::clock();
		double gridGenTimeMS = 1000.0 * (grid_end - grid_start) / CLOCKS_PER_SEC;
		std::cout << "CPU time for generating grid: " << gridGenTimeMS << "ms." << std::endl;

   	   for (auto i=0; i!=1; i++) // replicates per value
   		{	
 		// at one timestep:
		 std::cout << "Starting grid check: " << std::endl;
  		 std::clock_t gridcheck_start = std::clock();	  
	   	 G.stepThroughCellsAlt(focalFarms,compFarms);
  		 std::clock_t gridcheck_end = std::clock();
//  		 std::unordered_map<double, std::vector<double>> inf = G.getTotalInfections();
  		
 		 double gridCheckTimeMS = 1000.0 * (gridcheck_end - gridcheck_start) / CLOCKS_PER_SEC;
		 std::cout << "CPU time for checking grid: " << gridCheckTimeMS << "ms." << std::endl;			  
 		
// 		std::unordered_map<double, std::vector<double>> grid_inf = G.getTotalInfections();
// 	  	std::string allLinesToPrint, oneLine;
// 		char temp[10];
// 		for (auto l:focalFarms){ // for each focal Farm
// 			oneLine = "F\t"; // "F" designates focal farm
//  			sprintf(temp, "%f\t", l->get_id()); // farm ID
// 				oneLine += temp;
// 			oneLine+="0\t"; // 0 for infection count - can't be infected
// 		 	sprintf(temp, "%f\t", l->get_x()); // x coordinate
// 		 		oneLine += temp;
// 		 	sprintf(temp, "%f\t", l->get_y()); // y coordinate
// 		 		oneLine += temp;
// 		 	oneLine.replace(oneLine.end()-1, oneLine.end(), "\n"); // add line break 		
// 			allLinesToPrint += oneLine;
// 		 }
// 		for (auto k:grid_inf){ // for each farm that became infected
// 			oneLine = "C\t"; // "C" designates comparison farm
//  			sprintf(temp, "%f\t", k.first); // farm ID
// 				oneLine += temp;
//  			sprintf(temp, "%f\t", k.second[0]); // count of infections
// 		 		oneLine += temp;
// 		 	sprintf(temp, "%f\t", k.second[1]); // x coordinate
// 		 		oneLine += temp;
// 		 	sprintf(temp, "%f\t", k.second[2]); // y coordinate
// 		 		oneLine += temp;
// 		 	oneLine.replace(oneLine.end()-1, oneLine.end(), "\n"); // add line break 		
// 			allLinesToPrint += oneLine;
// 		}
// 		 

// 		std::string ofilename = "gridResults.txt";
// 		char temp[10];
// 	sprintf(temp, "%f", j);
// 	ofilename += temp;
// 	ofilename += "sidelength.txt";
//  	std::ofstream f(ofilename);
// 		if(f.is_open()){
// 			f << allLinesToPrint;
// 			f.close();
// 		}
		} // end for each rep i
// 	} // end for each parameter j
} // end "if griddingOn"

bool pairwiseOn = 0;
if(pairwiseOn){
	std::cout << "Conducting pairwise comparisons - go get a snack." << std::endl;
	std::unordered_map<double, int> infectedFarms;
// replicating the pairwise comparisons
	std::vector<double> inf;
for (auto i=0; i!=1; i++){
	std::cout << "Test #" << i << ": ";
	std::clock_t slow_start = std::clock();
	// run this farm by farm (no gridding) for comparison
		int totalcomparisons = 0;
 	
		for (auto f1:focalFarms)
		{
		double f1x = f1 -> get_x(); // get farm 1 x coordinate
		double f1y = f1 -> get_y(); // get farm 1 y coordinate

			for (auto f2:compFarms)
			{
				totalcomparisons++;
				double f2x = f2 -> get_x(); // get farm 2 x coordinate
				double f2y = f2 -> get_y(); // get farm 2 y coordinate
				
				double xdiff = (f1x - f2x);
				double ydiff = (f1y - f2y);
				double distBWfarms = sqrt(xdiff*xdiff + ydiff*ydiff);
				double kernelBWfarms = kernel(distBWfarms);
				// get farm infectiousness/susceptibility values 
				// (dep on farm size), assumes infectOut is true
				double farmInf = getFarmInf(f1);	
				double farmSus = getFarmSus(f2);

				// calculate probability between these specific farms
				double betweenFarmsProb = 1-exp(-farmSus * farmInf * kernelBWfarms); // prob tx between this farm pair
				// "prob3" in MT's Fortran code

				double random3 = unif_rand();
				if (random3 < betweenFarmsProb){
					// success... infect
					if (infectedFarms.count(f2->get_id())==0){ // if this farm hasn't been infected
						infectedFarms[f2->get_id()] = 1;
					} else {
						infectedFarms.at(f2->get_id())=infectedFarms.at(f2->get_id())+1;
					}
					std::cout << "Infection: dist bw farms = " << distBWfarms << std::endl;
				}
			}
		}
			
		std::cout << "Total infections (pairwise): " << infectedFarms.size() << std::endl <<
		 "  Total comparisons (pairwise): " << totalcomparisons << std::endl;

		std::clock_t slow_end = std::clock();
	
		std::cout << "CPU time for checking pairwise: "
				  << 1000.0 * (slow_end - slow_start) / CLOCKS_PER_SEC
				  << "ms." << std::endl << std::endl;
				  
// 		std::string toPrint;
// 		char temp[10];
// 		for(auto it = inf.begin(); it != inf.end(); it++){
// 			sprintf(temp, "%f\n", *it);
// 			toPrint += temp;
// 		}
// 
// 		std::string ofilename = "numPairwiseInf.txt";
// 		std::string headers = "NumInf";
// 		std::ofstream f(ofilename);
// 		if(f.is_open())
//  		{
// 			f << headers;
// 			f.close();
// 		}
// 	
// 		std::ofstream outfile;
// 		outfile.open("numPairwiseInf.txt", std::ios_base::app);
// 		outfile << toPrint;	
	
//		std::string ofilename = "numPairwiseInf.txt";
//		std::ofstream f(ofilename);
//		if(f.is_open())
// 		{
//			f << toPrint;
//			f.close();
//		}
	} // end for each i value
} // end if pairwiseOn

return 0;
}