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

// used in grid_manager to look up kernel values
std::vector<double> orderNumbers(double number1, double number2)
// order number1 and number2 from lowest to highest
{
	std::vector<double> ordered;
	ordered.emplace_back(number1);
	if (number2 < number1){
		ordered.insert(ordered.begin(),number2);
	} else {
		ordered.emplace_back(number2); // if number2 is larger or equal to number1
	}
	return ordered;
}