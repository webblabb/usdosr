#include <cmath>
#include <iostream>
#include "Region.h"

Region::Region(std::string id) :
    id(id), centroid(0.0, 0.0)
{
    set_initialized(is_set_id);
}

Region::Region(std::string id, double x, double y) :
    id(id), centroid(x, y)
{
    set_initialized(is_set_id);
    set_initialized(is_set_position);
}

Region::~Region() {}

double Region::distance_to(Region* target) const
{
    return measure_distance(&centroid, target->get_centroid());
}

double Region::measure_distance(const Point* p1, const Point* p2) const
{
    return std::sqrt(std::pow(p1->x - p2->x, 2) +
                     std::pow(p1->y - p2->y, 2));
}

void Region::set_position(double x, double y)
{
    centroid.x = x;
    centroid.y = y;
    set_initialized(is_set_position);
}

void Region::set_initialized(bool& parameter)
{
    parameter = true;
    all_initialized();
}

void Region::all_initialized()
{
    if(is_set_id and is_set_position)
    {
        region_initialized = true;
    }
}

void Region::not_initialized()
{
    std::cout << "Error: " << type << " " << id << " has not yet been completely initialized." << std::endl
              << "Exiting...";
    exit(EXIT_FAILURE);
}
