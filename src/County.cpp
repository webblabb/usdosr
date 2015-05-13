#include "County.h"
#include "Farm.h"
#include "State.h"
#include "Shipment_kernel.h"

County::County(std::string id) :
    Region(id), area(0.0)
{
    type = "county";
}

County::County(std::string id, double x, double y) :
    Region(id, x, y), area(0.0)
{
    type = "county";
}

County::~County() {}

//Measures the distance to all counties and calculates probabilities to send to them.
//Arguments: a vector of all th counties, a pointer to a function describing the
//kernel and a function that calculates the distance between two point objects (pointers).
void County::init_probabilities(std::vector<County*>& in_counties,
                                Shipment_kernel& k)
{
    if(!county_initialized)
    {
        std::cout << "Make sure all of the following are set before attempting to "
                  << "initialize shipment probabilities." << std::endl;
        print_bools();
        not_initialized();
    }

    std::vector<double> probabilities;
    probabilities.reserve(in_counties.size());
    double normalization_sum = 0.0;

    //Get all kernel values and keep track of the total for use when normalizing.
    for(auto c : in_counties)
    {
        double kernel_value = 0.0;
        kernel_value = k.kernel(this, c);
        probabilities.push_back(kernel_value);
        normalization_sum += kernel_value;
    }

    //Normalize probabilities.
    for(auto it = probabilities.begin(); it != probabilities.end(); it++)
    {
        *it = *it / normalization_sum;
    }

    //Insert probabilities and outcomes into the alias table.
    county_probabilities.init(in_counties, probabilities);
    set_initialized(is_set_shipment);

}

//Sets the farms that belong to this county by passing a
//vector of pointers to them
void County::set_farms(const std::vector<Farm*>& in_farms)
{
    member_farms = in_farms;
    for(auto farm : member_farms)
    {
        farm->set_parent_county(this);
    }
}

//Adds one single farm that belongs to this county by passing a pointer to it.
void County::add_farm(Farm* in_farm)
{
    member_farms.push_back(in_farm);
    in_farm->set_parent_county(this);
}

void County::set_area(double in_area)
{
    area = in_area;
    set_initialized(is_set_area);
}

void County::set_parent_state(State* target)
{
    parent_state = target;
    set_initialized(is_set_state);
}

void County::set_control_status(std::string status, int level)
{
    statuses[status] = level;
}

//Generates one shipment originating from this county.
County* County::get_shipment_destination()
{
    County* destination;
    if(is_set_shipment)
        destination = county_probabilities.generate();
    else
    {
        std::cout << "Run init_probabilities() before generating shipments."
                  << std::endl;
        not_initialized();
    }

    return destination;
}

void County::print_bools()
{
	std::cout << "For " << id << std::endl;
	std::cout << "id = " << is_set_id << std::endl;
	std::cout << "position = " << is_set_position << std::endl;
	std::cout << "area = " << is_set_area << std::endl;
	std::cout << "state = " << is_set_state << std::endl;
	std::cout << "region = " << region_initialized << std::endl;
}

void County::calculate_centroid()
{
    if(member_farms.size() > 0)
    {
        double x_mean, y_mean;
        double x_sum = 0.0;
        double y_sum = 0.0;

        for(Farm* f : member_farms)
        {
            x_sum += f->get_x();
            y_sum += f->get_y();
        }

        x_mean = x_sum / member_farms.size();
        y_mean = y_sum / member_farms.size();
        this->set_position(x_mean, y_mean);
    }
    else
    {
        std::cout << "Error: The centroid of county " << get_id()
                  << "cannot be estimated because there are no farms in the county."
                  << " Exiting..." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void County::set_initialized(bool& parameter)
{
    parameter = true;
    all_initialized();
}

void County::all_initialized()
{
    Region::all_initialized();
//    if(is_set_area and is_set_state and
//       is_set_id and region_initialized)
        if(is_set_area and
           is_set_id and region_initialized)
    {
        county_initialized = true;
    }
}


