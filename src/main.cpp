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

int verboseLevel; // global variable
int verbose = verboseLevel; // local interpretation of global verboseLevel

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
	
//~~~~~~~ Copy/apply configuration settings
	// Element numbers correspond to those specified in config.txt
	// Batch settings
	//std::string batch_name = pv[0]; //Folder where this run will be saved. Currently win-only.
	int reps; str_cast(pv[1],reps); // Number of replicates to run - save pv[1] as int reps.
	bool pairwiseOn; str_cast(pv[2],pairwiseOn);
	// pv[3] ... pv[9]

	// General settings
	std::string pfile = pv[10]; // All premises filename
	std::string seedfile = pv[11]; // Initially infected premises filename
	bool oneRandomSeed; str_cast(pv[12],oneRandomSeed); // Pick one premises at random from 11 to be infected
	int timesteps; str_cast(pv[13],timesteps); // Total timesteps to run
	// pv[14]	//If run times for replicates should be saved (0 or 1).
	str_cast(pv[15],verboseLevel); // set level of global variable verbose
	bool switchXY; str_cast(pv[16],switchXY); // if off, y is before x
	// pv[17] ... pv[19]
	
	// Grid-related settings
	int maxFarms=-1;
	double kernelRadius=-1;
	double cellSide=0;
	std::string cellFile = pv[20]; // Grid cells filename
	std::vector<double> gridParamsDensity = stringToNumVec(pv[21]); // Maximum farms per cell - split into ints
	if (gridParamsDensity.size()!=0){
		maxFarms = gridParamsDensity[0];
		kernelRadius = gridParamsDensity[1];
	}
	str_cast(pv[22],cellSide); // Length of side for uniform cells	
	// pv[23] ... pv[28]
	std::string outCellFile = pv[29]; // Filename to write cells to
	
	// Shipment-related settings
	std::vector<int> coShipMethods = stringToIntVec(pv[30]); // Methods to use to ship county-county
	std::vector<int> coShipTimes = stringToIntVec(pv[31]); // Times to start using the above methods
		coShipTimes.emplace_back(timesteps+1); // add this b/c whichElement function needs a maximum (element won't be used)

	int farmFarmMethod; str_cast(pv[32],farmFarmMethod); // Method to assign shipments to premises
	// pv[33] ... pv[37]
	std::string outShipFile = pv[38]; // Filename to write shipments (with flagged bans) to
	int outShipRes; str_cast(pv[39], outShipRes); // Output shipments at premises (0), county (1), or state (2) level
	
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
	std::string outStatusFile = pv[48];
	std::vector<std::string> outStatuses = stringToStringVec(pv[49]);
	
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
		Grid_manager G(pfile,switchXY,speciesOnPrems,spSus,spInf,pairwiseOn);
		
		// for instantiating Shipment manager: FIPS of loaded farms and populations of each species/type
		std::unordered_map<std::string, std::vector<Farm*>> fipsmap = G.get_FIPSmap();
		std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Farm*> >> fipsSpeciesMap = G.get_fipsSpeciesMap();
		// get full list of farms
		std::unordered_map<int, Farm*> allFarms = G.get_allFarms(); 

		std::clock_t loading_end = std::clock();
	
 		if(verbose>0){
 			std::cout << std::endl << "CPU time for loading premises: "
 			<< 1000.0 * (loading_end - loading_start) / CLOCKS_PER_SEC
 			<< "ms." << std::endl;
 		}
 		
		// initiate grid
	 	std::clock_t grid_start = std::clock();		
	 	// if file provided, use that
	 	if (cellFile!="*"){G.initiateGrid(cellFile);} // reading in/making cells takes ~45 sec
	 	// else use density params
	 	else if (maxFarms>-1 && kernelRadius >-1){G.initiateGrid(maxFarms,kernelRadius);}
	 	// else use uniform params
	 	else if (cellSide!=0){G.initiateGrid(cellSide);}
	 	else {std::cout<<"Error (main.cpp): no gridding parameters suitable for initiation"<<std::endl<<
	 		"Exiting..." << std::endl;
			exit(EXIT_FAILURE);}
	 	
		// to do: if filename provided, turn on bool to print cells
		
 		std::clock_t grid_end = std::clock();
		double gridGenTimeMS = 1000.0 * (grid_end - grid_start) / CLOCKS_PER_SEC;
		std::cout << "CPU time for generating grid: " << gridGenTimeMS << "ms." << std::endl;

