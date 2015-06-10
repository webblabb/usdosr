#include "State.h"
#include "Farm.h"
#include "County.h"
#include "shared_functions.h"

State::State(std::string id, int state_code) :
    Region(id),
    state_code(state_code),
    generator(generate_distribution_seed())
{
    type = "state";
}

State::State(std::string id, double x, double y, int state_code) :
    Region(id, x, y),
    state_code(state_code),
    generator(generate_distribution_seed())
{
    type = "state";
}

State::~State() {}

void State::add_county(County* in_county)
{
    bool already_present = false;
    for(County* existing_county : member_counties)
    {
        if(existing_county == in_county)
        {
            already_present = true;
        }
    }

    if(!already_present)
    {
        member_counties.push_back(in_county);
    }
}

void State::init_poisson()
{
    for(auto ft_vol_pair : ship_volume_map)
    {
        Farm_type* current_ft = ft_vol_pair.first;
        int n_farms = 0;
        for(County* current_county : member_counties)
        {
            n_farms += current_county->get_n_farms(current_ft);
        }

        if(n_farms != 0)
        {
            poisson_mean[current_ft] = ship_volume_map[current_ft] / n_farms;
        }
        poisson_map[current_ft] = new std::poisson_distribution<int>(poisson_mean[current_ft]);
        //std::cout << "Mean for " << id << " is " << poisson_mean << ", shipments_per_t: " << shipments_per_t << ", n_farms: " << n_farms << std::endl;
    }
    set_initialized(is_set_poisson);
}

void State::set_a(double in_a, Farm_type* in_type)
{
    a_map[in_type] = in_a;
}

void State::set_b(double in_b, Farm_type* in_type)
{
    b_map[in_type] = in_b;
}

void State::set_shipment_volume(double in_volume, Farm_type* in_type)
{
    ship_volume_map[in_type] = in_volume / 365;
}

int State::get_code()
{
    return state_code;
}

double State::get_a(Farm_type* ft)
{
    return a_map[ft];
}

double State::get_b(Farm_type* ft)
{
    return b_map[ft];
}

int State::get_poisson_shipments(Farm_type* ft)
{
    if(!state_initialized)
        not_initialized();

//    std::cout << "Attempting to generate shipments from a farm in " << id <<
//                 ". Shipment volume for " << ft->get_species() << " is " <<
//                 ship_volume_map[ft] << ". Mean of poisson d. is " <<
//                 poisson_mean[ft] << "." << std::endl;
    return (*poisson_map[ft])(generator);
}

void State::set_initialized(bool& parameter)
{
    parameter = true;
    all_initialized();
}

void State::all_initialized()
{
    if(is_set_id and is_set_poisson)
    {
        state_initialized = true;
    }
}
