#include <stdio.h>
#include <iostream>
#include "Farm.h"
#include "County.h"
#include "State.h"

Farm::Farm(int in_id, double in_x, double in_y, std::string in_fips)
	:
	id(in_id),
	cellID(-1),
	x_coordinate(in_x),
	y_coordinate(in_y),
	position(in_x, in_y),
	fips(in_fips)
{
}

Farm::~Farm()
{
}

int Farm::get_size(const std::string species) const
{
	int count = 0;
	if (speciesCounts.count(species) != 0){count = speciesCounts.at(species);}

	return count;
}

void Farm::set_cellID(const int in_cellID)
{
	cellID = in_cellID;
}

void Farm::set_speciesCount(const std::string species, int count)
{
	speciesCounts[species] = count;
}

void Farm::set_sus(const double in_sus)
{
	sus = in_sus;
}

void Farm::set_inf(const double in_inf)
{
	inf = in_inf;
}

void Farm::set_status(const std::string s, const int i)
{
	statuses[s] = i;
}

void Farm::set_parent_county(County* in_county)
{
    parent_county = in_county;
}

void Farm::set_parent_state(State* in_state)
{
    parent_state = in_state;
}
