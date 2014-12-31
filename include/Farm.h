/* A class describing a farm object.
One object of this type is created for each farm/premises 
described by the input data. 

Contains the data members:
id, a unique integer identifying this farm (PPN)
x and y coordinates of the farm
size, animal-holding capacity
status, a flag describing the infectious status of the farm:
		0 - susceptible
		1 - infected but not infectious
		2 - infected and infectious
		3 - recovered/removed
Time_transp_map, an std::unordered_map containing timesteps as keys
and a vector as value. The vector contains pointers to transport objects
that originate from this farm. The idea is that this map provides an easy 
way to quickly provide access to the transports that need to be processed
during the current timestep instead of going through all transports that originate
from this farm.

Contains the public member functions:
get_id(), get_x(), get_y() returns corresponding data member as int.
get_size() & get_status(), returns size/status as short.
set_status(short), sets the status to the provided argument. Needs failsafe/logging functionality.
set_time_of_infection(short), sets the timestep that infection occurred.
get_transports(short), returns a vector of pointers to the transport objects that has
this farm as origin.
add_live_transport(Live_transport*) used in the initialization process. Takes a pointer to a
transport object and associates it (adds it to Transp_vector[timestep]) with this farm. */

#ifndef FARM_H
#define FARM_H

#include <string>
#include <unordered_map>
#include <vector>

class Farm
{
	private:
		int id, size, cellID;
		double x_coordinate, y_coordinate;
		std::string fips, status;
//		int id, x_coordinate, y_coordinate, infected_at_timestep;
// 		short size, status;
	
	public:
		Farm(int, std::string, double, double, int, std::string);
		~Farm();
		int get_id() const; //Inlined
		std::string get_fips() const; //Inlined
		double get_x() const; //Inlined
		double get_y() const; //Inlined
		int get_size() const; //Inlined
		int get_cellID() const; //inlined
 		std::string get_status() const; //Inlined
 		
// 		std::string get_info() const;
 		
		void set_cellID(const int cellID);
 		void set_status(const std::string);
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

inline int Farm::get_size() const
//inline short Farm::get_size() const
{
	return size;
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