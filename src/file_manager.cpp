/* file_manager.cpp
Dec 23 2014

Reads configuration file and checks for errors. Holds vector of all input parameters.
*/

#include "file_manager.h"

file_manager::file_manager()
{
	verbose = verboseLevel;
}

file_manager::~file_manager()
{
}

void file_manager::readConfig(std::string& cfile)
// Reads in config file and checks for requirements/inconsistencies
{
	pv.emplace_back("0"); // fills in [0] so that line numbers match up with elements
	// Read in file and store in parameter vector pv (private class variable)
	std::ifstream f(cfile);
	if(f.is_open()){
		while(!f.eof()){ // until end of file
			std::string line;
			std::stringstream line2;
			// First get a whole line from the config file
			std::getline(f, line);
			// Put that in a stringstream (required for getline) and use getline again
			// to only read up until the comment character (delimiter).
			line2 << line;
			std::getline(line2, line, '#');
			// If there is whitespace between the value of interest and the comment character
			// this will be read as part of the value. Therefore strip it of whitespace first.
			line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
			if(line.size() != 0){pv.emplace_back(line);}
		}
		if (pv.size()!=61){std::cout<<"Warning: expected configuration file with 60 lines, loaded file with "<<pv.size()-1<<" lines."<<std::endl;
		} else {
		std::cout << "Configuration file loaded with "<<pv.size()-1<<" lines."<<std::endl;}

		// Check for consistencies/requirements in parameter vectors, spit out warnings/errors and exit if needed
		bool exitflag = 0;
		bool checkExit;
		
		// Batch name
		params.batch = pv[1];
		// Outputs on/off
		if (pv[2]=="*" || pv[3]=="*"|| pv[4]=="*"|| pv[5]=="*"|| pv[6]=="*"){
			std::cout << "Warning: (config 1-5): Output should be specified - setting unspecified (*) to off." << std::endl;
			if (pv[1]=="*"){pv[2]="0";}
			if (pv[2]=="*"){pv[3]="0";}
			if (pv[3]=="*"){pv[4]="0";}
			if (pv[4]=="*"){pv[5]="0";}
			if (pv[5]=="*"){pv[6]="0";}
		}
		params.printSummary = stringToNum<int>(pv[1]);
		params.printDetail = stringToNum<int>(pv[2]);
		params.printCells = stringToNum<int>(pv[3]);
		params.printShipments = stringToNum<int>(pv[4]);
		params.printControl = stringToNum<int>(pv[5]);
		// pv[6] ... pv[10]
		// Premises file
		if (pv[11]=="*"){
			std::cout << "ERROR (config 11): No premises file specified." << std::endl; exitflag=1;}
		params.premFile = pv[11];
		// Species
		if (pv[12]=="*"){
			std::cout << "ERROR (config 12): No species list provided." << std::endl; exitflag=1;}
		params.species = stringToStringVec(pv[12]);
		// Timesteps
		params.timesteps = stringToNum<int>(pv[13]);
		if (params.timesteps<1){std::cout << "Warning (config 13): Number of timesteps must be 1 or more. Setting number of timesteps to 365." << std::endl; 
			params.timesteps = 365;}
		// Replicates
		params.replicates = stringToNum<int>(pv[14]);
		if (params.replicates<1){std::cout << "Warning (config 14): Number of replications must be 1 or more. Setting number of replications to 1." << std::endl; 
			params.replicates = 1;}
		// Verbose level
		params.verboseLevel = stringToNum<int>(pv[15]);
		if (params.verboseLevel!=0 && params.verboseLevel!=1 && params.verboseLevel!=2){
			std::cout << "Warning (config 15): Verbose option must be 0, 1 or 2. Setting option to off." << std::endl; 
			params.verboseLevel = 0;}
		// Pairwise on
		params.pairwiseOn = stringToNum<int>(pv[16]) ;
		if (params.pairwiseOn!=0 && params.pairwiseOn!=1){
			std::cout << "ERROR (config 16): Pairwise algorithm must be 1 (on) or 0 (off)." << std::endl; exitflag=1;}
		// Reverse x/y
		params.reverseXY = stringToNum<int>(pv[17]);
		if (params.reverseXY!=0 && params.reverseXY!=1){
			std::cout << "ERROR (config 17): Reversing x/y must be 0 (y first) or 1 (x first)." << std::endl; exitflag=1;}
		// pv[18] ... pv[20]
		// Infectious seed files
		if (pv[21]=="*" && pv[22]=="*"){
			std::cout << "ERROR (config 21-22): No infectious premises/county seed file specified." << std::endl; exitflag=1;}
		params.seedPremFile = pv[21];
		params.seedCountyFile = pv[22];
		// Infection seed method
		params.seedMethod = stringToNum<int>(pv[23]);
		// Susceptibility exponents by species
		params.susExponents = stringToNumVec(pv[24]);
		checkExit = checkPositive(params.susExponents, 24); if (checkExit==1){exitflag=1;}
		if (params.susExponents.size() != params.species.size()){
			std::cout<<"ERROR (config 12 & 24): Different numbers of species and susceptibility exponents provided: "<<params.species.size()<<" species and "
			<<params.susExponents.size()<<" exponents." <<std::endl; exitflag=1;}
		// Infectiousness exponents by species
		params.infExponents = stringToNumVec(pv[25]);
		checkExit = checkPositive(params.infExponents, 25); if (checkExit==1){exitflag=1;}
		if (params.infExponents.size() != params.species.size()){
			std::cout<<"ERROR (config 25): Different numbers of species and infectiousness exponents provided." <<std::endl; exitflag=1;}
		// pv[26] ... pv[27]
		// Kernel type & parameters
		if (pv[28]=="*"){std::cout << "ERROR (config 28): No diffusion kernel type specified." << std::endl; exitflag=1;}
		params.kernelType = stringToNum<int>(pv[28]);
		params.kernelParams = stringToNumVec(pv[29]);
		if (params.kernelParams.size() < 3){
			std::cout<<"ERROR (config 29): Need three kernel parameters." <<std::endl; exitflag=1;}
		// Latency parameters
		checkExit = checkMeanVar(pv[30],30,"latency"); if (checkExit==1){exitflag=1;} // if exit triggered by this check, set exitflag=1
		std::vector<double> tempVec = stringToNumVec(pv[30]);
		params.latencyParams = std::make_tuple(tempVec[0],tempVec[1]);
		// Infectiousness parameters
		checkExit = checkMeanVar(pv[31],31,"infectiousness"); if (checkExit==1){exitflag=1;} // if exit triggered by this check, set exitflag=1	
		tempVec = stringToNumVec(pv[31]);
		params.infectiousParams = std::make_tuple(tempVec[0],tempVec[1]);
		//pv[32] ... pv[35]
		// Grid settings
		if (pv[36]=="*" && pv[37]=="*" && pv[38]=="*"){std::cout << "ERROR (config 36-38): No grid cell parameters specified." << std::endl; exitflag=1;}
		params.cellFile = pv[36];
		params.uniformSide = stringToNum<double>(pv[37]);
		params.densityParams = stringToIntVec(pv[38]);
			checkExit = checkPositive(params.densityParams, 38); if (checkExit==1){exitflag=1;}
			if ((params.densityParams).size()!=2){std::cout << "ERROR (config 38): Two parameters required for grid creation by density." << std::endl; exitflag=1;}		
		//pv[39] ... pv[40]
		// Shipping methods and times
		if (pv[41]=="*"){std::cout << "ERROR (config 41): No county-level shipment method(s) specified." << std::endl; exitflag=1;}
		params.shipMethods = stringToIntVec(pv[41]);
		if (pv[42]=="*"){std::cout << "ERROR (config 42): No shipment method start time(s) specified." << std::endl; exitflag=1;}
		params.shipMethodTimeStarts = stringToIntVec(pv[42]);		
		if (params.shipMethods.size() != params.shipMethodTimeStarts.size()){
			std::cout << "ERROR (config 41-42): Number of methods and start times provided must be the same ("<<params.shipMethods.size()<<" method(s) and "<<params.shipMethodTimeStarts.size()<<" start time(s) provided)."<<std::endl; exitflag=1;}
		if (params.shipMethodTimeStarts.at(0) != 1){std::cout << "ERROR (config 42): First county-level shipment method start time must be 1."<<std::endl; exitflag=1;}
		  // check that each value is between 1 and number of timesteps
		bool rangeflag = 0;
		for (auto& t:params.shipMethodTimeStarts){if(t<1 || t>params.timesteps){rangeflag=1;}}
		if (rangeflag){std::cout << "Warning (config 42): Simulation timespan (config 13) is shorter than largest timepoint for county-level shipment methods - some methods may not be used." << std::endl;}
		params.shipMethodTimeStarts.emplace_back(params.timesteps+1); // add this b/c whichElement function needs a maximum (element won't be used)
		if (pv[43]=="*"){std::cout << "ERROR (config 43): No premises-level shipment assignment method specified." << std::endl; exitflag=1;}
		params.shipPremAssignment = stringToNum<int>(pv[43]);
		//pv[44] ... pv[50]
		// Reporting lags - index
		checkExit = checkMeanVar(pv[51],51,"index reporting"); if (checkExit==1){exitflag=1;}
		tempVec = stringToNumVec(pv[51]);
		params.indexReportLag = std::make_tuple(tempVec[0],tempVec[1]);
		// Reporting lags - normal
		checkExit = checkMeanVar(pv[52],52,"reporting"); if (checkExit==1){exitflag=1;}
		tempVec = stringToNumVec(pv[52]);
		params.indexReportLag = std::make_tuple(tempVec[0],tempVec[1]);
		// Control - shipping bans
		params.shipBanCompliance = stringToNum<double>(pv[53]);
		if (params.shipBanCompliance<0 || params.shipBanCompliance>100){
			std::cout << "ERROR (config 53): Shipment ban compliance must be specified as percentage." << std::endl; exitflag=1;}
		params.banLevel = stringToNum<int>(pv[54]);
		if (params.banLevel!=0 && params.banLevel!=1){std::cout << "ERROR (config 54): Shipment ban scale must be county-(0) or state-(1) level." << std::endl;  exitflag=1;}
		// Ban lag - initiation
		checkExit = checkMeanVar(pv[55],55,"ban initiation"); if (checkExit==1){exitflag=1;}
		tempVec = stringToNumVec(pv[55]);
		params.reportToOrderBan = std::make_tuple(tempVec[0],tempVec[1]);
		// Ban lag - compliance
		checkExit = checkMeanVar(pv[56],56,"ban compliance"); if (checkExit==1){exitflag=1;}
		tempVec = stringToNumVec(pv[56]);
		params.reportToOrderBan = std::make_tuple(tempVec[0],tempVec[1]);
		//pv[57] ... pv[60]
	
		if (exitflag){
			std::cout << "Exiting..." << std::endl;
			exit(EXIT_FAILURE);
		}	
		
		// Put together infection lag parameters
		params.lagParams["latency"] = params.latencyParams; 		//exposed-infectious
		params.lagParams["infectious"] = params.infectiousParams; 	//infectious-recovered
		
		// Put together control-related lag parameters
		params.controlLags["indexReport"] = params.indexReportLag; 	//first case exposed-reported
		params.controlLags["report"] = params.reportLag; 				//exposed-reported
		params.controlLags["startBan"] = params.reportToOrderBan; 	//reported-banned
		params.controlLags["complyBan"] = params.orderToCompliance; 	//banned-compliant	
		
		
	} else { // if file not found
		std::cout << "ERROR: Configuration file not found: " << cfile << std::endl <<
		"Exiting..." << std::endl;
		exit(EXIT_FAILURE);
	}
}