// start loop here
for (auto r=1; r<=reps; r++){
		std::clock_t rep_start = std::clock();
		// randomly pick a proportion to be focal farms and print to external file - for testing
//		std::string fname = "seedFarms.txt"; Status.pickInfAndPrint(0.05, allFarms, fname)
		// load initially infected farms and instantiate Status manager
		// note that initial farms are started as infectious (2) rather than exposed (1)
		Status_manager Status(seedfile, oneRandomSeed, lagParams, allFarms, timesteps);	 	
		Shipment_manager Ship(fipsmap, shipParams, speciesOnPrems, fipsSpeciesMap);

 		int t=0;		
 		// set focalFarms as all farms with status inf at time 0
		std::vector<Farm*> focalFarms = Status.premsWithStatus("inf", t);
		// set compFarms as all farms with status sus at time 0
		std::vector<Farm*> compFarms = Status.premsWithStatus("sus", t);
		int numExposed = Status.premsWithStatus("exp", t).size();
		bool potentialTx = (focalFarms.size()>0 && compFarms.size()>0) || (numExposed>0 && compFarms.size()>0);

   	   while (t<timesteps && potentialTx){ // timesteps, stop early if dies out
   	   	 std::clock_t timestep_start = std::clock();		
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
		 if(verbose>0){std::cout << "Starting grid check (local spread): "<<std::endl;}
  		 std::clock_t gridcheck_start = std::clock();	 
		 G.stepThroughCells(focalFarms,compFarms);
		 
  		 std::clock_t gridcheck_end = std::clock();
  		 double gridCheckTimeMS = 1000.0 * (gridcheck_end - gridcheck_start) / CLOCKS_PER_SEC;
 		 if(verbose>0){std::cout << "CPU time for checking grid: " << gridCheckTimeMS << "ms." << std::endl
 		 	<< "Starting shipments: ";		 }
 		 	
 		 // determine infections that will happen from shipments	 
 		 std::clock_t ship_start = std::clock();	  
 		 std::vector<std::string> bannedFIPS = Status.FIPSWithStatus("banActive", t);
 		 // assign county-level shipment method according to time
 		 int cmElement = whichElement(t, coShipTimes); // which time span does t fall into
 		 int countyMethod = coShipMethods[cmElement]; // get matching shipment method
 		 Ship.makeShipments(focalFarms, compFarms, countyMethod, bannedFIPS);
 		 auto fs = Ship.get_farmShipments();
 		 // output shipments as specified	
		if (outShipFile!="*"){
			std::string printString = Ship.formatOutput(outShipRes, t); // t only used if t==1, tells func to print column headings
			printLine(outShipFile, printString);
		}
 		  		 
 		 auto infShips = Ship.get_infFarmShipments(); // need this for later
 		 std::cout << infShips.size()<<" infectious shipments, of which "<<std::endl;
 		 int banCount = 0;
 		 for (auto& shipment:infShips){if(std::get<5>(shipment)==1){banCount++;}}
 		 std::cout << banCount<<" were ban-compliant."<<std::endl;
 		 std::clock_t ship_end = std::clock();
 		 double shipTimeMS = 1000.0 * (ship_end - ship_start) / CLOCKS_PER_SEC;
 		 if(verbose>0){std::cout << "CPU time for shipping: " << shipTimeMS << "ms." << std::endl;}
 		 
 		 // change statuses:
 		 // infections from local spread
 		 std::vector<Farm*> gridInf = G.get_exposedFarms();
 		 for (auto& gi:gridInf){ // record method of exposure for these farms
 		 	std::vector<int> tfm; /// tfm = time, farmID, method
 		 	tfm.emplace_back(t);
 		 	tfm.emplace_back(-1); // source farm ID in local spread currently untracked
 		 	tfm.emplace_back(0); // method 0 = local diffusion
 		 	gi->set_time_exp(tfm);
 		 }

 		 // infections from shipping (exclude ban-compliant shipments)
 		 std::vector<Farm*> shipInf; shipInf.clear();
 		 for (auto& is:infShips){
 		  if (std::get<5>(is)==0){ // if this shipment was not ban-compliant (includes not banned at all)
 		 	int destFarmID = std::get<1>(is); // get destination farm ID
 		 	shipInf.emplace_back(allFarms.at(destFarmID)); // add to list of farms to become exposed
 		 	// record method of exposure
 		 	std::vector<int> tfm; // tfm = time, farmID, method
			tfm.emplace_back(t);
			tfm.emplace_back(std::get<0>(is)); // origin farm ID
 		 	tfm.emplace_back(1); // method 1=shipping
 		 	allFarms.at(destFarmID)->set_time_exp(tfm); // record method of exposure
 		  }
 		 }
 		 
 		 // combine & eliminate duplicates
 		 std::vector<std::vector<Farm*>> toCombine {gridInf, shipInf}; // create a vector of vectors called "toCombine"
 		 std::vector<Farm*> makeExposed = uniqueFrom(toCombine); // eliminate duplicates

 		 // change statuses for these farms
 		 Status.changeTo("exp", makeExposed, t, latencyParams);
		 if(verbose==2){
		 	std::cout<<gridInf.size()<<" farms now exposed from local spread. "<<shipInf.size()
		 	<<" farms now exposed from shipments."<<std::endl;
		 }

		focalFarms = Status.premsWithStatus("inf", t);
		compFarms = Status.premsWithStatus("sus", t);
		numExposed = Status.premsWithStatus("exp", t).size();
		
		// output status as specified	
		if (outStatusFile!="*"){
			for (auto& os:outStatuses){
				std::string printString = Status.formatOutput(os,t,0); // method 0 sums all prems for each timepoint (only enabled option)
				printLine(outStatusFile, printString);	
			}
		}	
		potentialTx = ((focalFarms.size()>0 && compFarms.size()>0) || (numExposed>0 && compFarms.size()>0));
		std::clock_t timestep_end = std::clock();				
 		double timestepTimeMS = 1000.0 * (timestep_end - timestep_start) / CLOCKS_PER_SEC;
		std::cout << "CPU time for timestep "<< timestepTimeMS << "ms." << std::endl;
 		}  	// end "while under time and exposed/infectious and susceptible farms remain"
 		
 		std::clock_t rep_end = std::clock();
 		double repTimeMS = 1000.0 * (rep_end - rep_start) / CLOCKS_PER_SEC;
		std::cout << "CPU time for rep "<<r<<" ("<<t<<" timesteps): " << repTimeMS << "ms." << std::endl;
} // end for loop

return 0;
}

