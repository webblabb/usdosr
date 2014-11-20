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
#include "Shipment_manager.h"

int main(int argc, char* argv[])
{
	// read in file
	std::string pfile; // file containing premises
	if(argc > 1){
		pfile = argv[1];
	} else {
		std::cout << "ERROR: No premises file specified." << std::endl <<
		"Exiting..." << std::endl;
		exit(EXIT_FAILURE);
	}

	int timesteps = 1;
 	bool griddingOn = 1;
 	bool printgInfFarms = 0;
if(griddingOn){

  		// generate map of farms and xylimits
  	 	std::clock_t loading_start = std::clock();
		Grid_manager G(pfile,0,0); // reverse x/y on/off, verbose on/off
		std::unordered_map<std::string, std::vector<Farm*>> fipsmap = G.get_FIPSmap();
		Shipment_manager S(fipsmap);
		std::clock_t loading_end = std::clock();
	
 		std::cout << std::endl << "CPU time for loading premises: "
 			<< 1000.0 * (loading_end - loading_start) / CLOCKS_PER_SEC
 			<< "ms." << std::endl;
 		
		// pick a proportion to be focal farms and print to external file
//		std::vector <std::vector<Farm*>> f_c_farms = G.setFarmStatuses(0.05);
//		std::vector<Farm*> focalFarms = f_c_farms[0];
//		std::string fname = "seedFarms.txt";
//		G.printVector(focalFarms,fname);

	// load initial farms from file
		std::string fname = "seedFarms.txt";
	 	std::vector<Farm*> focalFarms1, compFarms1;
	 	std::unordered_map<int, Farm*> allFarms = G.get_allFarms(); 
	 	for (auto& x:allFarms){compFarms1.emplace_back(x.second);}
	 	// "compFarms" first holds all farms, then is pared down and used as comparison farm list
	 	int fID;
 		std::ifstream f(fname);
		if(!f){std::cout << "Input file not found." << std::endl;}
		if(f.is_open()){
		std::cout << "Loading seed farms." << std::endl;
			while(! f.eof()){
				std::string line;
				getline(f, line); // get line from file "f", save as "line"			
				if(! line.empty()){ // if line has something in it
					str_cast(line, fID);
					focalFarms1.emplace_back(allFarms.at(fID));
				} // close "if line_vector not empty"
			} // close "while not end of file"
		} // close "if file is open"	
 		removeFarmSubset(focalFarms1,compFarms1); // removes focalFarms from compFarms, now compFarms only has compFarms

	 	std::clock_t grid_start = std::clock();		
		G.initiateGrid(2000,50000); // max farms in cell, kernel radius
//		std::string cellfile = "2000f_731c_USprems.txt"; G.initiateGrid(cellfile); // reading in/making cells takes ~ 60 sec
 		std::clock_t grid_end = std::clock();
		double gridGenTimeMS = 1000.0 * (grid_end - grid_start) / CLOCKS_PER_SEC;
		std::cout << "CPU time for generating grid: " << gridGenTimeMS << "ms." << std::endl;
/*		
	for (auto r=0; r!=1; r++){
	
		std::vector<Farm*> focalFarms = focalFarms1;
		std::vector<Farm*> compFarms = compFarms1;
		int runningTotal = 0;
		
		// write initially infected farms to file
// 		if (printgInfFarms == 1){
// 		int j=0;
// 		std::string toPrint;
// 		char temp[6];
// 		std::string outfilename = "gridInfFarms";
// 		sprintf(temp, "%d", int(j));
// 			outfilename += temp;
// 		outfilename += "_rep";
// 		sprintf(temp, "%d", r);
// 			outfilename += temp;
// 		outfilename += ".txt";
// 		std::ofstream outfile(outfilename);
// 		  for (auto fi:focalFarms1){		  	
// 			sprintf(temp, "%d\t", -1); // timestep = -1
// 				toPrint	+= temp; 
// 			toPrint += to_string(fi); // farm info and line break
// 		  } // end for each farm
// 		  if(outfile.is_open()){
// 			outfile << toPrint;
// 			outfile.close();
// 		  }
// 		} // end if print infected farms

 		int t=0;
   	   while (t!=timesteps && focalFarms.size()!=0 && compFarms.size()!=0){ // timesteps, stop early if dies out
   		 std::cout << focalFarms.size() << " focal farms and " << compFarms.size() << " comparison farms." << std::endl;
		 std::cout << "Starting grid check (local spread): " << std::endl;
  		 std::clock_t gridcheck_start = std::clock();	  
		 G.stepThroughCells(focalFarms,compFarms);
		 std::vector<Farm*> gInfFarms = G.getInfVec();
		
// 		 if (printgInfFarms == 1){
// 		 	std::cout << "t = " << t <<std::endl;
// 		 	std::string toPrint2;
// 		 	char temp[4];
// 	  	 for (auto fi:gInfFarms){
// 	    	sprintf(temp, "%d\t", t);
// 				toPrint2 += temp; // timestep
// 			toPrint2 += to_string(fi); // farm info and line break
// 	  	 }
// 	  	 outfile.open(outfilename, std::ios_base::app); // append to existing file
// 		 outfile << toPrint2;
// 		 outfile.close();
// 	  	 } // end if print grid infected farms
	  	 
  		 std::clock_t gridcheck_end = std::clock();
  		 double gridCheckTimeMS = 1000.0 * (gridcheck_end - gridcheck_start) / CLOCKS_PER_SEC;
 		 std::cout << "CPU time for checking grid: " << gridCheckTimeMS << "ms." << std::endl
 		 	<< "Starting shipments: ";		 
 		 // initiate shipments
 		 S.makeShipments(focalFarms,compFarms);
 		 std::vector<std::tuple<std::string,std::string,int>> coShips = S.get_countyShipments();
 		 std::cout <<std::endl<< coShips.size()<<" county shipments, ";
 		 std::vector<std::tuple<int,int,int>> fShips = S.get_farmShipments();
 		 std::cout << fShips.size()<<" farm-farm shipments."<<std::endl;
 		 std::vector<std::tuple<int,int,int>> infShips = S.get_infFarmShipments();
 		 std::cout << infShips.size()<<" infectious shipments."<<std::endl;

 	// 	 
// 		std::vector<Farm*> newInf = G.getInfVec();
// 			 for (auto y:newInf){focalFarms.emplace_back(y);}
// 			 G.removeFarmSubset(newInf,compFarms);
// 			 runningTotal += newInf.size();
// 			 std::cout << "Cumulative infections: " << runningTotal <<std::endl;
//  		 }
 		 t++;
 		}  	
 	} // end for each rep r	

*/
} // end "if griddingOn"

bool pairwiseOn = 0; // 1,949,147,792 comparisons
bool printInfFarms = 0;
if(pairwiseOn){
	  	// generate map of farms and xylimits
  	 	std::clock_t loading_start = std::clock();
		Grid_manager G(pfile,0,0); // reverse x/y on/off, verbose on/off
		std::clock_t loading_end = std::clock();
	
 		std::cout << std::endl << "CPU time for loading premises: "
 			<< 1000.0 * (loading_end - loading_start) / CLOCKS_PER_SEC
 			<< "ms." << std::endl;
 		
		// pick a proportion to be focal farms and print to external file
//		std::vector <std::vector<Farm*>> f_c_farms = G.setFarmStatuses(0.05);
//		std::vector<Farm*> focalFarms = f_c_farms[0];
//		std::string fname = "seedFarms.txt";
//		G.printVector(focalFarms,fname);

	// load initial farms from file
		std::string fname = "seedFarms.txt";
	 	std::vector<Farm*> focalFarms1, compFarms1;
	 	std::unordered_map<int, Farm*> allFarms = G.get_allFarms(); 
	 	for (auto x:allFarms){compFarms1.emplace_back(x.second);}
	 	// compFarms first holds all farms for ID reference, then is pared down and used as comparison farm list
	 	double fID;
 		std::ifstream f(fname);
		if(!f){std::cout << "Input file not found." << std::endl;}
		if(f.is_open()){
		std::cout << "Loading seed farms." << std::endl;
			while(! f.eof()){
				std::string line;
				getline(f, line); // get line from file "f", save as "line"			
				if(! line.empty()){ // if line has something in it
					str_cast(line, fID);
					focalFarms1.emplace_back(compFarms1.at(fID)); // using compFarms for reference here
				} // close "if line_vector not empty"
			} // close "while not end of file"
		} // close "if file is open"	
 		removeFarmSubset(focalFarms1,compFarms1); // removes focalFarms from compFarms, now compFarms only has compFarms

	std::cout << "Conducting pairwise comparisons - go get a snack." << std::endl;
	std::vector<Farm*> focalFarms = focalFarms1;
	std::vector<Farm*> compFarms = compFarms1;
	std::unordered_map<double, int> infectedFarms;
	int t=0;

	// write initially infected farms to file
// 	if (printInfFarms == 1){
// 	  for (auto fi:focalFarms){
// 	    char temp[4];
// 	    sprintf(temp, "%d\t", -1);
// 		std::string toPrint	= temp; // timestep = -1
// 		toPrint += to_string(fi); // farm info and line break
// 		std::ofstream outfile;
// 		outfile.open("pairwiseInfFarms.txt", std::ios_base::app); // append to existing file
// 		outfile << toPrint;	
// 	  }
// 	}
	
  while (t!=timesteps && focalFarms.size()!=0 && compFarms.size()!=0){
	std::cout << "Timestep " << t << ": ";
	std::clock_t slow_start = std::clock();
	// run this farm by farm (no gridding) for comparison
//		int totalcomparisons = 0;
 		int runningTotal = 0;
		for (auto& f1:focalFarms)
		{
		double f1x = f1 -> Farm::get_x(); // get farm 1 x coordinate
		double f1y = f1 -> Farm::get_y(); // get farm 1 y coordinate

			for (auto& f2:compFarms)
			{
//				totalcomparisons++;
				double f2x = f2 -> Farm::get_x(); // get farm 2 x coordinate
				double f2y = f2 -> Farm::get_y(); // get farm 2 y coordinate
				
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
					double infFarmID = f2->Farm::get_id();
					if (infectedFarms.count(infFarmID)==0){ // if this farm hasn't been infected
						infectedFarms[infFarmID] = 1;
					} else {
						infectedFarms.at(infFarmID)=infectedFarms.at(infFarmID)+1;
					}
					if (printInfFarms == 1){
						char temp[4];
	   					sprintf(temp, "%d\t", t);
						std::string toPrint	= temp;
						toPrint += to_string(f2); // farm info and line break
						std::ofstream outfile;
						outfile.open("pairwiseInfFarms.txt", std::ios_base::app); // append to existing file
						outfile << toPrint;	
					}
				}
			}
		} // end for all farm comparisons
			
		std::cout << "New infections (pairwise): " << infectedFarms.size() << std::endl;
//		<< "  Total comparisons (pairwise): " << totalcomparisons << std::endl;

		std::clock_t slow_end = std::clock();
	
		std::cout << "CPU time for checking pairwise: "
				  << 1000.0 * (slow_end - slow_start) / CLOCKS_PER_SEC
				  << "ms." << std::endl << std::endl;

		// reassign infected farms from comp to focal	
		if (timesteps >1){		  
			 runningTotal += infectedFarms.size();
			 std::cout << "Cumulative infections: " << runningTotal <<std::endl; 
			 std::vector<Farm*> newInfVec;
			 for (auto y:infectedFarms){
			 	newInfVec.emplace_back(allFarms.at(y.first)); // put new inf farms in vector
			 	focalFarms.emplace_back(allFarms.at(y.first));} // add to infectious list
			 removeFarmSubset(newInfVec,compFarms); // remove from comparison list
			 t++;
			}  // end if timesteps >1
		} // end while spread still possible and under timesteps
} // end if pairwiseOn

return 0;
}
