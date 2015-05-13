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

#include "Point.h"


class County;
class State;

class Farm
{
	private:
		int id, cellID;
		double x_coordinate, y_coordinate, sus, inf;
		Point position;
		std::string fips;
		County* parent_county;
		State* parent_state;
		std::unordered_map< std::string, int > speciesCounts; // species and counts
		std::unordered_map< std::string, int > statuses; // used in Control_actions
		std::unordered_map< std::string, int > start; // used in Status_manager: start times for disease statuses
		std::unordered_map< std::string, int > end; // used in Status_manager: end times for disease statuses
		std::string diseaseStatus; // used in Status manager;

	public:
		Farm(int, double, double, std::string);
		~Farm();
		int get_id() const; // inlined
		int get_cellID() const; // inlined
		double get_x() const; // inlined
		double get_y() const; // inlined
		double get_sus() const; // inlined
		double get_inf() const; // inlined
 		std::string get_fips() const; // inlined
 		std::string get_diseaseStatus() const; //inlined

 		const std::unordered_map< std::string, int >* get_spCounts(); // inlined
 		int get_status(std::string) const; //inlined
 		int get_size(const std::string species) const;
 		int get_start(std::string) const; //inlined
 		int get_end(std::string) const; //inlined
		County* get_parent_county() const; //Inlined
 		State* get_parent_state() const; //Inlined
 		bool beenExposed() const; //inlined

		void set_cellID(const int cellID);
 		void set_speciesCount(const std::string, int);
 		void set_sus(const double);
 		void set_inf(const double);
 		void set_status(const std::string, const int);
 		void set_start(const std::string, const int); //inlined - set start time for disease status
 		void set_end(const std::string, const int); //inlined - set end time for disease status
		void set_diseaseStatus(std::string&); //inlined
		void set_parent_county(County* in_county);
 		void set_parent_state(State* in_state);

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
inline std::string Farm::get_fips() const
{
	return fips;
}
inline std::string Farm::get_diseaseStatus() const
{
	return diseaseStatus;
}
const inline std::unordered_map< std::string, int >* Farm::get_spCounts()
{
	return &speciesCounts;
}
inline int Farm::get_status(const std::string s) const
{
	return statuses.at(s);
}
inline void Farm::set_start(const std::string status, const int t)
{
	start[status] = t;
}
inline void Farm::set_end(const std::string status, const int t)
{
	end[status] = t;
}
inline int Farm::get_start(std::string s) const
{
	return start.at(s);
}
inline int Farm::get_end(std::string s) const
{
	return end.at(s);
}
inline void Farm::set_diseaseStatus(std::string& stat)
{
	diseaseStatus = stat;
}
inline bool Farm::beenExposed() const
{
	return start.count("exp")==1;
}
inline County* Farm::get_parent_county() const
{
    return parent_county;
}
inline State* Farm::get_parent_state() const
{
    return parent_state;
}

#endif //FARM_H
