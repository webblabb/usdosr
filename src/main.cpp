#include <Rcpp.h>

#include <iostream>
#include <ctime>
#include <stdlib.h>

#include "file_manager.h"
#include "Grid_manager.h"
#include "Grid_checker.h"
#include "Shipment_manager.h"
#include "County.h"

int verboseLevel; // global variable determining console output

// [[Rcpp::plugins(cpp11)]]

//' Runs USDOS model for a given config file.
//' 
//' @param cfile the name of the config file to use
//' @return writes output to file
// [[Rcpp::export]]
int run_usdos(std::string cfile)
{
  std::clock_t process_start = std::clock();
	
	std::ifstream f(cfile);
	if(!f.is_open()){
	  Rcpp::stop("Config file does not exist.");
	} 
	
	file_manager fm; // construct file_manager object
	fm.readConfig(cfile); // reads config file, creates parameters object, and checks for errors
	const Parameters* p = fm.getParams();
	// make string: batch_date_time
	std::string batchDateTime = p->batch;
	// get/format current time
	time_t rawtime;
	struct tm* timeinfo;
	char buffer[80];
	time (&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer,80,"_%Y%b%d_%H%M",timeinfo);
	std::string str(buffer);
	
	batchDateTime += str;
	// write parameters from config file to settings_batchname
	std::string settingsOutFile = "runlog.txt";
	// columns are batchDateTime, config lines 1-70, tab-separated
	std::string printString = fm.getSettings(batchDateTime);
	printLine(settingsOutFile,printString);

	// set values for global,
	verboseLevel = p->verboseLevel;

	int verbose = verboseLevel; // override global value for main here if desired
	int timesteps = p->timesteps;
	
	// Read in farms, determine xylimits
	std::clock_t loading_start = std::clock();
	Grid_manager G(p);
	// get pointers to full list of farms & cells
	auto allPrems = G.get_allFarms();
	auto fipsmap = G.get_FIPSmap();
	auto allCells = G.get_allCells(); // will be filled when grid is initiated, for now pointer just exists
 	auto fipsSpeciesMap = G.get_fipsSpeciesMap(); // for shipments
	std::clock_t loading_end = std::clock();

	if(verbose>0){
		std::cout << std::endl << "CPU time for loading premises: "
		<< 1000.0 * (loading_end - loading_start) / CLOCKS_PER_SEC
		<< "ms." << std::endl;
	}

	// Initiate grid...
	std::clock_t grid_start = std::clock();
	// if cell file provided, use that
	if (p->cellFile!="*"){
		std::string cellFile = p->cellFile;
		G.initiateGrid(cellFile);} // reading in 730 cells takes ~45 sec
	// else use uniform params
	else if (p->uniformSide>0){
		G.initiateGrid(p->uniformSide);}
	// else use density params
	else {
		G.initiateGrid(p->densityParams.at(0), // max prems per cell
					   p->densityParams.at(1)); // min cell side}
	}

	std::clock_t grid_end = std::clock();
	double gridGenTimeMS = 1000.0 * (grid_end - grid_start) / CLOCKS_PER_SEC;
	if (verbose > 0){
	  std::cout << "CPU time for generating grid: " << gridGenTimeMS << "ms." << std::endl;
	}

	// Get initially infected (seed) premises 
	std::vector<std::vector<Farm*>> seedFarmsByRun;
	G.get_seedPremises(seedFarmsByRun); // saves to seedFarmsByRun
	
	if (seedFarmsByRun.size() == 0 ){
	  std::cout << "ERROR: No valid seed farms provided/located. Exiting... ";
	  exit(EXIT_FAILURE);
	}

    //~~~~~~~~~~~~~~~~~~ Loop starts here
    for (int r=1; r<=seedFarmsByRun.size(); r++){ 
    	std::vector<Farm*> seedFarms = seedFarmsByRun[r];
    	
			std::clock_t rep_start = std::clock();
			// load initially infected farms and instantiate Status manager
			// note that initial farms are started as exposed

			Control_actions Control(p);

			Status_manager Status(seedFarms, p, &G, &Control); // seeds initial exposures, modify to pass grid manager, p
			Shipment_manager Ship(fipsmap, fipsSpeciesMap, &Status, p->shipPremAssignment, p->species, p); // modify to pass grid manager, p
			Grid_checker gridCheck(allCells, Status.get_sources(),p->kernel);

			int t=0;
			int numSuscept, numExposed;
			std::vector<Farm*> focalFarms;
			bool potentialTx = 1;

      while (t<timesteps && potentialTx){ // timesteps, stop early if dies out
            std::clock_t timestep_start = std::clock();

            ++t; // starts at 1, ends at timesteps
            Status.updates(t); // disease updates: when applicable, exposed -> infectious, infectious -> immune
    if(verbose>1){std::cout<<std::endl<<"Time "<<t<<std::endl<<"Disease statuses updated."<<std::endl;}
//            Control.updates(t); // control updates: when applicable, exposed -> reported, reported -> banned, banned -> compliant
//    if(verbose>1){std::cout<<"Control statuses updated."<<std::endl;}
            Status.premsWithStatus("inf", focalFarms);	// set focalFarms as all farms with disease status inf
            if(verbose > 0){
              std::cout << std::endl<<std::endl<<"Timestep "<<t<<": "
              <<Status.numPremsWithStatus("sus")<<" susceptible, "
              <<Status.numPremsWithStatus("exp")<<" exposed, "
              <<focalFarms.size()<<" infectious, "
              <<Status.numPremsWithStatus("imm")<<" immune premises. "<<std::endl;
            }

			// output Control statuses to console
			
             // determine infections that will happen from local diffusion

    if(verbose>0){std::cout << "Starting grid check (local spread): "<<std::endl;}
             std::clock_t gridcheck_start = std::clock();

             std::vector<Farm*> notSus;
             Status.newNotSus(notSus); // gets newly not-susceptible farms
             gridCheck.stepThroughCells(focalFarms,notSus);

             std::vector<Farm*> gridInf;
             gridInf.reserve(allPrems->size());
             gridCheck.take_exposed(gridInf); // simultaneously takes values and clears in gridCheck, resetting with reserve of number of all prems

             std::clock_t gridcheck_end = std::clock();
             double gridCheckTimeMS = 1000.0 * (gridcheck_end - gridcheck_start) / CLOCKS_PER_SEC;
    if(verbose>0){std::cout << "Total local infections: " << gridInf.size() << std::endl;
        std::cout << "CPU time for checking grid: " << gridCheckTimeMS << "ms." << std::endl;}

            // determine shipments
            std::clock_t ship_start = std::clock();
            // assign county-level shipment method according to time
            auto timeVec = p->shipMethodTimeStarts;
            int cmElement = whichElement(t, timeVec); // which time span does t fall into
            int countyMethod = (p->shipMethods).at(cmElement); // get matching shipment method
            std::vector<Shipment*> fs; // fs = farm shipments
            Ship.makeShipments(focalFarms, countyMethod, fs);
            std::clock_t ship_end = std::clock();
            double shipTimeMS = 1000.0 * (ship_end - ship_start) / CLOCKS_PER_SEC;
            if(verbose>0){std::cout << "CPU time for shipping: " << shipTimeMS << "ms." << std::endl;}

            // change statuses for these farms (checks for duplicates)
            if (gridInf.size()>0){
            	Status.localExposure(gridInf,t);
            }
            
            if (fs.size()>0){
            	Status.shipExposure(fs,t);
            } // send farm shipments to be checked, begin exposure where appropriate

            // at the end of this transmission day, statuses are now...
            Status.premsWithStatus("inf", focalFarms); // assign "inf" farms as focalFarms
            numSuscept = Status.numPremsWithStatus("sus");
            numExposed = Status.numPremsWithStatus("exp");

            // write output for details of exposures from this rep, t
            if (p->printDetail > 0){
                // output detail to file
                // rep, ID, time, sourceID, method
                std::string detOutFile = batchDateTime;
                detOutFile += "_detail.txt";
                // specify seed farm/county?
                if (r==1 && t==1){
                    std::string header = "Rep\tExposedID\tatTime\tSourceID\tInfRoute\n";
                    printLine(detOutFile,header);
                }
                std::string printString = Status.formatDetails(r,t);
                printLine(detOutFile, printString);
            }

            potentialTx = ((focalFarms.size()>0 && numSuscept>0) || (numExposed>0 && numSuscept>0));

            std::clock_t timestep_end = std::clock();
            if(verbose > 0){
              double timestepTimeMS = 1000.0 * (timestep_end - timestep_start) / CLOCKS_PER_SEC;
              std::cout << "CPU time for timestep "<< timestepTimeMS << "ms." << std::endl;
            }
        }  	// end "while under time and exposed/infectious and susceptible farms remain"

        std::clock_t rep_end = std::clock();
        double repTimeMS = 1000.0 * (rep_end - rep_start) / CLOCKS_PER_SEC;
        
        if(verbose > 0){
          std::cout << "CPU time for rep "<<r<<" ("<<t<<" timesteps): " << repTimeMS << "ms." << std::endl;
        }

        if (p->printSummary > 0){
            // output summary to file (rep, days inf, run time)
            // rep, # farms infected, # days of infection, seed farm and county, run time
            std::string sumOutFile = batchDateTime;
            sumOutFile += "_summary.txt";
            if (r==1){
                std::string header = "Rep\tNum_Inf\tDuration\tSeed_Farms\tSeed_FIPS\tRunTimeSec\n";
                printLine(sumOutFile,header);
            }
            std::string repOut = Status.formatRepSummary(r,t,repTimeMS);
            printLine(sumOutFile,repOut);

        }

    } // end for loop

    //Old loop below. I've left this here temporary to see that everything works after resolving
    //conflicts while merging shipment code into master. /Stefan.
    //~~~~~~~~~~~~~~~~~~ Loop starts here
    /*
    for (int r=1; r<=reps; r++){
        std::clock_t rep_start = std::clock();
        // load initially infected farms and instantiate Status manager
        // note that initial farms are started as infectious rather than exposed

        auto cParams = p->controlLags;
        Control_actions Control(cParams);

        if (p->seedMethod < 0){ // if choosing seeds by county, choose county based on rep number
            seedFarms = fipsmap->at(FIPSlist.at(r))->get_farms();
        }
        int seedType = p->seedMethod;
        auto lagP = p->lagParams;

        Status_manager Status(seedFarms, seedType, lagP, allPrems, timesteps, &Control); // seeds initial exposures
        Shipment_manager Ship(fipsmap, fipsSpeciesMap, p->shipPremAssignment, species);
        Grid_checker gridCheck(allCells, Status.get_sources(),kernelP);

        int t=0;
        int numSuscept, numExposed;
        std::vector<Farm*> focalFarms;
        bool potentialTx = 1;

       while (t<timesteps && potentialTx){ // timesteps, stop early if dies out
            std::clock_t timestep_start = std::clock();

            ++t; // starts at 1, ends at timesteps
            Status.updates(t); // disease updates: when applicable, exposed -> infectious, infectious -> immune
    if(verbose>1){std::cout<<std::endl<<"Time "<<t<<std::endl<<"Disease statuses updated."<<std::endl;}
            Control.updates(t); // control updates: when applicable, exposed -> reported, reported -> banned, banned -> compliant
    if(verbose>1){std::cout<<"Control statuses updated."<<std::endl;}
            Status.premsWithStatus("inf", focalFarms);	// set focalFarms as all farms with disease status inf

            std::cout << std::endl<<std::endl<<"Timestep "<<t<<": "
            <<Status.numPremsWithStatus("sus")<<" susceptible, "
            <<Status.numPremsWithStatus("exp")<<" exposed, "
            <<focalFarms.size()<<" infectious, "
            <<Status.numPremsWithStatus("imm")<<" immune premises. "<<std::endl

            <<Control.getNcounties("report", 1)<<" FIPS reported. " // reported counties have "reported" status = 1
            <<Control.getNcounties("shipBan", 1)<<" FIPS with ban ordered. " // level 1 = ordered, not yet compliant
            <<Control.getNcounties("shipBan", 2)<<" FIPS with active shipping ban. "// level 2 = ordered and compliant
            <<std::endl;

             // determine infections that will happen from local diffusion

    if(verbose>0){std::cout << "Starting grid check (local spread): "<<std::endl;}
             std::clock_t gridcheck_start = std::clock();
             std::vector<Farm*> notSus;
             Status.take_notSus(notSus); // simultaneously takes values and clears vector in Status
             //gridCheck.stepThroughCells(focalFarms,notSus);
             std::vector<Farm*> gridInf;
             gridInf.reserve(840000);
             gridCheck.take_exposed(gridInf); // simultaneously takes values and clears in gridCheck, resetting with reserve of 840k

             std::clock_t gridcheck_end = std::clock();
             double gridCheckTimeMS = 1000.0 * (gridcheck_end - gridcheck_start) / CLOCKS_PER_SEC;
    if(verbose>0){std::cout << "Total grid infections: " << gridInf.size() << std::endl;
        std::cout << "CPU time for checking grid: " << gridCheckTimeMS << "ms." << std::endl;}

             // determine shipments
             std::clock_t ship_start = std::clock();
             // assign county-level shipment method according to time
             auto timeVec = p->shipMethodTimeStarts;
             int cmElement = whichElement(t, timeVec); // which time span does t fall into
             int countyMethod = (p->shipMethods).at(cmElement); // get matching shipment method
             ; // will be filled with farm-farm shipments
             std::vector<Shipment*> fs = Ship.makeShipments(focalFarms, countyMethod);
             // output shipments as specified
    // 		if (outShipFile!="*"){
    // 			std::string printString = Ship.formatOutput(outShipRes, t); // t only used if t==1, tells func to print column headings
    // 			printLine(outShipFile, printString);
    // 		}

    //  		 auto infShips = Ship.get_infFarmShipments(); // need this for later
    //  		 std::cout << infShips.size()<<" infectious shipments, of which "<<std::endl;
    //  		 int banCount = 0;
    //  		 for (auto& shipment:infShips){if(std::get<5>(shipment)==1){banCount++;}}
    //  		 std::cout << banCount<<" were ban-compliant."<<std::endl;
            std::clock_t ship_end = std::clock();
            double shipTimeMS = 1000.0 * (ship_end - ship_start) / CLOCKS_PER_SEC;
            if(verbose>0){std::cout << "CPU time for shipping: " << shipTimeMS << "ms." << std::endl;}

            // change statuses for these farms (checks for duplicates)
            if (gridInf.size()>0){Status.localExposure(gridInf,t);}
            if (fs.size()>0){Status.shipExposure(fs,t);} // send farm shipments to be checked, begin exposure where appropriate

            // at the end of this transmission day, statuses are now...
            Status.premsWithStatus("inf", focalFarms); // assign "inf" farms as focalFarms
            numSuscept = Status.numPremsWithStatus("sus");
            numExposed = Status.numPremsWithStatus("exp");

            // write output for details of exposures from this rep, t
            if (p->printDetail > 0){
                // output detail to file
                // rep, ID, time, sourceID, method
                std::string detOutFile = p->batch;
                detOutFile += "_detail.txt";
                // specify seed farm/county?
                if (r==1 && t==1){
                    std::string header = "Rep\tExposedID\tatTime\tSourceID\tInfRoute\n";
                    printLine(detOutFile,header);
                }
                std::string printString = Status.formatDetails(r,t);
                printLine(detOutFile, printString);
            }

            potentialTx = ((focalFarms.size()>0 && numSuscept>0) || (numExposed>0 && numSuscept>0));

            std::clock_t timestep_end = std::clock();
            double timestepTimeMS = 1000.0 * (timestep_end - timestep_start) / CLOCKS_PER_SEC;
            std::cout << "CPU time for timestep "<< timestepTimeMS << "ms." << std::endl;
        }  	// end "while under time and exposed/infectious and susceptible farms remain"

        std::clock_t rep_end = std::clock();
        double repTimeMS = 1000.0 * (rep_end - rep_start) / CLOCKS_PER_SEC;
        std::cout << "CPU time for rep "<<r<<" ("<<t<<" timesteps): " << repTimeMS << "ms." << std::endl;

        if (p->printSummary > 0){
            // output summary to file (rep, days inf, run time)
            // rep, # farms infected, # days of infection, seed farm and county, run time
            std::string sumOutFile = batchDateTime;
            sumOutFile += "_summary.txt";
            if (r==1){
                std::string header = "Rep\tNum_Inf\tDuration\tSeed_Farms\tSeed_FIPS\tRunTimeSec\n";
                printLine(sumOutFile,header);
            }
            std::string repOut = Status.formatRepSummary(r,t,repTimeMS);
            printLine(sumOutFile,repOut);

        }

    } // end for loop
    */

    std::clock_t process_end = std::clock();
    double process_time = 1000.0 * (process_end - process_start) / CLOCKS_PER_SEC;
    std::cout << "Entire process was alive for " << process_time << "ms." << std::endl;

    return 0;
} // end main()
