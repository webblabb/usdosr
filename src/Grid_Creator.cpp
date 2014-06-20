//
//  Grid_Creator.cpp
//
//  Creates a grid of varying cell sizes, dictated by farm density. Limited
// by size of kernel radius - will not create cells much smaller than kernel
// radius or cells without farms inside.
//  Checks range of x/y coordinates of all premises. Uses larger difference as
// side to form a square, with the lower left corner at the minimum x/y
// positions.
//  Adds square (initial cell) to queue of cells to check. As cells meet
// criteria (either by farm density within or size), they are committed as
// members of class Grid_Cell
//
//  7 Apr 2014
//
// Testing notes: start with 3x3 grid


#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath> // std::sqrt
#include <stack>
#include <algorithm> // std::sort

#include "Grid_Creator.h"
// #include "grid_cell.h" // class grid_cell
#include "farm.h" // class Farm - need to simplify for this test
#include "String_functions.h" // for str_cast

Grid_Creator::Grid_Creator(std::string &fname, bool v)
// fills farm_map and xylimits
{
	setVerbose(v);
	if (verbose){std::cout << "Verbose option on" << std::endl;}
	// modified from Stefan's Farm Manager
	// read in file of premises
	int id, size, x, y;

	std::ifstream f(fname);
	if(f.is_open())
	{
	if (verbose){std::cout << "File open" << std::endl;}
		while(! f.eof())
		{
			std::string line;
			getline(f, line); // get line from file "f", save as "line"
			std::vector<std::string> line_vector = split(line, '\t'); // separate by tabs
			
			if(! line_vector.empty()) // if line_vector has something in it
			{
				str_cast(line_vector[0], id);
				str_cast(line_vector[1], x);
				str_cast(line_vector[2], y);
				str_cast(line_vector[3], size);

				farm_map[id] = new Farm(id, x, y, size); 
				// write pointer to private var farm_map
				
				// compare/replace limits of xy plane
				if (!xylimits.empty())// if there are values to compare
					{ 
					if (x < xylimits[0])
						{
						xylimits[0] = x;
						if (verbose){std::cout << "x min set to " << x << std::endl;}
						}
					else if (x > xylimits[1])
						{
						xylimits[1] = x;
						if (verbose){std::cout << "x max set to " << x << std::endl;}
						}
					
					if (y < xylimits[2])
						{
						xylimits[2] = y;
						if (verbose){std::cout << "y min set to " << y << std::endl;}
						}
					else if (y > xylimits[3])
						{
						xylimits[3] = y;
						if (verbose){std::cout << "y max set to " << y << std::endl;}
						}
					}
				else {
					if (verbose){std::cout << "Initializing xy limits..." << std::endl;}
					xylimits.push_back(x); // min x value
					xylimits.push_back(x); // max x value
					xylimits.push_back(y); // min y value
					xylimits.push_back(y); // max y value
					} 

			} // close "if line_vector not empty"
		} // close "while not end of file"
	} // close "if file is open"
	f.close();
	if (verbose){std::cout << "File closed" << std::endl;}

}

Grid_Creator::~Grid_Creator()
{
	for(auto f:farm_map)
	{
		delete f.second; // delete mapped value
	}
}

void Grid_Creator::setVerbose(bool v)
{
	verbose = v;
}

