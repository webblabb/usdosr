#include <Rcpp.h>

#include "file_manager.h"

file_manager::file_manager()
{
	verbose = verboseLevel;
}

file_manager::~file_manager()
{
	delete params.kernel;
}

/// Returns a string to output to run log file, containing batchDateTime (provided as argument), and contents of pv (config lines 1-70)
const std::string file_manager::getSettings(std::string& bdt)
{
	std::string output = bdt;
	for (int i=1; i<=70; i++){
		output += "\t";
		output += pv[i];
	}
	output += "\n";
	return output;
}

/// Reads configuration file, stores parameter values in a character vector.
/// Checks validity of values and groups closely related parameters.
void file_manager::readConfig(std::string& cfile)
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
		if (pv.size()!=71){std::cout<<"Warning: expected configuration file with 70 lines, loaded file with "<<pv.size()-1<<" lines."<<std::endl;
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
			if (pv[2]=="*"){pv[2]="0";}
			if (pv[3]=="*"){pv[3]="0";}
			if (pv[4]=="*"){pv[4]="0";}
			if (pv[5]=="*"){pv[5]="0";}
			if (pv[6]=="*"){pv[6]="0";}
		}
		params.printSummary = stringToNum<int>(pv[2]);
		params.printDetail = stringToNum<int>(pv[3]);
		params.printCells = stringToNum<int>(pv[4]);
		params.printShipments = stringToNum<int>(pv[5]);
		params.printControl = stringToNum<int>(pv[6]);
		// pv[7] ... pv[10]
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
		// pv[14] unused
		// Verbose level
		params.verboseLevel = stringToNum<int>(pv[15]);
		if (params.verboseLevel!=0 && params.verboseLevel!=1 && params.verboseLevel!=2){
			std::cout << "Warning (config 15): Verbose option must be 0, 1 or 2. Setting option to off." << std::endl;
			params.verboseLevel = 0;}
		verbose = params.verboseLevel;
		// Pairwise on
		params.pairwiseOn = stringToNum<int>(pv[16]) ;
		if (params.pairwiseOn!=0 && params.pairwiseOn!=1){
			std::cout << "ERROR (config 16): Pairwise algorithm must be 1 (on) or 0 (off)." << std::endl; exitflag=1;}
		// Reverse x/y
		params.reverseXY = stringToNum<int>(pv[17]);
		if (params.reverseXY!=0 && params.reverseXY!=1){
			std::cout << "ERROR (config 17): Reversing x/y must be 0 (y first) or 1 (x first)." << std::endl; exitflag=1;}
		//County data file
		if (pv[18] == "*"){
            std::cout << "ERROR (config 18): No county data file specified." << std::endl; exitflag=1;}
		params.fipsFile = pv[18];
		if (pv[19] == "*"){
            std::cout << "ERROR (config 19): No fips shipping weights file specified." << std::endl; exitflag=1;}
		params.fips_weights = pv[19];

        //pv[20];
		// Infectious seed file
		if (pv[21]=="*"){
			std::cout << "ERROR (config 21): No infectious premises seed source specified." << std::endl; exitflag=1;}
		params.seedSource = pv[21];
		// Infection seed method
		params.seedSourceType = pv[22];
		if (params.seedSourceType.compare("fips") == 0 && 
				params.seedSourceType.compare("singlePremises") == 0 &&
				params.seedSourceType.compare("multiplePremises") == 0){
		std::cout << "ERROR (config 22): Seed source type must be 'fips','singlePremises', or 'multiplePremises'." << std::endl; exitflag=1;}
		// pv[23] unused
		// Susceptibility exponents by species
		std::vector<double> tempVec1 = stringToNumVec(pv[24]);
		checkExit = checkPositive(tempVec1, 24); if (checkExit==1){exitflag=1;}
		if (tempVec1.size() != params.species.size()){
			std::cout<<"ERROR (config 12 & 24): Different numbers of species and susceptibility exponents provided: "<<params.species.size()<<" species and "
			<<tempVec1.size()<<" exponents." <<std::endl; exitflag=1;}
		// Infectiousness exponents by species
		std::vector<double> tempVec2 = stringToNumVec(pv[25]);
		checkExit = checkPositive(tempVec2, 25); if (checkExit==1){exitflag=1;}
		if (tempVec2.size() != params.species.size()){
			std::cout<<"ERROR (config 25): Different numbers of species and infectiousness exponents provided." <<std::endl; exitflag=1;}
		// Susceptibility constants by species
		std::vector<double> tempVec3 = stringToNumVec(pv[26]);
		checkExit = checkPositive(tempVec3, 26); if (checkExit==1){exitflag=1;}
		if (tempVec3.size() != params.species.size()){
			std::cout<<"ERROR (config 12 & 26): Different numbers of species and susceptibility constants provided: "<<params.species.size()<<" species and "
			<<tempVec3.size()<<" constants." <<std::endl; exitflag=1;}
		// Infectiousness constants by species
		std::vector<double> tempVec4 = stringToNumVec(pv[27]);
		checkExit = checkPositive(tempVec4, 27); if (checkExit==1){exitflag=1;}
		if (tempVec4.size() != params.species.size()){
			std::cout<<"ERROR (config 12 & 27): Different numbers of species and infectiousness constants provided: "<<params.species.size()<<" species and "
			<<tempVec4.size()<<" constants." <<std::endl; exitflag=1;}
		// Map exponents and constant values to species
		int i = 0;
		for (auto &sp:params.species){
			params.susExponents[sp] = tempVec1.at(i);
			params.infExponents[sp] = tempVec2.at(i);
			params.susConsts[sp] = tempVec3.at(i);
			params.infConsts[sp] = tempVec4.at(i);
			++i;
		}
		// Kernel type & parameters
		if (pv[28]!="0" && pv[28]!="1"){std::cout << "ERROR (config 28): Kernel type must be 0 (power law) or 1 (data-based)." << std::endl; exitflag=1;}
		params.kernelType = stringToNum<int>(pv[28]);
		params.kernelParams = stringToNumVec(pv[29]);
		if (params.kernelType == 0 && params.kernelParams.size() < 3){
			std::cout<<"ERROR (config 29): For kernel type 0, three kernel parameters are required." <<std::endl; exitflag=1;}
		// Data-based kernel file (checked against kernel option)
		params.dataKernelFile = pv[30];
		if (params.dataKernelFile!="*" && params.kernelType!=1){std::cout<<"Warning: Data kernel file provided but kernel type is not set to 1; file will be ignored." << std::endl;}
		if (params.kernelType == 1 && params.dataKernelFile=="*"){std::cout<<"ERROR (config 28): Kernel type 1 (data-based) requires file in config 30."<< std::endl; exitflag=1;}
		// Latency parameters
		checkExit = checkMeanVar(pv[31],31,"latency"); if (checkExit==1){exitflag=1;} // if exit triggered by this check, set exitflag=1
		std::vector<double> tempVec = stringToNumVec(pv[31]);
		params.latencyParams = std::make_tuple(tempVec[0],tempVec[1]);
		// Infectiousness parameters
		checkExit = checkMeanVar(pv[32],32,"infectiousness"); if (checkExit==1){exitflag=1;} // if exit triggered by this check, set exitflag=1
		tempVec = stringToNumVec(pv[32]);
		params.infectiousParams = std::make_tuple(tempVec[0],tempVec[1]);
		//pv[33] ... pv[35]
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
		params.orderToCompliance = std::make_tuple(tempVec[0],tempVec[1]);
		//pv[57] ... pv[60]

		if (exitflag){
			std::cout << "Exiting..." << std::endl;
			exit(EXIT_FAILURE);
		}

		// Group shipment ban parameters
		params.controlLags["shipBan"].emplace_back(std::make_tuple(0,0)); // Level/index 0
		params.controlLags["shipBan"].emplace_back(params.reportToOrderBan); // Level/index 1: time from report to order
		params.controlLags["shipBan"].emplace_back(params.orderToCompliance); // Level/index 2: time from order to implementation

		// Construct kernel object
		switch (params.kernelType)
		{
			case 0:{
				params.kernel = new Local_spread(0, params.kernelParams);
				break;
			}
			case 1:{
				params.kernel = new Local_spread(1, params.dataKernelFile);
				break;
			}
		}

if (verbose>0){std::cout<<"Parameter loading complete."<<std::endl;}

	} else { // if file not found
	  
	  Rcpp::stop("Config file does not exist.");

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
