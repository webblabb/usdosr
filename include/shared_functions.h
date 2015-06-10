#ifndef shared_functions_h
#define shared_functions_h

#include <algorithm>
#include <random> // for random number generator
#include <chrono> // for random number generator
#include <cmath> // for std::sqrt in gKernel, floor in randomFrom
#include <fstream> // for printing
#include <iostream> // for troubleshooting output
#include <sstream>
#include <unordered_map>

class Farm; // for Farm* for farm sus/inf

//struct County // used in Status and Control
//{
//	std::string fips;
//	std::unordered_map<std::string, int> statuses; // for control type and level
//};

struct Shipment // used in Shipment, Status
{
	int t; // time of shipment
	int origID; // premises ID of shipment origin
	int destID; // premises ID of shipment destination
	std::string origFIPS;
	std::string destFIPS;
	std::string species;
	bool transmission; // if this is from an infectious to a susceptible premises
	int ban; // 0 = no ban, 1 = ban ordered but not active, 2 ban ordered & active, 3 = ban active & compliant
};

	double unif_rand();
	double norm_rand();
	int draw_binom(int, double);
	unsigned int generate_distribution_seed();
	double oneMinusExp(double);
 	int normDelay(std::tuple<double, double>&);
	std::vector<std::string>
		split(const std::string&, char, std::vector<std::string>&);
	std::vector<std::string>
		split(const std::string&, char);
    void skipBOM(std::ifstream &in);
	std::string to_string(Farm*);
 	void removeFarmSubset(std::vector<Farm*>&, std::vector<Farm*>&);
	std::vector<double> stringToNumVec(std::string&);
	std::vector<int> stringToIntVec(std::string&);
	std::vector<std::string> stringToStringVec(std::string&);
	std::string vecToCommaSepString(const std::vector<int> vecToPaste);
	std::string vecToCommaSepString(const std::vector<std::string> vecToPaste);
	void addItemTab(std::string&, int);
	void addItemTab(std::string&, double);
	void addItemTab(std::string&, std::string);
	void printLine(std::string&, std::string&);


template<typename T>
void str_cast(const std::string &s, T &ref)
	// str_cast(s, v) casts a number represented as a string and stores it in v,
	// v can be any type that can store numericals such as int, double etc.
{
	std::stringstream convert;
	convert << s;
	if(!(convert >> ref))
		ref = -1;
}

template<typename T>
T stringToNum(const std::string& text)
{
	std::istringstream ss(text);
	T result;
	if (! (ss>>result)){result = -1;}
	return result;
}

// choose a random element from a vector
template<typename T>
T randomFrom(std::vector<T>& vec)
{
	int maxSize = vec.size();
	double rUnif = unif_rand();
    if (rUnif ==1 )
    {
        rUnif=0.999; // avoids assigning actual maxSize value (out of range)
    }
	int rIndex = (int) floor(rUnif*maxSize);
	return vec[rIndex];
}

// choose multiple random elements from a vector based on the Fisher-Yates shuffling algorithm:
// num_random selected random values are copied to output,
// selected values are swapped to the end of the vector so they're not selected again
// Used in Grid_manager to select binomial-success farms
// Used in Shipping_manager to select random premises in counties
template<typename T>
void random_unique(std::vector<T> elements, int num_random, std::vector<T>& output1)
	// elements not referenced (&) because we're rearranging it
{
	std::vector<T> output;
	output.reserve(num_random);
	int endIndex = elements.size();
	// endIndex separates non-selected values (elements [0, endIndex-1]) from selected values
	for (auto i = 1; i<= num_random; i++){
		// choose random number between 0 and 1
		double rUnif = unif_rand();
		// scale up to endIndex, so r is an index in [0, endIndex)
		if (rUnif == 1 ){rUnif=0.999;} // avoids assigning actual endIndex value (out of range)
		int r = (int)floor(rUnif*endIndex);
		// copy value to output
		output.emplace_back(elements[r]);
		// swap r and endIndex-1 (last element of non-selected values)
		std::swap(elements[r], elements[endIndex-1]);
		endIndex--;
	}
 output.swap(output1);
}

// check if something is in a vector
template<typename T>
bool isWithin(const T target, const std::vector<T> vec)
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
template<typename T>
std::vector<T> uniqueFrom(std::vector<std::vector<T>>& vec)
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
template<typename T>
int whichElement(T& toMatch, std::vector<T>& elementMaxes)
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
		unsigned int it = 1;
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

template<typename T>
inline bool sortByID(const T item1, const T item2)
// "compare" function to sort farms by ID
//  used with grid_cell*s in grid checker: stepThroughCells
// must be defined outside of class, or else sort doesn't work
{
	return (item1 -> get_id()) < (item2 -> get_id());
}

#endif
