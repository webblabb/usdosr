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

County::~County()
{
    for(Shipment_kernel* k : shipment_kernels)
    {
        delete k;
    }
}

//Measures the distance to all counties and calculates probabilities to send to them.
//Arguments: a vector of all th counties, a pointer to a function describing the
//kernel and a function that calculates the distance between two point objects (pointers).
void County::init_probabilities(std::vector<County*>& in_counties)
{
    if(!county_initialized)
    {
        std::cout << "Make sure all of the following are set before attempting to "
                  << "initialize shipment probabilities." << std::endl;
        print_bools();
        not_initialized();
    }

    county_probabilities.resize(farms_by_type.size());
    shipment_kernels.resize(farms_by_type.size());

    for(auto ft_vec_pair : farms_by_type)
    {
        Farm_type* current_ft = ft_vec_pair.first;
        std::vector<double> probabilities;
        probabilities.reserve(in_counties.size());
        Shipment_kernel* k = new Shipment_kernel(this->get_parent_state()->get_a(current_ft),
                                                 this->get_parent_state()->get_b(current_ft),
                                                 "linear", true);
        double normalization_sum = 0.0;

        //Get all kernel values and keep track of the total for use when normalizing.
        for(auto c : in_counties)
        {
            //std::cout << "Doing county " << c->get_id() << std::endl;
            double kernel_value = 0.0;
            //Kernel value * Flow of state of 'origin' county * number of farms in 'origin' county.
            kernel_value = k->kernel(this, c) * c->get_weight(current_ft);
            probabilities.push_back(kernel_value);
            normalization_sum += kernel_value;
            //std::cout << "Calc. probability of sending from " << this->get_id() << " to " << c->get_id() << ". Weight is " << c->get_weight(current_ft) << std::endl;
        }

        //Normalize probabilities.
        for(auto it = probabilities.begin(); it != probabilities.end(); it++)
        {
            *it = *it / normalization_sum;
        }

        //Insert probabilities and outcomes into the alias table.
        county_probabilities[current_ft->get_index()] = Alias_table<County*>(in_counties, probabilities);
        shipment_kernels[current_ft->get_index()] = k; //Save for if needed later.
    }

    set_initialized(is_set_shipment);
}

//Sets the farms that belong to this county by passing a
//vector of pointers to them
void County::set_farms(const std::vector<Farm*>& in_farms)
{
    for(Farm* in_farm : in_farms)
    {
        this->add_farm(in_farm);
    }
}

//Adds one single farm that belongs to this county by passing a pointer to it.
//Also called internally by set_farms
void County::add_farm(Farm* in_farm)
{
    member_farms.push_back(in_farm);
    farms_by_type[in_farm->get_farm_type()].push_back(in_farm);
    in_farm->set_parent_county(this);
}

void County::set_area(double in_area)
{
    area = in_area;
    set_initialized(is_set_area);
}

void County::set_weights(std::vector<double> in_weights)
{
    weights = in_weights;
    set_initialized(is_set_weights);
}

void County::set_parent_state(State* target)
{
    parent_state = target;
    target->add_county(this);
    set_initialized(is_set_state);
}

void County::set_control_status(std::string status, int level)
{
    statuses[status] = level;
}

void County::set_all_counties(std::vector<County*> in_counties)
{
    all_counties = in_counties;
}

std::vector<Farm*>& County::get_farms(Farm_type* ft)
{
    return farms_by_type[ft];
}

//Generates one shipment originating from this county.
County* County::get_shipment_destination(Farm_type* ft)
{
    County* destination = nullptr;
    if(!is_set_shipment) //If the shipping prob has not been created for this county before
    {
        std::cout << "The shipping probabilities of " << id << " has not yet been set. Calculating..." << std::endl;
        std::clock_t shipping_start = std::clock();
        this->init_probabilities(all_counties);
        std::cout << "Probabilities calculated in " <<
              1000.0 * (std::clock() - shipping_start) / CLOCKS_PER_SEC <<
              "ms." << std::endl;
    }

    if(farms_by_type.find(ft) == farms_by_type.end())
    {
        std::cout << "There are no farms of type " << ft->get_species() <<
                     " in county " << id << "." << std::endl;
        exit(EXIT_FAILURE);
    }
    destination = county_probabilities[ft->get_index()].generate();

    return destination;
}

double County::get_weight(Farm_type* in_type)
{
    unsigned int index = in_type->get_index();
    if(!is_set_weights)
    {
        std::cout << "The weights for county " << this->get_id() <<
                  " has not been set. Setting them to 0..." << std::endl;

        std::vector<double> temp_weights;
        for(size_t i = 0; i < farms_by_type.size(); i++)
        {
            temp_weights.push_back(0.0);
        }
        set_weights(temp_weights);
    }

    if(index > weights.size())
    {
        std::cout << "The index " << index << " is not present in " <<
                     this->get_id() << ". Exiting..." << std::endl;
        exit(EXIT_FAILURE);
    }
    return weights[index];
}

//std::vector<Farm*>* County::get_farms_of_type(Farm_type* in_farm_type)
//{
//    std::string type_as_string = in_farm_type->get_herd();
//    try
//    {
//        return &(farms_by_type.at(type_as_string));
//    }
//    catch(std::exception& e)
//    {
//        std::cout << "No farm of type " << in_farm_type->get_herd() << " exists in " <<
//                     id << "." << e.what() << std::endl;
//        exit(EXIT_FAILURE);
//    }
//    return nullptr;
//}

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

//bool County::is_present(Farm_type* farm_type)
//{
//    bool present = false;
//    for(Farm_type* ft_to_check : present_farm_types)
//    {
//        if(farm_type == ft_to_check)
//            present = true;
//    }
//    return present;
//}

