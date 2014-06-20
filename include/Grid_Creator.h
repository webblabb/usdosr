//
//  Grid_Creator.h
// 
//  Creates a vector of grid_cell objects, determined by local farm density
//

#ifndef Grid_Creator_h
#define Grid_Creator_h

#include "grid_cell.h"
#include "farm.h"

#include <vector>
#include <stack>
#include <unordered_map>

class Grid_Creator
{
	private:
// 		std::vector<grid_cell> allCells; // vector of cells in grid
		std::unordered_map<int, Farm*> farm_map; // Contains all farm objects. Id as key.
// 		std::vector<Farm*> farmList; // vector of pointers to all farms (shortened as grid is created)
		std::vector<int> xylimits; // [0]x min, [1]x max, [2]y min, [3]y max
		bool verbose; // if true, outputs processing details
// 		std::vector <std::vector <double> cellDists; // distances between cell pairs
// 		std::vector <std::vector <double> gridCellKernel; // kernel values for cellDists
		
		void setVerbose(bool v);
		
		// functions for grid creation
// 		void updateFarmList(std::vector<Farm*>& farmsInCell); // removes farms from recently committed cell from main list
// 		void getFarms(std::vector<double> cellSpecs,std::vector<Farm*>& farmsInCell); // makes list of farms in a cell
// 		void removeParent(std::stack< std::vector<double> >& queue);// removes 1st vector in queue
// 		void addOffspring(std::vector<double> cellSpecs, std::stack< std::vector<double> >& queue); // adds subdivided cells to queue
// 		void commitCell(std::vector<double> cellSpecs, std::vector<Farm*>& farmsInCell); // adds cell as type GridCell to set allCells
// 		void splitCell(std::vector<double> cellSpecs, std::stack< std::vector<double> >& queue); // replaces parent cell with subdivided offspring quadrants
// 
// 		// functions for calculating reference distance/kernel matrices from grid
// 		double shortestCellDist(grid_cell* cell1, grid_cell* cell2); // calculates shortest distance between two cells
// 		double gridKernel(double dist); // returns kernel value for a given distance
// 		void makeCellRefs(); // make reference matrices for distance and kernel
		
	public:
		Grid_Creator(std::string &fname, bool v);
		~Grid_Creator();
// 		void initiateGrid(const std::vector<int>, const unsigned int, const int, std::vector<Farm*>); // main function that splits/commits cells
};

// bool sortByX(const Farm* farm1, const Farm* farm2)
// // "compare" function to sort farms by x-coordinate
// // must be defined outside of class, or else sort doesn't work
// {
// 	return (farm1 -> get_x()) < (farm2 -> get_x());
// }

#endif
