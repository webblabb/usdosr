#include <County.h>

County::County(double x, double y, std::string name) :
    Region(x, y), name(name), area(0.0) {}

County::~County() {}

//Measures the distance to all counties and calculates probabilities to send to them.
//Arguments: a vector of all th counties, a pointer to a function describing the
//kernel and a function that calculates the distance between two point objects (pointers).
void County::init_probabilities(std::vector<County*>& in_counties,
                                Shipment_kernel& k)
{
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

void County::set_parent_state(const State* target)
{
    parent_state = target;
    set_initialized(is_set_state);
}

//Generates one shipment originating from this county.
County* County::get_shipment_destination()
{
    County* destination;
    if(is_initialized)
        destination = county_probabilities.generate();
    else
        not_initialized();
    return destination;
}

void County::set_initialized(bool& parameter)
{
    parameter = true;
    if(is_set_area and is_set_state and is_set_shipment)
        is_initialized = true;
}

void County::not_initialized()
{
    std::cout << "Error: county has not yet been completely initialized." << std::endl
              << "Exiting...";
    exit(EXIT_FAILURE);
}
