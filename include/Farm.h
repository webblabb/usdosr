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

//class Live_transport;
//class Slaughter_transport;

// typedef std::vector<Live_transport*> Transp_vector;
// typedef std::unordered_map<short, Transp_vector> Time_transp_map;
// typedef std::unordered_map<int, std::vector<Slaughter_transport*>> Time_sl_map;

class Farm
{
	private:
		double id, x_coordinate, y_coordinate, size, cellID;
		std::string fips;
//		int id, x_coordinate, y_coordinate, infected_at_timestep;
// 		short size, status;
// 		Time_transp_map timed_transports;
// 		Time_sl_map timed_sl_transports;
	
	public:
		Farm(double, std::string, double, double, double);
// 		Farm(int, int, int, short);
		~Farm();
		double get_id() const; //Inlined
		std::string get_fips() const; //Inlined
		double get_x() const; //Inlined
		double get_y() const; //Inlined
		double get_size() const; //Inlined
		double get_cellID() const; //inlined

//  	short get_size() const; //Inlined
// 		short get_status() const; //Inlined
// 		int get_infected_at() const; //Inlined
// 		std::string get_info() const;
// 		Transp_vector* get_transports(const short);	//Inlined
// 		std::vector<Slaughter_transport*>* get_sl_transports(int const);
// 		
		void set_cellID(const double cellID);
// 		void set_status(const short);
// 		void set_time_of_infection(const int);
// 		void add_live_transport(Live_transport*);
// 		void add_sl_transport(int const, Slaughter_transport*);
};

inline double Farm::get_id() const
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

inline double Farm::get_size() const
//inline short Farm::get_size() const
{
	return size;
}

inline double Farm::get_cellID() const
{
	return cellID;
}

// inline short Farm::get_status() const
// {
// 	return status;
// }
// 
// inline int Farm::get_infected_at() const
// {
// 	return infected_at_timestep;
// }
// 
// inline Transp_vector* Farm::get_transports(const short timestep)
// {
// 	if(timed_transports.find(timestep) != timed_transports.end())
// 	{
// 		if(!timed_transports[timestep].empty())
// 		{
// 			return &timed_transports[timestep];
// 		}
// 	}
// 	return NULL;
// }

#endif //FARM_H