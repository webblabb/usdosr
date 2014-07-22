#ifndef shared_functions_h
#define shared_functions_h

#include <random> // for random number generator in unif_rand
#include <cmath> // for std::sqrt in gKernel
#include <vector> // for str_cast
#include <string>
#include <sstream>
#include <iostream> // just for troubleshooting output

#include "farm.h" // for Farm* for farm sus/inf

	double unif_rand();
// 	double gKernel(double);
	double linearDist(double);
	std::vector<std::string>& split(const std::string&, char, std::vector<std::string>&);
	std::vector<std::string> split(const std::string&, char);
	double getFarmSus(Farm*);
	double getFarmInf(Farm*);
	
	template<typename T> void str_cast(const std::string &s, T &ref)
{
	std::stringstream convert;
	convert << s;
	if(!(convert >> ref))
		ref = 0;
}
#endif