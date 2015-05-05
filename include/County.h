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
#include <Region.h>
#include <Farm.h>
#include <State.h>
#include <Alias_table.h>
#include <Shipment_kernel.h>

class County : public Region
{
public:
    County(double x, double y, std::string name);
    ~County();

    virtual void set_farms(const std::vector<Farm*>& in_farms);
    void add_farm(Farm* in_farm);
    void init_probabilities(std::vector<County*>& in_counties, Shipment_kernel& k); //Incomplete
    void set_area(double in_area);
    void set_parent_state(const State* target);

    std::string get_name() const; //Inlined
    double get_area() const; //Inlined
    const State* get_parent_state() const; //Inlined
    County* get_shipment_destination();

private:
    std::string name;
    double area;
    const State* parent_state;
    std::unordered_map <County*, double> county_distances;
    Alias_table<County*> county_probabilities;
    std::vector<Farm*> infected_farms;
    std::vector<Farm*> susceptible_farms;
    void set_initialized(bool& parameter);
    void not_initialized();

    bool is_set_area = false;
    bool is_set_state = false;
    bool is_set_shipment = false;
    bool is_initialized = false;
};

inline std::string County::get_name() const
{
    return name;
}

inline double County::get_area() const
{
    return area;
}

inline const State* County::get_parent_state() const
{
    return parent_state;
}

#endif // COUNTY_H