// void Grid_Creator::updateFarmList(std::vector<Farm*>& farmsInCell)
// // remove farmsInCell from farmList
// {
// 	// farmsInCell should already be sorted (subsetted from sorted farmList)
// 	auto it2 = farmList.begin();
// 	for(auto it = farmsInCell.begin(); it != farmsInCell.end(); it++)
// 	// loop through each farm in cell
// 	{		
// 		while (it2 != farmList.end())
// 		{
// 			if(*it2 == *it)
// 			{
// 				it2 = farmList.erase(it2);
// 				break;
// 			}
// 			it2++;
// 		}
// 	}
// }
// 
// void Grid_Creator::getFarms(std::vector<double> cellSpecs, std::vector<Farm*>& farmsInCell)
// // makes list of farms in a cell, stores as vector of Farm pointers
// {
//     // cellSpecs[0] is placeholder for ID number
//     double x = cellSpecs[1];
//     double y = cellSpecs[2];
//     double s = cellSpecs[3];
//     std::vector<Farm*> inCell;
//     
//     auto i = farmList.begin();
//     while ((*i)->get_x() < x){i++;} // iterate through farmList until farms' x coords = x
//     while ((*i)->get_x() >= x // while farms are within [x, x+s), copy to vector, i++
//     		&&(*i)->get_x() < x+s)
//     {
//     	inCell.emplace_back(*i);
// 		i++;
//     }
//     for (auto it = inCell.end(); it != inCell.begin(); it--)
//     // loop through the farms within x range to find those within y range 
//     // (run backwards to avoid erasure/iterator reference issues)
//     {
//     	if ((*it)->get_y() < y // if south of lower bound,
//     	|| (*it)->get_y() >= y+s) // or north of upper bound
//     		{inCell.erase(it);} // remove farm 
//     }
//     // (pointers to) farms remaining in inCell should still be sorted by x-coordinate
//     farmsInCell = inCell;
// }
// 
// void Grid_Creator::removeParent(std::stack< std::vector<double> >& queue)
// // The parent cell is the working cell 1st in the queue, so remove first element
// {
//     queue.pop();
// }
// 
// void Grid_Creator::addOffspring(std::vector<double> cellSpecs, std::stack< std::vector<double> >& queue)
// // offspring cells are quadrants of parent cell
// {
// 	// cellSpecs[0] is placeholder for ID number
//     double x = cellSpecs[1];
//     double y = cellSpecs[2];
//     double s = cellSpecs[3];
//     
//     // lower left quadrant: same x/y, side/2
//     std::vector<double> lowerLeft = {x, y, s/2};
//     // lower right quadrant: add side/2 to x, same y, side/2
//     std::vector<double> lowerRight = {x+s/2, y, s/2};
//     // upper left quadrant: same x, add side/2 to y, side/2
//     std::vector<double> upperLeft = {x, y+s/2, s/2};
//     // upper right quadrant: add side/2 to x and y, side/2
//     std::vector<double> upperRight = {x+s/2, y+s/2, s/2};
//     
//     // add offspring cells to queue (in reverse order so lower left is first)
//     queue.emplace(upperRight);
//     queue.emplace(upperLeft);
//     queue.emplace(lowerRight);
//     queue.emplace(lowerLeft);
// }
// 
// void Grid_Creator::commitCell(std::vector<double> cellSpecs, std::vector<Farm*>& farmsInCell)
// // write cellSpecs as class grid_cell into set allCells
// {
//     double id, x, y, s;
//     id = cellSpecs[0];
//     x = cellSpecs[1];
//     y = cellSpecs[2];
//     s = cellSpecs[3];
//     std::vector<Farm*> farms = farmsInCell;
//     
//     allCells.emplace_back(grid_cell(id,x,y,s,farms)); // add to vector of all committed cells
// }
// 
// void Grid_Creator::splitCell(std::vector<double> cellSpecs, std::stack< std::vector<double> >& queue)
// {
//     removeParent(queue);
//     addOffspring(cellSpecs,queue);
// }
// 
// void Grid_Creator::initiateGrid(const std::vector<int> xylimits, const unsigned int maxFarms, const int kernelRadius, std::vector<Farm*> farmList)
// // maxFarms: If cell contains at least this many farms, subdivision will continue
// // kernelRadius: maximum diffusion kernel radius (minimum cell size)
// {
// 	int cellCount = 0;
//     std::stack<std::vector <int>> queue;// temporary list of cells to check for meeting criteria for commitment
//     std::vector<Farm*> farmsInCell; // vector of (pointers to) farms in working cell
//     
//     std::sort(farmList.begin(),farmList.end(),sortByX); // sorted farms by x-coordinates
//     
//     int min_x = xylimits[0];
//     int max_x = xylimits[1];
//     int min_y = xylimits[2];
//     int max_y = xylimits[3];
//     
//     int side_x = max_x - min_x;
//     int side_y = max_y - min_y;
//     // use whichever diff is larger, x or y
//     if (side_y > side_x)
//        side_x = side_y; 
//     
//     // add cell specifications to temporary vector
//     std::vector<int> cellSpecs = {cellCount, min_x, min_y, side_x};
// 
//     // add initial cell to the queue
//     queue.emplace(cellSpecs);
// 
//     // while there are any items in queue
//     while(queue.size()>0)
//     {
//     cellSpecs = queue.top(); // set first in queue as working cell
//     getFarms(cellSpecs, farmsInCell); // get/write farms in cell (to farmsInCell)
//     if (cellSpecs[2] >= kernelRadius*2) // side >= kernel diameter
//         {
//         if (farmsInCell.size() > 0) // if any farms are in cell
//         {
//             if (farmsInCell.size() >= maxFarms) // if farm density too high
//                 splitCell(cellSpecs,queue);
//             else // farm density is below maximum
//             {
//                 commitCell(cellSpecs,farmsInCell);
//                 cellCount = cellCount+1.0;
//                 if (verbose){cout << "Cells committed: " << cellCount;}
//                 updateFarmList(farmsInCell);
//                 removeParent(queue);
//             }
//         }
//         else // cell has no farms at all - remove from queue w/o committing
//             removeParent(queue);
//     }
//     else if (cellSpecs[2] < kernelRadius*2 // side < kernel diameter
//              && farmsInCell.size() > 0) // and there are farms in cell
//         {
//         commitCell(cellSpecs,farmsInCell);
//         cellCount = cellCount+1.0;
//         if (verbose){cout << "Cells committed: " << cellCount;}
//         updateFarmList(farmsInCell);
//         removeParent(queue);
//         }
//     else // side is smaller than kernel diameter and no farms in cell
//         removeParent(queue);
//     } // end "while anything in queue"
// }
// 
// double Grid_Creator::shortestCellDist(grid_cell* cell1, grid_cell* cell2)
// // returns shortest distance between cell1 and cell2
// {
// 	double cell1_x, cell1_y, cell2_x, cell2_y; // will use these points to calc distance
// 
// 	double cell1_South = cell1->get_y(); // lower boundary of cell1
// 	double cell1_North = cell1->get_y()+get_s(); // upper boundary of cell1
// 	double cell1_West = cell1->get_x(); // leftmost boundary of cell1
// 	double cell1_East = cell1->get_x()+get_s(); // rightmost boundary of cell1
// 	
// 	double cell2_South = cell2->get_y(); // lower boundary of cell2
// 	double cell2_North = cell2->get_y()+get_s(); // upper boundary of cell2
// 	double cell2_West = cell2->get_x(); // leftmost boundary of cell2
// 	double cell2_East = cell2->get_x()+get_s(); // rightmost boundary of cell2
// 	
// 	// In comparing cell positions, due to nestedness of sub-cells, cell 1 could be:
// 	// Horizontally: W of, E of, or directly above/below all or part of cell2.
// 	// Vertically: N of, S of, or directly beside all or part of cell2.
// 	
// 	// Determine horizontal relationship and set x values accordingly:
// 	if (cell1_East <= cell2_West) // cell1 west of cell2
// 		{
// 		cell1_x = cell1_East;
// 		cell2_x = cell2_West;
// 		}
// 	// or cell1 is east of cell2
// 	else if (cell1_West >= cell2_East)
// 		{
// 		cell1_x = cell1_West;
// 		cell2_x = cell1_East;
// 		}
// 	// or cell1 is directly atop all or part of cell2
// 	else
// 		{
// 		cell1_x = 0;
// 		cell2_x = 0;
// 		// only use distance between y values
// 		}
// 	
// 	// Determine vertical relationship and set y values accordingly:
// 	if (cell1_South >= cell2_North) // cell1 north of cell2
// 		{
// 		cell1_y = cell1_South;
// 		cell2_y = cell2_North;
// 		}
// 	// or cell1 is below cell2
// 	else if (cell1_North <= cell2_South)
// 		{
// 		cell1_y = cell1_North;
// 		cell2_y = cell2_South;
// 		}
// 	// or cell1 is directly beside cell2
// 	else
// 		{
// 		cell1_y = 0;
// 		cell2_y = 0;
// 		// only use distance between x values
// 		}	
// 	
// 	double xDiff = cell1_x-cell2_x;
// 	double yDiff = cell1_y-cell2_y;
// 	double cellDist = sqrt(xDiff*xDiff + yDiff*yDiff);
// 	
// 	if (verbose){cout << cellDist << "between cells" << cell1->get_id() 
// 					  << "and" << cell2->get_id();}
// 
// return cellDist;	
// }
// 
// double Grid_Creator::gridKernel(double dist)
// // retrieves kernel value based on distance
// {
// return(dist);
// }
// 
// void Grid_Creator::makeCellRefs()
// // output a matrix (actually vector of vectors) of distances between all pairs of cells
// // maybe use unordered map?
// {
// 	numCells = allCells.size();
// 	for (auto c1:allCells)
// 		{
// 		int whichCell1 = int(c1->get_id());
// 		std::vector <double> tempDists(numCells,0);
// 		std::vector <double> tempKernels(numCells,0);
// 		for (auto c2:allCells)
// 			{
// 			int whichCell2 = int(c2->get_id());
// 			tempDists[whichCell2] = shortestCellDist(c1, c2); // distance between c1, c2
// 			tempKernels[whichCell2] = gridKernel(tempDists[whichCell2]); // kernel value
// 			}
// 		cellDists[whichCell1] = tempDists;
// 		gridCellKernel[whichCell1] = tempKernels;
// 		}
// }