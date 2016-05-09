/* Base class for county, state and grid cells. */

#ifndef REGION_H
#define REGION_H

#include <vector>
#include <string>
#include "Point.h"

class Farm;

class Region
{
public:
    Region(std::string id);
    Region(std::string id, double x, double y);
    virtual ~Region();
    //virtual void set_farms(const std::vector<Farm*>& in_farms) = 0;
    //virtual void add_farm(Farm* in_farm) = 0;
    double distance_to(Region* target) const;
    double measure_distance(const Point* p1, const Point* p2) const;

    void set_position(double x, double y);

    std::string get_id(); //Inlined
    //const std::vector<Farm*>& get_farms(); //Inlined
    const Point* get_centroid(); //Inlined

protected:
    std::string type = "region/unchanged";
    std::string id = "unknown";
    Point centroid;
    bool is_set_id = false;
    bool is_set_position = false;
    bool region_initialized = false;

    virtual void set_initialized(bool& parameter);
    virtual void all_initialized(); //If all parameters that need to be initialized
                                    //have been init., set x_initialized to true.
    void not_initialized();
};

inline std::string Region::get_id()
{
    if(!region_initialized)
        not_initialized();

    return id;
}

//inline const std::vector<Farm*>& Region::get_farms()
//{
//    if(!region_initialized)
//        not_initialized();
//
//    return member_farms;
//}

inline const Point* Region::get_centroid()
{
    if(!region_initialized)
        not_initialized();

    return &centroid;
}
#endif // REGION_H
