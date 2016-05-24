#include <iostream>
#include <vector>
#include <algorithm> // for std::max_element (susceptibility/infectiousness)

#include "grid_cell.h"
#include "Farm.h"

/// Constructed with cell dimensions and premises within. Calculates and stores maximum 
/// transmission values (for overestimating transmission probabilities).
grid_cell::grid_cell(const int in_id, const double in_x, const double in_y, 
	const double in_s, const std::vector<Farm*> in_farms)
	:
	id(in_id),
	x(in_x),
	y(in_y),
	s(in_s),
	farms(in_farms)
{
	// Add all farms' susceptibility and infectiousness to respective vectors and find max
	std::vector <double> allSus;
	std::vector <double> allInf;
	for (auto& f:in_farms){
		allSus.emplace_back(f->Farm::get_sus()); // add farm's susceptibility to vector
		allInf.emplace_back(f->Farm::get_inf()); // add farm's infectiousness to vector		
		}
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

///> Swaps contents of the empty member map for calculated kernel*susceptibility values
void grid_cell::take_KernelValues(std::unordered_map<int, double>& in_kern)
{
	susxKern.swap(in_kern);
}

void grid_cell::removeFarmSubset(std::vector<int>& toRemove)
{
	auto newEnd = std::remove_if(farms.begin(),farms.end(),farmIDpresent(toRemove));
	std::vector<Farm*> newFarms(farms.begin(),newEnd);
	farms.swap(newFarms);
}
