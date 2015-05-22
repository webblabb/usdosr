#include <iostream>
#include "Farm_types.h"


Farm_type::Farm_type(std::string herd) :
    herd(herd)
{
    //A Farm_type object is characterized by a number that corresponds to
    //set of strings describing what species is on the farm. Which number
    //corresponds to which set can be found in the Farm_types object.
    //the herd argument is a vector where each element correspond to
    //the number of animal of each type.
    //It also has a bunch of numbers (corresponding to similar sets) that
    //describe what other types this farm type sends to and with what
    //probability. The farm type can be asked to generate a destination
    //from these probabilities with the method generate_destination_type().
}

std::string Farm_type::get_herd()
{
    return herd;
}

void Farm_type::add_weight(Farm_type* to_type, double weight)
{
    bool is_found = false;
    for(size_t i = 0; i < destinations.size(); i++)
    {
        if(destinations[i]->get_herd() == to_type->get_herd())
        {
            is_found = true;

            if(weights[i] != weight)
            {
                std::cout << "ERROR: One of the origin-destination pairs specified in the "
                          << "species shipment weights file appears more than once with "
                          << "different weights." << std::endl << "Exiting..." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }

    if(!is_found)
    {
        destinations.push_back(to_type);
        weights.push_back(weight);
    }

    prob_vector.clear();
    double w_sum = 0.0;
    for(double w : weights)
        w_sum += w;
    for(double w : weights)
        prob_vector.push_back(w/w_sum);

    prob_table.clear();
    prob_table.init(destinations, prob_vector);
}

Farm_type* Farm_type::generate_destination_type()
{
    return prob_table.generate();
}

void Farm_type::print_me()
{
    std::cout << "Printing details for " << herd << std::endl;
    std::cout << "\tDestinations: ";
    for(auto d : destinations)
        std::cout << d->get_herd() << ", ";
    std::cout << std::endl << "\tWeights: ";
    for(auto w : weights)
        std::cout << w << ", ";
    std::cout << std::endl << "\tProbabilities: ";
    for(auto p : prob_vector)
        std::cout << p << ", ";
    std::cout << std::endl;
}

Farm_types::Farm_types(std::vector<std::string> species) :
    species(species)
{
//    std::cout << "Farm_types has species ";
//    for(auto i : species)
//        std::cout << i << ", ";
//    std::cout << std::endl;

}

void Farm_types::add_weight(std::vector<std::string> from_vector,
                                           std::vector<std::string> to_vector,
                                           double weight)
{
    //std::unordered_set<std::string> this_type(from_vector.begin(), from_vector.end());
    std::string from_as_string = type_to_string(from_vector);
    std::string to_as_string = type_to_string(to_vector);

    //Check if it already exists:
    bool from_already_exists = false;
    bool to_already_exists = false;
    for(auto t : existing_types)
    {
        if(t == from_as_string)
            from_already_exists = true;
        if(t == to_as_string)
            to_already_exists = true;
    }

    //Check if the origin farm type already exists
    if(!from_already_exists)
    {
        //Create a new farm_type
        Farm_type* ft = new Farm_type(from_as_string);
        type_map[from_as_string] = ft;
        current_type_index += 1;
        //Insert this type into existing_types
        existing_types.push_back(from_as_string);
    }

    //Check if the destination farm type already exists
    if(!to_already_exists)
    {
        //Create a new farm_type
        Farm_type* ft = new Farm_type(to_as_string);
        type_map[to_as_string] = ft;
        current_type_index += 1;
        //Insert this type into existing_types
        existing_types.push_back(to_as_string);
    }

    type_map[from_as_string]->add_weight(type_map[to_as_string], weight);

//    std::cout << "After adding one new type, types are now: " << std::endl;
//    for(auto t : type_map)
//    {
//        std::cout << "\t" << t.first << ": ";
//        t.second->print_me();
//    }
}

std::string Farm_types::type_to_string(std::vector<std::string> from_vector)
{
    std::string result;
    for(std::string s : species) //s is a species of all the species in the data
    {
        bool this_species_present = false;
        for(std::string s2 : from_vector) //s2 is a species that exists on this farm type
        {
            if(s == s2)
                this_species_present = true;
        }
        if(this_species_present)
            result += '1';
        else
            result += '0';
    }
    if(result.size() != species.size())
    {
        std::cout << "SOMETHING WRONG IN TYPE_TO_STRING" << std::endl;
        exit(EXIT_FAILURE);
    }

    return result;
}

Farm_type* Farm_types::get_type(std::string herd_string)
{
    return type_map[herd_string];
}
