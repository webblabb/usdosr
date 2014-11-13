#ifndef shared_functions_h
#define shared_functions_h

#include <random> // for random number generator in unif_rand
#include <cmath> // for std::sqrt in gKernel, floor in randomFrom
#include <vector> // for str_cast
#include <string>
#include <sstream>
#include <iostream> // just for troubleshooting output
#include <chrono>

#include "farm.h" // for Farm* for farm sus/inf

	double unif_rand();
 	double kernel(double dist);
	std::vector<std::string>
		split(const std::string&, char, std::vector<std::string>&);
	std::vector<std::string> 
		split(const std::string&, char);
	std::string to_string(Farm*);
	double getFarmSus(Farm*);
	double getFarmInf(Farm*);
	std::vector<double> orderNumbers(double, double);
	
	template<typename T> void str_cast(const std::string &s, T &ref)
{
	std::stringstream convert;
	convert << s;
	if(!(convert >> ref))
		ref = 0;
}

// choose a random element from a vector
	template<typename T> T randomFrom(std::vector<T>& vec)
{
	int maxSize = vec.size();
	int rUnif = unif_rand();
		if (rUnif == 1){rUnif = 0.99999;} // avoid out-of-range problems
	int rUnif = floor(unif_rand()*maxSize);
	return vec[rUnif];
}
#endif