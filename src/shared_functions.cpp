#include "shared_functions.h"

// Used in gridding (decision making for stepping into cells)
double unif_rand()
{
	static std::uniform_real_distribution<double> unif_dist(0.0, 1.0);
	static unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
	static std::mt19937 generator(seed); //Mersenne Twister pseudo-random number generator. Generally considered research-grade.
	return unif_dist(generator);
}

// double gKernel(double dist)
// // retrieves kernel value based on distance
// {
// 	if (dist==0){dist = 0.00001;}
// 	double gaussian_a = sqrt(2)*(dist / 2.99); // our old friend the gaussian kernel
// 	double temp = dist/gaussian_a;
// 	return 1 * exp(-(temp*temp));
// }

// Used in gridding (kernel values for fixed grid distances)
double linearDist(double dist) // kernel radius relative to min/max grid cell distances - made up
{
	double y = dist*-(0.8/1000) + 0.8;
	if (y < 0){y = 0;}
	return y;
}

std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems)
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

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

// Used by grid_cell to determine max values for inf/sus farms in cell
// Used by grid_cell_checker to get individual farm sus/inf values
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
