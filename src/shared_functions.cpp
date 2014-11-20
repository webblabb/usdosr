#include "shared_functions.h"

// Used in gridding (decision making for stepping into cells)
// Used in pairwise evaluations in main
double unif_rand()
{
	static std::uniform_real_distribution<double> unif_dist(0.0, 1.0);
	static unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
	static std::mt19937 generator(seed); //Mersenne Twister pseudo-random number generator. Generally considered research-grade.
	return unif_dist(generator);
}

// Used in gridding (kernel values for fixed grid distances)
// Used in pairwise evaluations in main
double kernel(double dist)
{	
	double usedist = dist;
	if (usedist==0){usedist = 1;}
	return std::min(1.0, 0.12 / (1 + pow((usedist/1000), 3)));
}

// used in reading in files
std::vector<std::string> 
	split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
	{
		if (!item.empty())
		{
			elems.emplace_back(item);
		}
    }
    return elems;
}

std::vector<std::string> 
	split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

// used to print infection results from main
std::string to_string(Farm* farm)
// overloaded to_string function, makes tab-delim string (one line) for farm with ID, cellID, x, y, pop
{
	std::string toPrint;
	char temp[20];
	std::vector<double> vars;
		vars.resize(5);
		vars[0] = farm->get_id();
		vars[1] = farm->get_cellID();
		vars[2] = farm->get_x();
		vars[3] = farm->get_y();
		vars[4] = farm->get_size();

	for(auto it:vars){ // for each element in vars (each farm variable)
		sprintf(temp, "%f\t", it);
		toPrint += temp;
	}
	toPrint.replace(toPrint.end()-1, toPrint.end(), "\n");
	
	return toPrint;
}

// Used by grid_cell to determine max values for inf/sus farms in cell
// Used by Grid_manager to get individual farm sus/inf values
double getFarmSus(Farm* f)
{
	// species-specific susceptibility - each element is a species
	std::vector <double> speciesSus;
	// will be replaced by input from config file
	speciesSus.emplace_back(10.5);
	
	double farmSize = f -> get_size(); // will be a vector, double for now
	// will be sum of each species (count*transmission)
	double farmSus = farmSize * speciesSus[0];
	
	return farmSus;
}

double getFarmInf(Farm* f)
{
	// species-specific infectiousness - each element is a species
	std::vector <double> speciesInf;
	// will be replaced by input from config file
	speciesInf.emplace_back(0.00000077);
	
	double farmSize = f -> get_size(); // will be a vector, double for now
	// will be sum of each species (count*transmission)
	double farmInf = farmSize * speciesInf[0];

	return farmInf;
}

// uses fips-indexed maps to reduce unnecessary comparisons
void removeFarmSubset(std::vector<Farm*>& subVec, std::vector<Farm*>& fullVec)
// remove farms in first vector from second vector
{
	int expectedSize = fullVec.size()-subVec.size();
//	std::cout << "Removing "<<subVec.size()<<" farms from list of "<<fullVec.size()<<std::endl;

	// put vectors into fips-indexed maps
	std::unordered_map< std::string, std::vector<Farm*> > subMap, fullMap; 
	for (auto& sv:subVec){
		subMap[sv->get_fips()].emplace_back(sv);}
	for (auto& fv:fullVec){
		fullMap[fv->get_fips()].emplace_back(fv);}

	for (auto& sub:subMap){
		// for each fips in sub list
		std::string fips = sub.first;
		// sort both lists of farms with this fips
		std::sort(sub.second.begin(),sub.second.end(),sortByID);
		std::sort(fullMap.at(fips).begin(),fullMap.at(fips).end(),sortByID);
		// iterate through full list, erasing matching sub as found
		auto it2 = fullMap.at(fips).begin();
		for(auto it = sub.second.begin(); it != sub.second.end(); it++)
		// loop through each farm in this FIPS
		{		
			
			while (it2 != fullMap.at(fips).end()){ // while end of full list not reached
				if(*it2 == *it){ // finds match in farmList to farmInCell
					fullMap.at(fips).erase(it2); // remove from farmList
					break; // start at next farm instead of looping over again
				}
				it2++;
			}
		}	
	}
	
	// rewrite fullVec
	std::vector<Farm*> temp;
	for (auto& f1:fullMap){
	  for (auto& f2:f1.second){
		temp.emplace_back(f2);}}
	fullVec = temp;
		
	if (expectedSize != fullVec.size()){std::cout << "Error in removeFarmSubset: expected size"<< expectedSize <<
		", actual size: "<< fullVec.size() <<". ";}
}


// determine which element's range a number falls into
// input is maximum for each element
int whichElement(double& toMatch, std::vector<int>& elementMaxes)
{
	int match = -1;
	if (elementMaxes.size() < 1){std::cout << "Vector of element sizes < 1. ";}
	if (toMatch < 0 || toMatch > 1){std::cout << "Number to match out of range. ";}
	else{
		double matchValue = toMatch * elementMaxes.back(); // scale up to 
		elementMaxes.insert(elementMaxes.begin(),0); // add 0 as minimum end of first range
		if (elementMaxes[0]!=0){std::cout << "0 insertion error. ";}
		bool found = 0;
		int it = 1;
		while (it!=elementMaxes.size() && found == 0){
			if (matchValue>elementMaxes[it-1] && matchValue<=elementMaxes[it]){ // greater than previous, <= current
				match = it-1; // subtract one to make up for the 0 we added at the beginning
				found = 1;}
			it++;
			if (it==elementMaxes.size() && found == 0){
				std::cout << " Match not found.";}
		}
	}
	return match;
}

