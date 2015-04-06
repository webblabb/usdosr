//
//  grid_cell.cpp
//
#include <iostream>
#include <vector>
#include <algorithm> // for std::max_element (susceptibility/infectiousness)

#include "grid_cell.h"

grid_cell::grid_cell(const int in_id, const double in_x, const double in_y, 
	const double in_s, const std::vector<Farm*> in_farms)
	:
	id(in_id),
	x(in_x),
	y(in_y),
	s(in_s),
	farms(in_farms)
{
// calculate maximum susceptibility/infectiousness for farms within cell
	std::vector <double> allSus;
	std::vector <double> allInf;
	for (auto& f:in_farms){
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

void grid_cell::addNeighbor(grid_cell* in_neighbor)
{
	neighbors.emplace_back(in_neighbor);
}

void grid_cell::take_KernelValues(std::unordered_map<int, double>& in_kern)
{
	susxKern.swap(in_kern);
}

void grid_cell::removeFarmSubset(std::vector<Farm*>& toRemove)
{
	auto newEnd = std::remove_if(farms.begin(),farms.end(),isInList<Farm*>(toRemove));
	std::vector<Farm*> newFarms(farms.begin(),newEnd);
	farms.swap(newFarms);
}
