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

#include "file_manager.h"
#include "Grid_manager.h"
#include "shared_functions.h"
#include "Shipment_manager.h"
#include "Status_manager.h"

bool verbose; // global variable

int main(int argc, char* argv[])
{

	std::string cfile; // Config file
// Check for command line arguments
	if(argc == 2){cfile = argv[1];}
	else if(argc < 2){
		std::cout << "ERROR: No config file specified." << std::endl <<
		"Exiting..." << std::endl;
		exit(EXIT_FAILURE);
	}
	else if(argc > 2){
		std::cout << "ERROR: Too many input arguments specified. Should only be config file." << std::endl <<
		"Exiting..." << std::endl;
		exit(EXIT_FAILURE);
	}

	file_manager fm; // construct file_manager object
	fm.readConfig(cfile); // reads config file, saves to pv, and checks for errors
	std::vector<std::string> pv = fm.getPV(); // get parameter vector
	
	// Element numbers correspond to those specified in config.txt
	// Batch settings
	//std::string batch_name = pv[0]; //Folder where this run will be saved. Currently win-only.
	int reps; str_cast(pv[1],reps); // Number of replicates to run - save pv[1] as int reps.
	bool griddingOn; str_cast(pv[2],griddingOn);
	bool pairwiseOn; str_cast(pv[3],pairwiseOn);
	// pv[4] ... pv[9]

	// General settings
	std::string pfile = pv[10]; // All premises filename
	std::string seedfile = pv[11]; // Initially infected premises filename
	//std::string seedFIPSfile = pv[12]; // Initially infected FIPS filename
	int timesteps; str_cast(pv[13],timesteps); // Total timesteps to run
	// pv[14]	//If run times for replicates should be saved (0 or 1).
	str_cast(pv[15],verbose); // turn global variable verbose on/off
	bool switchXY; str_cast(pv[16],switchXY); // if off, y is before x
	// pv[17] ... pv[19]
	
	// Grid-related settings
	int maxFarms=-1;
	double kernelRadius=-1;
	double cellSide=0;
	std::string cellFile = pv[20]; // Grid cells filename
	std::vector<double> gridParamsDensity = stringToNumVec(pv[21]); // Maximum farms per cell - split into ints
	if (gridParamsDensity.size()==0){
		maxFarms = gridParamsDensity[0];
		kernelRadius = gridParamsDensity[1];
	}
	str_cast(pv[22],cellSide); // Length of side for uniform cells	
	// pv[23] ... pv[28]
	std::string outCellFile = pv[29]; // Filename to write cells to
	
	// Shipment-related settings
	std::vector<int> coShipMethods = stringToIntVec(pv[30]); // Methods to use to ship county-county
	std::vector<int> coShipTimes = stringToIntVec(pv[31]); // Times to start using the above methods
		coShipTimes.emplace_back(timesteps+1); // whichElement function needs a maximum (element won't be used)

	int farmFarmMethod; str_cast(pv[32],farmFarmMethod); // Method to assign shipments to premises
	// pv[33] ... pv[37]
	// std::string outShipFile = pv[38]; // Filename to write shipments (with flagged bans) to
	// int outShipRes; str_cast(pv[39], outShipRes); // Output shipments at premises (0), county (1), or state (2) level
	
	// Infection-related settings
	int kernelType; str_cast(pv[40],kernelType); // for local (diffuse) spread
	// int quarantine = str_cast(pv[41]); // length of quarantine in days for all shipments
	std::vector<double> lp = stringToNumVec(pv[42]); // convert to numbers
		std::tuple<double,double> latencyParams = std::make_tuple(lp[0],lp[1]);
	std::vector<double> ip = stringToNumVec(pv[43]); // convert to numbers
		std::tuple<double,double> infectiousParams = std::make_tuple(ip[0],ip[1]);
	std::vector<std::string> speciesOnPrems = stringToStringVec(pv[44]); // get species names as vector of strings
	std::vector<double> spSus = stringToNumVec(pv[45]);
	std::vector<double> spInf = stringToNumVec(pv[46]);
	// pv[47]
	// std::string outStatusFile = pv[48];
	// std::vector<double> outStatuses = stringToNumVec(pv[49]);
	
	// Control-related settings
	std::vector<double> rp = stringToNumVec(pv[50]);
		std::tuple<double,double> reportParams = std::make_tuple(rp[0],rp[1]);
	int shipBanCompliance; str_cast(pv[51], shipBanCompliance); // Percent effectiveness
	int shipBanScale; str_cast(pv[52], shipBanScale); // resolution of shipping ban (0-county, 1-state)
	std::vector<int> shipParams = {shipBanCompliance, shipBanScale, farmFarmMethod};
	std::vector<double> bip = stringToNumVec(pv[53]);
		std::tuple<double,double> banInitParams = std::make_tuple(bip[0],bip[1]);
	std::vector<double> bcp = stringToNumVec(pv[54]);
		std::tuple<double,double> banComplianceParams = std::make_tuple(bcp[0],bcp[1]);
	//pv[55] ... pv[59]
	
	std::unordered_map< std::string, std::tuple<double,double> > lagParams;
												// Time lag (mean and variance) between:
		lagParams["latency"] = latencyParams; 		//exposed-infectious
		lagParams["infectious"] = infectiousParams; 	//infectious-recovered
		lagParams["report"] = reportParams; 		//exposed-reported
		lagParams["startBan"] = banInitParams; 		//reported-banned
		lagParams["complyBan"] = banComplianceParams; // banned-compliant
		
	
//~~~~~~~ Parameters loaded and ready to start...
 	
  		// generate map of farms and xylimits
  	 	std::clock_t loading_start = std::clock();
  	 	// load file containing premises and related info
		Grid_manager G(pfile,switchXY,speciesOnPrems,spSus,spInf);
		
		// for instantiating Shipment manager: FIPS of loaded farms
		std::unordered_map<std::string, std::vector<Farm*>> fipsmap = G.get_FIPSmap();
		
		// get full list of farms
		std::unordered_map<int, Farm*> allFarms = G.get_allFarms(); 

		std::clock_t loading_end = std::clock();
	
 		if(verbose){
 			std::cout << std::endl << "CPU time for loading premises: "
 			<< 1000.0 * (loading_end - loading_start) / CLOCKS_PER_SEC
 			<< "ms." << std::endl;
 		}
 		
		// initiate grid
	 	std::clock_t grid_start = std::clock();		
	 	// if file provided, use that
	 	if (cellFile!=""){G.initiateGrid(cellFile);} // reading in/making cells takes ~45 sec
	 	// else use density params
	 	else if (maxFarms>-1 && kernelRadius >-1){G.initiateGrid(maxFarms,kernelRadius);}
	 	// else use uniform params
	 	else if (cellSide!=0){G.initiateGrid(cellSide);}
	 	else {std::cout<<"Error (main.cpp): no gridding parameters suitable for initiation"<<std::endl<<
	 		"Exiting..." << std::endl;
			exit(EXIT_FAILURE);}
	 	
		// to do: if filename provided, turn on bool to print cells - maybe through file manager?
		
 		std::clock_t grid_end = std::clock();
		double gridGenTimeMS = 1000.0 * (grid_end - grid_start) / CLOCKS_PER_SEC;
		std::cout << "CPU time for generating grid: " << gridGenTimeMS << "ms." << std::endl;

// ideally start loop here
for (auto r=1; r<=reps; r++){
		std::clock_t rep_start = std::clock();
		// randomly pick a proportion to be focal farms and print to external file - for testing
//		std::string fname = "seedFarms.txt"; Status.pickInfAndPrint(0.05, allFarms, fname)
	if(griddingOn){
		// load initially infected farms and instantiate Status manager
		// note that initial farms are started as infectious (2) rather than exposed (1)
		Status_manager Status(seedfile, lagParams, allFarms, timesteps);	 	
		Shipment_manager Ship(fipsmap, shipParams);

 		int t=0;		
 		// set focalFarms as all farms with status inf at time 0
		std::vector<Farm*> focalFarms = Status.premsWithStatus("inf", t);
		// set compFarms as all farms with status sus at time 0
		std::vector<Farm*> compFarms = Status.premsWithStatus("sus", t);
		int numExposed = Status.premsWithStatus("exp", t).size();
		bool potentialTx = (focalFarms.size()!=0 && compFarms.size()!=0) || 
			(numExposed>0 && compFarms.size()!=0);

   	   while (t<timesteps && potentialTx){ // timesteps, stop early if dies out
   	   	 t++; // starts at 1
   	   	 
   		 std::cout << std::endl<<"Timestep "<<t<<": "
   		 	<<Status.premsWithStatus("sus", t).size()<<" susceptible, "
   		 	<<Status.premsWithStatus("exp", t).size()<<" exposed, "
   		 	<<Status.premsWithStatus("inf", t).size()<<" infectious, "
   		 	<<Status.premsWithStatus("imm", t).size()<<" immune premises. "<<std::endl
   		 	<<Status.FIPSWithStatus("reported", t).size()<<" FIPS reported. "
   		 	<<Status.FIPSWithStatus("banOrdered", t).size()<<" FIPS with ban ordered. "
   		 	<<Status.FIPSWithStatus("banActive", t).size()<<" FIPS with active shipping ban. "
   		 	<<std::endl;
   		 
   		 // update all farm/FIPS statuses: at the proper times, exposed become infectious, infectious recover,
   		 // exposed are reported, reported are banned, banned are compliant
   		 Status.updates(t);
   		 
   		 // determine infections that will happen from local diffusion
		 if(verbose){std::cout << "Starting grid check (local spread): "<<std::endl;}
  		 std::clock_t gridcheck_start = std::clock();	  
		 G.stepThroughCells(focalFarms,compFarms);
			  	 
  		 std::clock_t gridcheck_end = std::clock();
  		 double gridCheckTimeMS = 1000.0 * (gridcheck_end - gridcheck_start) / CLOCKS_PER_SEC;
 		 if(verbose){std::cout << "CPU time for checking grid: " << gridCheckTimeMS << "ms." << std::endl
 		 	<< "Starting shipments: ";		 }
 		 // determine infections that will happen from shipments	 
 		 std::clock_t ship_start = std::clock();	  
 		 std::vector<std::string> bannedFIPS = Status.FIPSWithStatus("banActive", t);
 		 // assign county method according to time
 		 
 		 int cmElement = whichElement(t, coShipTimes); // which time span does t fall into
 		 int countyMethod = coShipMethods[cmElement]; // get matching shipment method
 		 Ship.makeShipments(focalFarms, compFarms, countyMethod, bannedFIPS);
 		  		 
 		 std::vector<std::tuple<int,int,int,bool>> infShips = Ship.get_infFarmShipments(); // need this for later
 		 std::cout << infShips.size()<<" infectious shipments, of which "<<std::endl;
 		 int banCount = 0;
 		 for (auto& shipment:infShips){if(std::get<3>(shipment)==1){banCount++;}}
 		 std::cout << banCount<<" were banned (didn't happen)."<<std::endl;
 		 std::clock_t ship_end = std::clock();
 		 double shipTimeMS = 1000.0 * (ship_end - ship_start) / CLOCKS_PER_SEC;
 		 if(verbose){std::cout << "CPU time for shipping: " << shipTimeMS << "ms." << std::endl;}
 		 
 		 // actually change statuses:
 		 // infections from local spread
 		 std::vector<Farm*> gridInf = G.get_infectedFarms();

 		 // infections from shipping (exclude banned shipments)
 		 std::vector<Farm*> shipInf; shipInf.clear();
 		 for (auto& is:infShips){
 		  if (std::get<3>(is)==0){ // if this shipment was not banned
 		 	int destFarmID = std::get<1>(is); // get destination farm ID
 		 	shipInf.emplace_back(allFarms.at(destFarmID)); // add to list of farms to become exposed
 		  }
 		 }
 		 
 		 // combine & eliminate duplicates
 		 std::vector<std::vector<Farm*>> toCombine {gridInf, shipInf}; // create a vector of vectors called "toCombine"
 		 std::vector<Farm*> makeExposed = uniqueFrom(toCombine); // eliminate duplicates
 		 // change statuses for these farms
 		 Status.changeTo("exp", makeExposed, t, latencyParams);
		 if(verbose){
		 	std::cout<<gridInf.size()<<" farms now exposed from local spread. "<<shipInf.size()
		 	<<" farms now exposed from shipments."<<std::endl;
		 }
		focalFarms = Status.premsWithStatus("inf", t);
		compFarms = Status.premsWithStatus("sus", t);
		numExposed = Status.premsWithStatus("exp", t).size();
		potentialTx = (focalFarms.size()!=0 && compFarms.size()!=0) || 
			(numExposed>0 && compFarms.size()!=0);
//		std::cout<<"Retreived from statusTimeFarms: "<<focalFarms.size()<<" infectious and "<<compFarms.size()<<
//		" susceptible and "<<expFarms.size()<<" exposed farms."<<std::endl;
 		}  	// end "while under time and infectious and susceptible farms remain"
 		
 		std::clock_t rep_end = std::clock();
 		double repTimeMS = 1000.0 * (rep_end - rep_start) / CLOCKS_PER_SEC;
		std::cout << "CPU time for rep "<<r<<": " << repTimeMS << "ms." << std::endl;
	} // end "if griddingOn"

} // end for loop

/*
bool printInfFarms = 0;
if(pairwiseOn){
// 1,949,147,792 comparisons
	  	// generate map of farms and xylimits
  	 	std::clock_t loading_start = std::clock();
		Grid_manager G(pfile,0); // reverse x/y on/offs
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
*/
return 0;
}
