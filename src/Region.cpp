#include <cmath>
#include <Region.h>

Region::Region(double x, double y) :
    centroid(x, y) {}

Region::~Region() {}

double Region::distance_to(const Region* target) const
{
    return measure_distance(&centroid, target->get_centroid());
}

double Region::measure_distance(const Point* p1, const Point* p2) const
{
    return std::sqrt(std::pow(p1->x - p2->x, 2) +
                     std::pow(p1->y - p2->y, 2));
}
