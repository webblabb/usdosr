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
// Used in Status_manager to determine status duration
double norm_rand()
{
	static std::normal_distribution<double> norm_dist(0,1);
	static unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
	static std::mt19937 generator(seed); //Mersenne Twister pseudo-random number generator. Generally considered research-grade.
	return norm_dist(generator);
}
// Used in gridding (binomial method) to determing # of infected farms
int draw_binom(int N, double prob)
// draw from a binomial distribution based on N farms and prob (calc with focalInf & gridKern)
{		
	std::binomial_distribution<int> binom_dist(N,prob);
	static unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
	static std::mt19937 generator(seed); //Mersenne Twister pseudo-random number generator. Generally considered research-grade.
	return binom_dist(generator);
}

double oneMinusExp(double x)
// based on algo found at http://www.johndcook.com/blog/cpp_expm1/
{
	if (fabs(x) < 1e-5){
			return -(x + 0.5*x*x); // two-term Taylor approx for x<1e-5
	} else {
			return -(exp(x) - 1.0);
	}
}

// used by status_manager and control_rules
int normDelay(std::tuple<double, double>& params)
// determine length of period drawn from normal distribution
{
	double mean = std::get<0>(params);
	double var = std::get<1>(params);
	double normDraw = norm_rand()*var + mean; // scaled to # drawn from N(0,1)
	int draw = (int)(normDraw+0.5); // round up to nearest day
	if(draw<1){draw = 1;}
	return draw;
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

// used in commitCell in grid initiation
void removeFarmSubset(std::vector<Farm*>& subVec, std::vector<Farm*>& fullVec)
// remove farms in first vector from second vector
{
	unsigned int expectedSize = fullVec.size()-subVec.size();
//	std::cout << "Removing "<<subVec.size()<<" farms from list of "<<fullVec.size()<<std::endl;

	// put vectors into fips-indexed maps to speed up matching
	std::unordered_map< std::string, std::vector<Farm*> > subMap, fullMap; 
	for (auto& sv:subVec){
		subMap[sv->get_fips()].emplace_back(sv);}
	for (auto& fv:fullVec){
		fullMap[fv->get_fips()].emplace_back(fv);}

	for (auto& sub:subMap){
		// for each fips in subset list
		std::string fips = sub.first;
		// if needed, sort both lists of farms in this FIPS, by ID
		std::sort(sub.second.begin(),sub.second.end(),sortByID<Farm*>);
		std::sort(fullMap.at(fips).begin(),fullMap.at(fips).end(),sortByID<Farm*>);
		// iterate through full list, erasing matching sub as found
		auto it2 = fullMap.at(fips).begin();
		for(auto it = sub.second.begin(); it != sub.second.end(); it++){
		// loop through each farm in this FIPS
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
		
	if (expectedSize != fullVec.size()){
		std::cout << "Error in removeFarmSubset: expected size"<< expectedSize <<
		", actual size: "<< fullVec.size() <<". Exiting...";
		exit(EXIT_FAILURE);
	}

}

std::vector<double> stringToNumVec(std::string& toConvert)
{
	std::vector<double> output;
    std::string delim = ",";
    std::string substring;
    double temp;

    auto start = 0;
    auto end = toConvert.find(delim); // first delimiter occurrence
    while (end != std::string::npos)
    {
        substring = toConvert.substr(start, end - start);
        substring.erase(std::remove_if(substring.begin(), substring.end(), isspace), substring.end()); // remove whitespace
        str_cast(substring,temp); // convert substring to double
        output.emplace_back(temp); // add double to vector
        start = end + delim.length(); // set end(+delimiter) as new start point
        end = toConvert.find(delim, start); // find new endpoint
    }
    substring = toConvert.substr(start, end); // last substring
    substring.erase(std::remove_if(substring.begin(), substring.end(), isspace), substring.end()); // remove whitespace
	str_cast(substring,temp); // convert substring to double
    output.emplace_back(temp); // add double to vector
    
    return output;
}

std::vector<int> stringToIntVec(std::string& toConvert)
{
	std::vector<int> output;
    std::string delim = ",";
    std::string substring;
    int temp;

    auto start = 0;
    auto end = toConvert.find(delim); // first delimiter occurrence
    while (end != std::string::npos)
    {
        substring = toConvert.substr(start, end - start);
        substring.erase(std::remove_if(substring.begin(), substring.end(), isspace), substring.end()); // remove whitespace
        str_cast(substring,temp); // convert substring to double
        output.emplace_back(temp); // add double to vector
        start = end + delim.length(); // set end(+delimiter) as new start point
        end = toConvert.find(delim, start); // find new endpoint
    }
    substring = toConvert.substr(start, end); // last substring
    substring.erase(std::remove_if(substring.begin(), substring.end(), isspace), substring.end()); // remove whitespace
	str_cast(substring,temp); // convert substring to double
    output.emplace_back(temp); // add double to vector
    
    return output;
}

std::vector<std::string> stringToStringVec(std::string& toConvert)
{
	std::vector<std::string> output;
    std::string delim = ",";
    std::string substring;

    auto start = 0;
    auto end = toConvert.find(delim); // first delimiter occurrence
    while (end != std::string::npos)
    {
        substring = toConvert.substr(start, end - start);
        substring.erase(std::remove_if(substring.begin(), substring.end(), isspace), substring.end()); // remove whitespace
        output.emplace_back(substring); // add string to vector
        start = end + delim.length(); // set end(+delimiter) as new start point
        end = toConvert.find(delim, start); // find new endpoint
    }
    substring = toConvert.substr(start, end); // last substring
    substring.erase(std::remove_if(substring.begin(), substring.end(), isspace), substring.end()); // remove whitespace
    output.emplace_back(substring); // add string to vector
    
    return output;
}

// convert vector of one type (int) to comma-separated string
std::string vecToCommaSepString(const std::vector<int> vecToPaste)
{
	std::string output;
	char temp[10];
	for (auto& v:vecToPaste){
		sprintf(temp, "%d,", v);
		output += temp;
	}
	output.pop_back(); // remove last comma
	return output;
}

// overload for vector of strings
std::string vecToCommaSepString(const std::vector<std::string> vecToPaste)
{
	std::string output;
	for (auto& v:vecToPaste){
		output += v;
		output += ",";
	}
	output.pop_back();  // remove last comma
	return output;
}

void addItemTab(std::string& outString, int toAdd){
	char temp[20];
	sprintf(temp, "%d\t", toAdd);
	outString += temp;
}

void addItemTab(std::string& outString, double toAdd){
	char temp[50];
	sprintf(temp, "%.2f\t", toAdd);
	outString += temp;
}

void addItemTab(std::string& outString, std::string toAdd){
	outString += toAdd;
	outString +="\t";
}

// function for printing (adding) output to specified file
void printLine(std::string& outputFile, std::string& printString, bool overwrite)
{
	std::ofstream outfile;
	if (!overwrite){
		outfile.open(outputFile, std::ios::app); // append to existing file
	} else {
		outfile.open(outputFile); // overwrite/create new file
	}
	if(!outfile){std::cout<<"File "<<outputFile<<" not open."<<std::endl;}
	outfile << printString; 
	outfile.close();
}

