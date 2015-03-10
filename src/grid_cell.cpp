//
//  grid_cell.cpp
//
#include <iostream>
#include <vector>
#include <algorithm> // for maximum (susceptibility/infectiousness)

#include "grid_cell.h" // Point defined here

grid_cell::grid_cell(const int in_id, const double in_x, const double in_y, const double in_s, const std::vector<Farm*> in_farms)
{
    id = in_id;
    x = in_x;
    y = in_y;
    s = in_s;
    farms = in_farms;
    
//     Point* LL = new Point(x, y);
// 	Point* LR = new Point(x, y+s);
// 	Point* UL = new Point(x+s, y);
// 	Point* UR = new Point(x+s, y+s);
	
// 	corners.emplace_back(LL);
// 	corners.emplace_back(LR);
// 	corners.emplace_back(UL);
// 	corners.emplace_back(UR);
		
// calculate maximum susceptibility/infectiousness for farms within cell
	std::vector <double> allSus;
	std::vector <double> allInf;
	for (auto f : in_farms){
		allSus.emplace_back(f->Farm::get_sus()); // get farm's susceptibility and add to vector
		allInf.emplace_back(f->Farm::get_inf()); // get farm's infectiousness and add to vector		
		}

    // max_element "returns an iterator pointing to the element with the largest value"
	maxSus = *std::max_element(allSus.begin(),allSus.end());
	maxInf = *std::max_element(allInf.begin(),allInf.end());
	}

grid_cell::~grid_cell()
{
}
