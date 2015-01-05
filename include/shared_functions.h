// add functions to split comma-sep string into vector of ints

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
	double norm_rand();
 	double kernel(double dist);
	std::vector<std::string>
		split(const std::string&, char, std::vector<std::string>&);
	std::vector<std::string> 
		split(const std::string&, char);
	std::string to_string(Farm*);
	double getFarmSus(Farm*);
	double getFarmInf(Farm*);
	void removeFarmSubset(std::vector<Farm*>&, std::vector<Farm*>&);
// 	std::vector<Farm*> removeFarmSubset(std::vector<Farm*>&, std::vector<Farm*>&, bool);
	std::vector<double> stringToNumVec(std::string&);
	std::vector<int> stringToIntVec(std::string&);

	
	template<typename T> void str_cast(const std::string &s, T &ref)
	// str_cast(s, v) casts a number represented as a string and stores it in v,
	// v can be any type that can store numericals such as int, double etc.
{
	std::stringstream convert;
	convert << s;
	if(!(convert >> ref))
		ref = -1;
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

// combine multiple vectors and remove any duplicates
// input is vector of vectors to combine
	template<typename T> std::vector<T> uniqueFrom(std::vector<std::vector<T>>& vec)
{
	int totalElements = 0;
	std::unordered_map<T,int> combinedMap;
	for (auto& v1:vec){
		for (auto& v2:v1){
			totalElements++;
			if (combinedMap.count(v2)==0){combinedMap[v2]=1;}
		}
	}
	std::vector<T> toReturn;
	for (auto& cm:combinedMap){
		toReturn.emplace_back(cm.first);
	}
// 	std::cout << totalElements-combinedMap.size() <<" duplicate elements removed. ";
	return toReturn;
}

// determine which element's range a number falls into
// arguments are the number to match and vector of ordered maximums for each element
// returns largest element of elementMaxes that is less than or = toMatch
	template<typename T> int whichElement(T& toMatch, std::vector<T>& elementMaxes)
{
	int match = -1; // the element that will be returned
	if (toMatch > elementMaxes.back()){
		std::cout<<"Error: (whichElement): value to match exceeds largest of comparison values. Exiting..."
		<< std::endl; 
		exit(EXIT_FAILURE);}
	if (elementMaxes.size() < 1){
		std::cout << "Error (whichElement): Vector of element sizes < 1. Exiting..."
		<< std::endl; 
		exit(EXIT_FAILURE);}
	if (elementMaxes.size()==1){match=0;}
	else{
		bool found = 0;
		int it = 1;
		while (it!=elementMaxes.size() && found == 0){
			if (toMatch>=elementMaxes[it-1] && toMatch<elementMaxes[it]){ // >= than previous, < current
				match = it-1; // subtract one to get the element below
				found = 1;
			}
			it++;
		}
		if (it==elementMaxes.size() && found == 0){
			std::cout << "Warning: (whichElement): Match not found.";}
	}
	return match;
}

inline bool sortByID(const Farm* farm1, const Farm* farm2)
// "compare" function to sort farms by ID
// must be defined outside of class, or else sort doesn't work
{
	return (farm1 -> get_id()) < (farm2 -> get_id());
}	

#endif