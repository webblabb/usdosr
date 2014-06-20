//
//  grid_cell.cpp
//
//

#include <vector>
#include <algorithm> // for maximum (susceptibility/infectiousness)
#include "grid_cell.h"
#include "farm.h" // for class Farm

grid_cell::grid_cell(const double in_x, const double in_y, const double in_s, const std::vector<Farm*> in_farms)
{
    x = in_x;
    y = in_y;
    s = in_s;
    farms = in_farms;
    
    Point* LL = new Point(x,y);
	Point* LR = new Point(x,y+s);
	Point* UL = new Point(x+s,y);
	Point* UR = new Point(x+s,y+s);
	
	std::vector<Point *> corners;	
		corners.emplace_back(LL);
		corners.emplace_back(LR);
		corners.emplace_back(UL);
		corners.emplace_back(UR);
		
	// calculate maximum susceptibility/infectiousness for farms within cell
	// get farm sizes
	std::vector <short> allSizes;
	for (auto f : in_farms){
		allSizes.emplace_back(f -> get_size());
		}
	// this vector calculates/holds the susceptibility values for all farms (right now just copies/converts size value)
	std::vector <double> allSus = double(allSizes);
	// this vector calculates/holds the infectiousness values for all farms
	std::vector <double> allInf = double(allSizes);

    // max_element "returns an iterator pointing to the element with the largest value"
	double maxSus = *std::max_element(allSus.begin(),allSus.end());
	double maxInf = *std::max_element(allInf.begin(),allInf.end());
}

grid_cell::~grid_cell()
{
	for (auto f:farms)
	{
		delete f;
	}
	for (auto c:corners)
	{
		delete c;
	}
}