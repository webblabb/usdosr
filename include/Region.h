/* Base class for county, state and grid cells. */

#ifndef REGION_H
#define REGION_H

#include <vector>
#include <Point.h>

class Farm;

class Region
{
public:
    Region(double x, double y);
    ~Region();
    virtual void set_farms(const std::vector<Farm*>& in_farms) = 0;
    virtual void add_farm(Farm* in_farm) = 0;
    double distance_to(const Region* target) const;
    double measure_distance(const Point* p1, const Point* p2) const;
    const std::vector<Farm*>* get_farms() const; //Inlined
    const Point* get_centroid() const; //Inlined

protected:
    Point centroid;
    std::vector<Farm*> member_farms;
};

inline const std::vector<Farm*>* Region::get_farms() const
{
    return &member_farms;
}

inline const Point* Region::get_centroid() const
{
    return &centroid;
}
#endif // REGION_H
