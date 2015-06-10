/* A state, inherits from region */

#ifndef STATE_H
#define STATE_H

#include <string>
#include <random>
#include <unordered_map>
#include "Region.h"

class Farm;
class County;
class Farm_type;

class State : public Region
{
public:
    State(std::string id, int state_code);
    State(std::string name, double x, double y, int state_code);
    ~State();

    void add_county(County* in_county);
    void init_poisson();
    void set_a(double in_a, Farm_type* in_type);
    void set_b(double in_b, Farm_type* in_type);
    void set_shipment_volume(double in_volume, Farm_type* in_type);

    int get_code();
    double get_a(Farm_type* ft);
    double get_b(Farm_type* ft);
    int get_poisson_shipments(Farm_type* ft);
    double get_flow(); //Inlined
    int get_n_counties(); //inlined

private:
    int state_code;
    std::mt19937 generator;
    std::unordered_map<Farm_type*, double> poisson_mean;
    std::vector<County*> member_counties;
    bool state_initialized = false;
    bool is_set_poisson = false;
    std::unordered_map<Farm_type*, double> a_map;
    std::unordered_map<Farm_type*, double> b_map;
    std::unordered_map<Farm_type*, double> ship_volume_map;
    std::unordered_map<Farm_type*, std::poisson_distribution<int>*> poisson_map;


    virtual void set_initialized(bool& parameter);
    virtual void all_initialized();
};

inline int State::get_n_counties()
{
    return int(member_counties.size());
}

#endif // STATE_H
