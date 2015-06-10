#ifndef file_manager_h
#define file_manager_h

#include <fstream>
#include "shared_functions.h"
// shared_functions includes iostream, sstream, string, vector

extern int verboseLevel;

struct Parameters{
	// output parameters
	std::string batch;
	int printSummary;
	int printDetail;
	int printCells;
	int printShipments;
	int printControl;

	// general parameters
	std::string premFile;
	std::string fipsFile;
	std::string fips_weights;
	std::vector<std::string> species;
	int timesteps;
	int replicates;
	int verboseLevel;
	bool pairwiseOn;
	bool reverseXY;

	// infection parameters
	std::string seedPremFile;
	std::string seedCountyFile;
	int seedMethod;
	std::vector<double> susExponents;
	std::vector<double> infExponents;
	std::vector<double> susConsts;
	std::vector<double> infConsts;
	int kernelType;
	std::vector<double> kernelParams;
	std::tuple<double,double> latencyParams;
	std::tuple<double,double> infectiousParams;

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

class file_manager
{
	private:
		int verbose;
		std::vector<std::string> pv; // parameter vector for loading in
		Parameters params; // struct of all parameters

		bool checkMeanVar(std::string&, int, std::string);
		bool checkPositive(std::vector<int>&, int);
		bool checkPositive(std::vector<double>&, int);

	public:
		file_manager();
		~file_manager();
		const Parameters* getParams(); // inlined
		void readConfig(std::string&);
};

inline const Parameters* file_manager::getParams()
{
	return &params;
}

#endif
