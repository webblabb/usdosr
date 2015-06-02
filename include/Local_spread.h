#ifndef Local_spread_h
#define Local_spread_h

#include <iterator> // std::prev
#include <map>
#include "shared_functions.h" // for split; contains cmath, fstream, iostream

extern int verboseLevel;

class Local_spread
{
	private:
		int verbose;
		int kType; // kernel type
		std::vector<double> kp; // kernel parameters
		std::string datafile; // file containing data-based risk
		std::map<double,double> distProb; // key is distance (m) squared, value is probability	
		
	public:
		/// Constructs a kernel with power law parameters
		Local_spread(int kernelType, 
			std::vector<double> kparams = std::vector<double>());
		/// Constructs a kernel from external file
		Local_spread(int kernelType, 
			std::string fname);
		~Local_spread();
		/// Returns kernel value at distance
		double atDistSq(double);
};
	
#endif // Local_spread_h