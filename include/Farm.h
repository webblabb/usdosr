/* A class describing a premises object.
One object of this type is created for each premises 
described by the input data. 

Contains the data members:
id, a unique integer identifying this premises
x and y coordinates of the premises
fips, the county identifier (string)
status, the current infection status (mostly used to check if a premises is susceptible)
speciesCounts, a species-indexed map of how many animals of each species are on a premises

Contains the public member functions:
get_id(), get_cellID return corresponding data member as int.
get_x(), get_y() return corresponding data member as double.
get_fips() & get_status(), returns fips/status as string.

set_cellID(int), set_status(std::string), sets the cellID/status to the provided argument.
set_speciesCount(std::string species, int count) sets the population for "species" to "count"
 */

#ifndef FARM_H
#define FARM_H

#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>

class Farm
{
	private:
		int id, cellID;
		double x_coordinate, y_coordinate, sus, inf;
		std::string fips, status;
		std::unordered_map< std::string, int > speciesCounts;
		std::vector< std::vector<int> > time_ExpSource; // "time-exposure source"
		// element [0] holds time infected, following element(s) indicate(s) source premises id and method of exposure:
		// 0: local spread, 1: shipments
		// most will be of vector length 1, unless simultaneously infected locally and by shipments
	
	public:
		Farm(int, std::string, double, double);
		~Farm();
		int get_id() const; // inlined
		int get_cellID() const; // inlined
		double get_x() const; // inlined
		double get_y() const; // inlined
		double get_sus() const; // inlined
		double get_inf() const; // inlined
 		std::string get_status() const; // inlined
 		std::string get_fips() const; // inlined
 		std::unordered_map< std::string, int > get_spCounts() const; // inlined
 		std::vector< std::vector<int> > get_timeExp() const; // inlined
 		
 		int get_size(const std::string species) const;
 		
		void set_cellID(const int cellID);
 		void set_status(const std::string);
 		void set_speciesCount(const std::string, int);
 		void set_sus(const double);
 		void set_inf(const double);
 		void set_time_exp(const std::vector<int>&);
};

inline int Farm::get_id() const
{
	return id;
}
inline int Farm::get_cellID() const
{
	return cellID;
}
inline double Farm::get_x() const
{
	return x_coordinate;
}
inline double Farm::get_y() const
{
	return y_coordinate;
}
inline double Farm::get_sus() const
{
	return sus;
}
inline double Farm::get_inf() const
{
	return inf;
}
inline std::string Farm::get_status() const
{
	return status;
}
inline std::string Farm::get_fips() const
{
	return fips;
}
inline std::unordered_map< std::string, int > Farm::get_spCounts() const
{
	return speciesCounts;
}
inline std::vector< std::vector<int> > Farm::get_timeExp() const
{
	return time_ExpSource;
}

#endif //FARM_H