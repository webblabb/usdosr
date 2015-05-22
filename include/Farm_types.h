#ifndef FARM_TYPES_H
#define FARM_TYPES_H
#endif // FARM_TYPES_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "Alias_table.h"

class Farm_type
{
private:
    std::string herd;
    Alias_table<Farm_type*> prob_table;
    std::vector<Farm_type*> destinations;
    std::vector<double> weights;
    std::vector<double> prob_vector;

public:
    Farm_type(std::string herd);
    std::string get_herd();
    void add_weight(Farm_type* to_type, double weight);
    Farm_type* generate_destination_type();
    void print_me();
};

class Farm_types
{
private:
    int current_type_index = 0;
    std::vector<std::string> species;
    std::unordered_map<std::string, Farm_type*> type_map;
    std::unordered_map<std::string, int> assignment_map; //String is 1 for animal type exists, 0 for doesnt exist
    std::vector<std::string> existing_types;

    void make_type();
    std::string type_to_string(std::vector<std::string> from_vector);

public:
    Farm_types(std::vector<std::string> species);
    void add_weight(std::vector<std::string> from_vector,
                                           std::vector<std::string> to_vector,
                                           double weight);
    Farm_type* get_type(std::string herd_string);

};
