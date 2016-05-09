#ifndef file_manager_h
#define file_manager_h

#include <fstream>
#include "shared_functions.h"
#include "Local_spread.h"
// shared_functions includes iostream, sstream, string, vector, parameters definition

extern int verboseLevel;

/// Contains all parameters loaded from file
struct Parameters
{
	// output parameters
	std::string batch; ///< Prefix for summary and detailed output files
	int printSummary;
	int printDetail;
	int printCells;
	int printShipments;
	int printControl;

	// general parameters
	std::string premFile; ///< File containing tab-delimited premises data: ID, FIPS, x, y, population
	std::string fipsFile;
	std::string fips_weights;
	std::vector<std::string> species;
	int timesteps;
	int replicates;
	int verboseLevel;
	bool pairwiseOn;
	bool reverseXY;

	// infection parameters
	std::string seedSource;
	std::string dataKernelFile; ///< Name of file containing local spread probabilities by distance (set kernelType to 1)
	std::string seedSourceType;
	std::unordered_map<std::string,double> susExponents; ///< Species-specific exponents for susceptibility (q in USDOSv1)
	std::unordered_map<std::string,double> infExponents; ///< Species-specific exponents for infectiousness (p in USDOSv1)
	std::unordered_map<std::string,double> susConsts; ///< Species-specific constants for susceptibility (S in Tildesley ProcB 2008)
	std::unordered_map<std::string,double> infConsts; ///< Species-specific constants for infectiousness (T in Tildesley ProcB 2008)
	std::tuple<double,double> latencyParams;
	std::tuple<double,double> infectiousParams;
	// local spread kernel object (for grid manager and checker)
	int kernelType;
	std::vector<double> kernelParams;
	Local_spread* kernel;

	// grid parameters
	std::string cellFile;
	std::vector<int> densityParams;
	int uniformSide;

	// shipment parameters
	std::vector<int> shipMethods;
	std::vector<int> shipMethodTimeStarts;
	int shipPremAssignment;

	// control parameters
	std::tuple<double,double> indexReportLag;
	std::tuple<double,double> reportLag;
	// shipping ban
	double shipBanCompliance;
	int banLevel;
	std::tuple<double,double> reportToOrderBan;
	std::tuple<double,double> orderToCompliance;

	// related parameter sets
	std::unordered_map< std::string, std::tuple<double,double> > lagParams;
	std::unordered_map< std::string, std::vector<std::tuple<double,double>> > controlLags;
};

/// Loads and checks parameters from configuration file
class file_manager
{
	private:
		int verbose; ///< Can be set to override global setting for console output
		std::vector<std::string> pv; ///< Parameter vector for reading in from file
		Parameters params;
		
		bool checkMeanVar(std::string&, int, std::string);
		bool checkPositive(std::vector<int>&, int);
		bool checkPositive(std::vector<double>&, int);

	public:
		file_manager();
		~file_manager();
		const Parameters* getParams(); // inlined
		const std::string getSettings(std::string&);
		void readConfig(std::string&);

};

inline const Parameters* file_manager::getParams()
{
	return &params;
}

#endif // file_manager_h
