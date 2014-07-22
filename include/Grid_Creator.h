//
//  Grid_Creator.h
// 
//  Creates a vector of grid_cell objects, determined by local farm density
//

#ifndef Grid_Creator_h
#define Grid_Creator_h

#include "grid_cell.h"
#include "farm.h"
#include "shared_functions.h"

#include <vector>
#include <stack>
#include <unordered_map>

class Grid_Creator
{
	private:
		std::vector<grid_cell*> allCells; // vector of cells in grid
		std::unordered_map<int, Farm*> farm_map; // Contains all farm objects. Id as key.
 		std::vector<Farm*> farmList; // vector of pointers to all farms (shortened as grid is created)
		std::vector<double> xylimits; // [0]x min, [1]x max, [2]y min, [3]y max
		bool verbose; // if true, outputs processing details
		std::unordered_map<double, std::unordered_map<double, double>> cellDists; // distances between cell pairs
		std::unordered_map<double, std::unordered_map<double, double>> gridCellKernel; // kernel values between cell pairs
		
		void setVerbose(bool); // inlined
		// functions for grid creation
		void updateFarmList(std::vector<Farm*>& farmsInCell); // removes farms from recently committed cell from main list
		std::vector<Farm*> getFarms(std::vector<double> cellSpecs,const unsigned int maxFarms); // makes list of farms in a cell (quits early if over max)
		void removeParent(std::stack< std::vector<double> >& queue);// removes 1st vector in queue
		void addOffspring(std::vector<double> cellSpecs, std::stack< std::vector<double> >& queue); // adds subdivided cells to queue
		void commitCell(std::vector<double> cellSpecs, std::vector<Farm*>& farmsInCell); // adds cell as type GridCell to set allCells
		void splitCell(std::vector<double> cellSpecs, std::stack< std::vector<double> >& queue); // replaces parent cell with subdivided offspring quadrants
 		// functions for calculating reference distance/kernel matrices from grid
		double shortestCellDist(grid_cell* cell1, grid_cell* cell2) const; // calculates shortest distance between two cells
		void makeCellRefs(); // make reference matrices for distance and kernel
		
	public:
		Grid_Creator(std::string &fname, bool v);
		~Grid_Creator();
		void initiateGrid(const unsigned int, const int); // main function that splits/commits cells
		void printCells() const;
		std::vector<grid_cell*> get_allCells() const; //inlined
		std::unordered_map<double, std::unordered_map<double, double>> get_cellDists() const; //inlined
		std::unordered_map<double, std::unordered_map<double, double>> get_gridCellKernel() const; // inlined
		std::unordered_map<int, Farm*> get_allFarms() const; //inlined
};

inline bool sortByX(const Farm* farm1, const Farm* farm2)
// "compare" function to sort farms by x-coordinate
// must be defined outside of class, or else sort doesn't work
{
	return (farm1 -> get_x()) < (farm2 -> get_x());
}

inline void Grid_Creator::setVerbose(bool v)
{
	verbose = v;
}

inline std::vector<grid_cell*> Grid_Creator::get_allCells() const
{
	return (allCells);
}

inline std::unordered_map<double, std::unordered_map<double, double>> Grid_Creator::get_cellDists() const
{
	return (cellDists);
}

inline std::unordered_map<double, std::unordered_map<double, double>> Grid_Creator::get_gridCellKernel() const
{
	return(gridCellKernel);
}

inline std::unordered_map<int, Farm*> Grid_Creator::get_allFarms() const
{
	return(farm_map);
}

#endif
