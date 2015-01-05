// file_manager.cpp
// Dec 23 2014

#include "file_manager.h"

file_manager::file_manager()
{
}

file_manager::~file_manager()
{
}

void file_manager::readConfig(std::string& cfile)
// Reads in config file and checks for requirements/inconsistencies
{
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
		if (pv.size()!=60){std::cout<<"Warning: expected configuration file with 60 lines, loaded file with "<<pv.size()<<" lines."<<std::endl;
		} else if (pv[15]=="1"){ // verbose option on
		std::cout << "Configuration file loaded with "<<pv.size()<<" lines."<<std::endl;}

		// Check for consistencies/requirements in parameter vectors, spit out warnings/errors and exit if needed
		bool exitflag = 0;
		// for temporary conversions to numbers where comparisons are necessary:
		int tempInt, timesteps; 
		std::vector<double> tempVec;
		bool checkExit;
		
		str_cast(pv[1],tempInt);
		if (tempInt<1){std::cout << "Warning (config 1): Number of replications must be 1 or more. Setting number of replications to 1." << std::endl; 
			pv[1] = "1";}
		if (pv[2]!="0" && pv[2]!="1"){std::cout << "ERROR (config 2): Gridding algorithm must be 1 (on) or 0 (off)." << std::endl; exitflag=1;}
		if (pv[3]!="0" && pv[3]!="1"){std::cout << "ERROR (config 3): Pairwise algorithm must be 1 (on) or 0 (off)." << std::endl; exitflag=1;}
		if (pv[10]=="*"){std::cout << "ERROR (config 10): No premises file specified." << std::endl; exitflag=1;}
		if (pv[11]=="*" && pv[12]=="*"){std::cout << "ERROR (config 11-12): No infectious premises file specified." << std::endl; exitflag=1;}
		str_cast(pv[13],timesteps);
		if (timesteps<1){std::cout << "Warning (config 13): Number of timesteps must be 1 or more. Setting number of timesteps to 1." << std::endl; 
			pv[13] = "1";}
		if (pv[15]!="0" && pv[15]!="1"){std::cout << "Warning (config 15): Verbose option must be 1 (on) or 0 (off). Setting option to off." << std::endl; 
			pv[15] = "0";}
		if (pv[16]!="0" && pv[16]!="1"){std::cout << "Warning (config 16): X-Y option must be 0 (Y first) or 1 (X first). Setting option to Y-first." << std::endl; 
			pv[16] = "0";}
		if (pv[20]=="*" && pv[21]=="*" && pv[22]=="*"){std::cout << "ERROR (config 20-22): No grid cell parameters specified." << std::endl; exitflag=1;}
		if (pv[20]=="*" && pv[21]!="*"){
			tempVec = stringToNumVec(pv[21]);
			if (tempVec.size()!=2){std::cout << "ERROR (config 21): Two parameters required for grid creation by density." << std::endl; exitflag=1;}
			if (tempVec[0]<1){std::cout << "ERROR (config 21): Maximum farms/cell must be 1 or more."<<std::endl; exitflag=1;}
			if (tempVec[1]<0){std::cout << "ERROR (config 21): Radius of diffusion kernel must be positive."<<std::endl; exitflag=1;}
		}
		if (pv[20]=="*" && pv[21]=="*"){
			str_cast(pv[22],tempInt);
			if (tempInt<1){std::cout << "ERROR (config 22): Cell side length must be 1 or more."<<std::endl; exitflag=1;}
		}
		if (pv[30]=="*"){std::cout << "ERROR (config 30): No county-level shipment method(s) specified." << std::endl; exitflag=1;}
			// split pv[30] to get length
			std::vector<double> pv30 = stringToNumVec(pv[30]);
		if (pv[31]=="*"){std::cout << "ERROR (config 31): No county-level shipment method time(s) specified." << std::endl; exitflag=1;}
			// split pv[31] and check that length is same as pv[30]
			std::vector<double> pv31 = stringToNumVec(pv[31]);
			if (pv30.size() != pv31.size()){std::cout << "ERROR (config 30-31): Number of methods and start times provided must be the same ("<<pv30.size()<<" method(s) and "<<pv31.size()<<" start time(s) provided)."<<std::endl; exitflag=1;}
			 // check that each value is between 1 and number of timesteps
			bool rangeflag = 0;
			for (auto& pv31v:pv31){if(pv31v<1 || pv31v>timesteps){rangeflag=1;}}
			if (rangeflag){std::cout << "Warning (config 31): Simulation timespan (config 13) is shorter than largest timepoint for county-level shipment methods - some methods may not be used." << std::endl;}
			if (pv31[0] != 1){std::cout << "ERROR (config 31): First county-level shipment method start time must be 1."<<std::endl; exitflag=1;}
		if (pv[32]=="*"){std::cout << "ERROR (config 32): No premises-level shipment assignment method specified." << std::endl; exitflag=1;}
		if (pv[38]!="*" && pv[39]=="*"){std::cout << "ERROR (config 38-39): If writing shipments to file, scale must be specified." << std::endl; exitflag=1;}
		if (pv[40]=="*"){std::cout << "ERROR (config 40): No diffusion kernel type specified." << std::endl; exitflag=1;}
		str_cast(pv[41],tempInt);
		if (tempInt<0){std::cout << "Warning (config 41): Quarantine must be zero or positive number of days. Setting to 0." << std::endl; 
			pv[41]="0";}
		checkExit = checkMeanVar(pv[42],42,"latency"); if (checkExit==1){exitflag=1;} // if exit triggered by this check, set exitflag=1
		checkExit = checkMeanVar(pv[43],43,"infectiousness"); if (checkExit==1){exitflag=1;} // if exit triggered by this check, set exitflag=1
		if (pv[48]!="*" && pv[49]=="*"){std::cout << "ERROR (config 48-49): If writing statuses to file, statuses must be specified." << std::endl; exitflag=1;}
		checkExit = checkMeanVar(pv[50],50,"reporting"); if (checkExit==1){exitflag=1;} // if exit triggered by this check, set exitflag=1
		str_cast(pv[51],tempInt);
		if (tempInt<0 || tempInt>100){std::cout << "ERROR (config 51): Shipment ban compliance must be specified as percentage. Setting to 0% (no ban)." << std::endl; 
			pv[51]="0";}
		if (pv[52]!="0" && pv[52]!="1"){std::cout << "ERROR (config 52): Shipment ban scale must be county-(0) or state-(1) level." << std::endl;  exitflag=1;}
		checkExit = checkMeanVar(pv[53],53,"ban initiation"); if (checkExit==1){exitflag=1;} // if exit triggered by this check, set exitflag=1
		checkExit = checkMeanVar(pv[54],54,"ban compliance"); if (checkExit==1){exitflag=1;} // if exit triggered by this check, set exitflag=1

		// within class functions = if method doesn't exist, say so in output
		// add verbose as argument to all functions called from main

		if (exitflag){
			std::cout << "Exiting..." << std::endl;
			exit(EXIT_FAILURE);
			}		
		
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
	for (auto& tv:tempVec){
		if(tv<0){std::cout << "ERROR (config "<<lineNum<<"): All parameters must be positive. " << std::endl;
		exitflag=1;}
	}
	return exitflag;
}

