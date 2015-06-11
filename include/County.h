/*
A county. Inherits from Region.
Call init_probabilities(counties, k) to generate shipment probabilities,
where the argument counties is a vector containing pointers to all
counties (including self) and k is the shipment kernel object to use for
the probabilities. This function will populate an alias table within the
county, which is then used by the function get_shipment_destination() to
generate shipments. Each call to this function returns a destination county
for one shipment.

To create a complete county:
    (1) Construct with x,y and name.
    (2) Set the area of the county with set_area(double). This is important,
        otherwise the shipment probabilities will be wrong.
    (3) Call set_parent_state(State*) to set what state the county is in.
    (4) Add farms as pointers with add_farm or
        as a vector of pointers with set_farm.
    (5) When all counties have been created, call init_probabilities as above.
*/

#ifndef COUNTY_H
#define COUNTY_H

#include <string>
#include <unordered_map>
#include "Region.h"
#include "Alias_table.h"


class Farm;
class Farm_type;
class State;
class Shipment_kernel;

class County : public Region
{
public:
    County(std::string id);
    County(std::string id, double x, double y);
    ~County();

    void set_farms(const std::vector<Farm*>& in_farms);
    void add_farm(Farm* in_farm);
    void init_probabilities(std::vector<County*>& in_counties); //Incomplete
    void set_area(double in_area);
    void set_weights(std::vector<double> in_weights);
    void set_parent_state(State* target);
    void set_control_status(std::string status, int level);
    void set_all_counties(std::vector<County*> in_counties);

    double get_area(); //Inlined
    size_t get_n_farms(); //Inlined
    size_t get_n_farms(Farm_type* ft); //Inlined
    const std::vector<Farm*>& get_farms(); //Inlined
    std::vector<Farm*>& get_farms(Farm_type* ft);
    std::unordered_map<std::string, int> get_statuses(); //Inlined
    int get_control_status(std::string status); //Inlined
    State* get_parent_state(); //Inlined
    County* get_shipment_destination(Farm_type* ft);
    double get_weight(Farm_type* in_type);
    //std::vector<Farm*>* get_farms_of_type(Farm_type* in_farm_type);

    void calculate_centroid();
    void print_bools();
    //bool is_present(Farm_type* farm_type);

private:
    double area;
    State* parent_state;
    std::vector<Farm*> member_farms;
    std::vector<double> weights;
    std::unordered_map<Farm_type*, std::vector<Farm*>> farms_by_type;
    //std::vector<Farm_type*> present_farm_types;
    std::unordered_map <County*, double> county_distances;
    std::unordered_map<std::string, int> statuses; // for control type and level
    std::vector<Shipment_kernel*> shipment_kernels; //By farm type index.
    std::vector<Alias_table<County*>> county_probabilities; //By farm type index.
    std::vector<Farm*> infected_farms;
    std::vector<Farm*> susceptible_farms;
    std::vector<County*> all_counties;

    bool county_initialized = false;
    bool is_set_area = false;
    bool is_set_state = false;
    bool is_set_shipment = false;
    bool is_set_weights = false;

    virtual void set_initialized(bool& parameter);
    virtual void all_initialized();
};

inline double County::get_area()
{
    if(!is_set_area)
        not_initialized();

    return area;
}

inline size_t County::get_n_farms()
{
    return member_farms.size();
}

inline size_t County::get_n_farms(Farm_type* ft)
{
    return farms_by_type[ft].size();
}

inline const std::vector<Farm*>& County::get_farms()
{
    if(!county_initialized)
        not_initialized();

    return member_farms;
}

inline State* County::get_parent_state()
{
    if(!county_initialized)
        not_initialized();

    return parent_state;
}

inline std::unordered_map<std::string, int> County::get_statuses()
{
    if(!county_initialized)
        not_initialized();

    return statuses;
}

inline int County::get_control_status(std::string status)
{
    if(!county_initialized)
        not_initialized();

    if(statuses.find(status) == statuses.end())
        return 0;
    else
        return statuses.at(status);
}
#endif // COUNTY_H
