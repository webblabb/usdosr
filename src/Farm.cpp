#include <stdio.h>
#include <iostream>
#include "Farm.h"
// #include "Live_transport.h"
// #include "Slaughter_transport.h"

//Farm::Farm(int in_id, int in_x, int in_y, short in_size)
Farm::Farm(double in_id, std::string in_fips, double in_x, double in_y, double in_size)
{
	id = in_id;
	fips = in_fips;
	x_coordinate = in_x;
	y_coordinate = in_y;
	size = in_size;
	
// 	status = 0;
// 	infected_at_timestep = -1;
}

Farm::~Farm()
{
// 	for(auto timestep : timed_transports)
// 	{
// 		for(auto transport : timestep.second)
// 		{
// 			delete transport;
// 		}
// 	}
// 
// 	for(auto timestep : timed_sl_transports)
// 	{
// 		for(auto transport : timestep.second)
// 		{
// 			delete transport;
// 		}
// 	}	
}

// std::string Farm::get_info() const
// { 
// 	char s [100];
// 	sprintf(s, "%d\t%d\t%d\t%d\t%d\t%d", id, 
// 										x_coordinate,
// 										y_coordinate,
// 										size,
// 										status,
// 										infected_at_timestep);
// 	
// 	return s;
// }
// 

void Farm::set_cellID(const double in_cellID)
{
	cellID = in_cellID;
}
// void Farm::set_status(const short in_status)
// {
// 	status = in_status;
// }
// 
// void Farm::set_time_of_infection(const int in_timestep)
// {	
// 	infected_at_timestep = in_timestep;
// }
// 
// void Farm::add_live_transport(Live_transport* T)
// {
// 	timed_transports[T->get_timestep()].emplace_back(T);
// }
// 
// void Farm::add_sl_transport(int const timestep, Slaughter_transport* sl_trp)
// {
// 	timed_sl_transports[timestep].emplace_back(sl_trp);
// }
// 
// std::vector<Slaughter_transport*>* Farm::get_sl_transports(int const timestep)
// {
// 	if(timed_sl_transports.find(timestep) != timed_sl_transports.end())
// 	{
// 		if(!timed_sl_transports[timestep].empty())
// 		{
// 			return &timed_sl_transports[timestep];
// 		}
// 	}
// 	return NULL;
// }