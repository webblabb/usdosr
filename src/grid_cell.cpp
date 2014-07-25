//
//  grid_cell.cpp
//
#include <iostream>
#include <vector>
#include <algorithm> // for maximum (susceptibility/infectiousness)

#include "grid_cell.h" // Point defined here

grid_cell::grid_cell(const double in_id, const double in_x, const double in_y, const double in_s, const std::vector<Farm*> in_farms)
{
    id = in_id;
    x = in_x;
    y = in_y;
    s = in_s;
    farms = in_farms;
    
    Point* LL = new Point(x, y);
	Point* LR = new Point(x, y+s);
	Point* UL = new Point(x+s, y);
	Point* UR = new Point(x+s, y+s);
	
	corners.emplace_back(LL);
	corners.emplace_back(LR);
	corners.emplace_back(UL);
	corners.emplace_back(UR);
		
// calculate maximum susceptibility/infectiousness for farms within cell, after Tildesley et al 2006 Nature Letters
// get farm sizes - to be replaced by vector of vectors
	std::vector <double> allSus;
	std::vector <double> allInf;
	for (auto f : in_farms){
		allSus.emplace_back(getFarmSus(f)); // get farm's susceptibility and add to vector
		allInf.emplace_back(getFarmInf(f)); // get farm's infectiousness and add to vector		
		}

    // max_element "returns an iterator pointing to the element with the largest value"
	maxSus = *std::max_element(allSus.begin(),allSus.end());
	maxInf = *std::max_element(allInf.begin(),allInf.end());
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

std::string grid_cell::to_string() const // overloaded to_string function, makes tab-delim string specifically for cell
{
	std::string toPrint;
	char temp[20];
	std::vector<double> vars;
		vars.resize(5);
		vars[0] = id;
		vars[1] = x;
		vars[2] = y;
		vars[3] = s;
		vars[4] = farms.size();

	for(auto it = vars.begin(); it != vars.end(); it++)
	{
		sprintf(temp, "%f\t", *it);
		toPrint += temp;
	}
	
	toPrint.replace(toPrint.end()-1, toPrint.end(), "\n");
	
	return toPrint;
}