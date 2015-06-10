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

int Farm::get_n_shipments() const
{
    State* s = parent_county->get_parent_state();
    return s->get_poisson_shipments(farm_type);
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

void Farm::set_farm_type(Farm_type* in_type)
{
    farm_type = in_type;
}

void Farm::set_speciesCount(const std::string species, int sp_count)
{
	speciesCounts[species] = sp_count;
}

void Farm::set_sus(const double in_sus)
{
	sus = in_sus;
}

void Farm::set_inf(const double in_inf)
{
	inf = in_inf;
}

void Farm::set_control_status(const std::string s, const int i)
{
	statuses[s] = i;
}

void Farm::set_parent_county(County* in_county)
{
    parent_county = in_county;
}

State* Farm::get_parent_state() const
{
    return parent_county->get_parent_state();
}

Farm_type::Farm_type(int index, std::string herd, std::vector<std::string> in_species) :
    index(index), herd(herd)
{
    for(size_t i = 0; i < herd.size(); i++)
    {
        if(herd[i] != '0')
        {
            species = in_species[i];
            break;
        }
    }
}

Farm_type::~Farm_type()
{
}
