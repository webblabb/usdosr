#include "Local_spread.h"

Local_spread::Local_spread(int kernelType, std::vector<double> kparams)
	:
	kType(kernelType),
	kp(kparams)
{	
	verbose = verboseLevel; // for manually overriding verboseLevel
	// one-time calculations upon construction
	switch (kType)
	{
		case 0:{ // power law function
		// kp[0] is k1, [1] is k2, [2] is k3
		// precalculate k3/2 and add as fourth parameter at [3]
		// precalculate k2^k3 and add as fifth parameter at [4]
			kp.emplace_back(kp.at(2)/2);
			kp.emplace_back(pow(kp.at(1),kp.at(2)));
			break;
		}
		
		default:{
			std::cout << "Kernel type must be 0 if constructing with parameters. Exiting..." << std::endl; exit(EXIT_FAILURE);
		}
	}
}

/// Overloaded constructor to accept a data-based kernel from external file
Local_spread::Local_spread(int kernelType, std::string fname)
	:
	kType(kernelType),
	datafile(fname)
{	
	verbose = verboseLevel; // for manually overriding verboseLevel
	// one-time calculations upon construction
	switch (kType)
	{
		case 1:{ // data-based levels
std::cout<<"Creating a kernel with datafile "<<datafile<<std::endl;
		// read in levels from file
			std::ifstream d(datafile);
			if(!d){std::cout << "Data-based local spread file not found. Exiting..." << std::endl; exit(EXIT_FAILURE);}
			if(d.is_open()){
if(verbose>1){std::cout << "Data-based local spread file open." << std::endl;}
				while(! d.eof()){
					std::string line;
					getline(d, line); // get line from file "f", save as "line"
					std::vector<std::string> line_vector = split(line, ' '); // separate by tab
					if(! line_vector.empty()){ // if line_vector has something in it
						double distM = stringToNum<double>(line_vector[0]);
						// store distance as distance-squared
						distProb[distM*distM] = stringToNum<double>(line_vector[1]);
if(verbose>1){std::cout<<" At "<<distM<<", risk is "<<distProb[distM*distM]<<std::endl;}
					} // close "if line_vector not empty"
				} // close "while not end of file"
			} // close "if file is open"
			break;
		}
		
		default:{
			std::cout << "Kernel type must be 1 if constructing from file. Exiting..." << std::endl; exit(EXIT_FAILURE);
		}
	}
}

Local_spread::~Local_spread()
{
}

/// \param[in] distance squared
/// \returns kernel value
double Local_spread::atDistSq(double distSq)
{
	double k;
	switch (kType)
	{
		case 0:{ // power law function
			k = kp.at(0)/(1+pow(distSq,kp.at(3))/kp.at(4));
			break;
			/* 
			to demonstrate this is the same as kernel(dist), run in R:
			usedist=1:2000
			k1 = 0.12
			k2 = 1000
			k3 = 3
			# original f(distance)
			plot(k1 / (1 + (usedist/k2)^k3) ~ usedist) 
			usq = usedist^2
			points(usedist, (k1 / (1 + (usq^(k3/2))/(k2^k3))),col="blue",pch="*")
			*/
		}
		case 1:{ // UK data-based levels
			// if distance is past last value, use last value
			if (distSq >= distProb.rbegin()->first){k = 0;			
if(verbose>1){std::cout << "Distance^2 past end, set to 0"<<std::endl;}
			} else {
				auto equalOrHigher = distProb.lower_bound(distSq); // finds equal or higher match to distSq key
				k = equalOrHigher->second;
				// check if the previous key (if applicable) is closer
				if (equalOrHigher != distProb.begin()){
					auto lower = std::prev(equalOrHigher); // gets one key before that
					// use whichever has less difference (is closer)
					if ( std::abs(distSq - lower->first) < std::abs(equalOrHigher->first - distSq)){
						k = lower->second;
					}
				}
if(verbose>1){std::cout << "Matched distance "<<std::sqrt(distSq)<<" with probability value of "<<k<<std::endl;}
			}
			break;
		}
		default:{
			std::cout << "Unrecognized kernel type. Exiting..." << std::endl; exit(EXIT_FAILURE);
		}
	}
 return std::min(1.0,k);
}