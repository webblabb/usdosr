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
	
	public:
		Farm(int, std::string, double, double);
		~Farm();
		int get_id() const; //Inlined
		int get_cellID() const; //inlined
		double get_x() const; //Inlined
		double get_y() const; //Inlined
		double get_sus() const; //Inlined
		double get_inf() const; //Inlined
 		std::string get_status() const; //Inlined
 		std::string get_fips() const; //Inlined
 		int get_size(const std::string species) const;
 		 		
		void set_cellID(const int cellID);
 		void set_status(const std::string);
 		void set_speciesCount(const std::string, int);
 		void set_sus(const double);
 		void set_inf(const double);
};

inline int Farm::get_id() const
{
	return id;
}

inline std::string Farm::get_fips() const
{
	return fips;
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


inline int Farm::get_cellID() const
{
	return cellID;
}

inline std::string Farm::get_status() const
{
	return status;
}


#endif //FARM_H