bool file_manager::checkMeanVar(std::string& s, int lineNum, std::string paramDesc)
// use with any argument meant to be in the form (mean,variance)
// checks that string s contains two positive arguments
// if only one argument is provided, that will be used as the mean (with warning)
// outputs error information using line number (lineNum) and parameter description (paramDesc)
// returns exitFlag true/false
{
	bool exitflag = 0;
	std::vector<double> tempVec;
	tempVec = stringToNumVec(s);
		if (tempVec.size()==0){std::cout << "ERROR (config "<<lineNum<<"): No "<<paramDesc<<" parameters specified." << std::endl; exitflag=1;
		} else if (tempVec.size()==1){std::cout << "Warning (config "<<lineNum<<"): Only one "<<paramDesc<<" parameter provided, mean will be set to "<<tempVec[0]<<" and variance to 0."<<std::endl;
			s.append(",0");
		} else if (tempVec.size()>2){std::cout << "ERROR (config "<<lineNum<<"): Only 2 "<<paramDesc<<" parameters (mean,variance) permitted, "<<tempVec.size()<<" were provided." << std::endl; exitflag=1;}

		bool checkPos = checkPositive(tempVec, lineNum); if (checkPos==1){exitflag=1;}
	return exitflag;
}

bool file_manager::checkPositive(std::vector<int>& tempVec, int lineNum)
{
	bool exitflag = 0;
	unsigned int it = 0;
	while (it < tempVec.size() && exitflag ==0){
		auto tv = tempVec[it];
		if(tv<0){
			std::cout << "ERROR (config "<<lineNum<<"): All parameters must be positive. " << std::endl;
			exitflag=1;
		}
		it++;
	}
	return exitflag;
}

// overloaded for doubles
bool file_manager::checkPositive(std::vector<double>& tempVec, int lineNum)
{
	bool exitflag = 0;
	unsigned int it = 0;
	while (it < tempVec.size() && exitflag ==0){
		auto tv = tempVec[it];
		if(tv<0){
			std::cout << "ERROR (config "<<lineNum<<"): All parameters must be positive. " << std::endl;
			exitflag=1;
		}
		it++;
	}
	return exitflag;
}