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
	void removeFarmSubset(std::vector<Farm*>&, std::vector<Farm*>&);
	int whichElement(double&, std::vector<int>&);
	
	template<typename T> void str_cast(const std::string &s, T &ref)
{
	std::stringstream convert;
	convert << s;
	if(!(convert >> ref))
		ref = 0;
}

	// used in grid_manager to look up kernel values
	template<typename T> std::vector<T> orderNumbers(T& number1, T& number2)
	// order number1 and number2 from lowest to highest
{
	std::vector<T> ordered;
	ordered.emplace_back(number1);
	if (number2 < number1){
		ordered.insert(ordered.begin(),number2);
	} else {
		ordered.emplace_back(number2); // if number2 is larger or equal to number1
	}
	return ordered;
}

// choose a random element from a vector
	template<typename T> T randomFrom(std::vector<T>& vec)
{
	int maxSize = vec.size();
	double rUnif = unif_rand();
		if (rUnif == 1){rUnif = 0.99999;} // avoid out-of-range problems
	rUnif = floor(unif_rand()*maxSize);
	return vec[rUnif];
}

// check if something is in a vector
	template<typename T> bool isWithin(T& target, std::vector<T>& vec)
{
	auto it = vec.begin();
	bool found = 0;
	while (it!=vec.end() && found==0){ // keep going while not done with vector and target not found
		if(*it == target){found = 1;}
		it++;
	}
	
	return found;
}

inline bool sortByID(const Farm* farm1, const Farm* farm2)
// "compare" function to sort farms by ID
// must be defined outside of class, or else sort doesn't work
{
	return (farm1 -> get_id()) < (farm2 -> get_id());
}	

#endif