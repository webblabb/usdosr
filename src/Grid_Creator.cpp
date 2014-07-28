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

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath> // std::sqrt
#include <stack> // for queue
#include <algorithm> // std::sort

#include "Grid_Creator.h"

Grid_Creator::Grid_Creator(std::string &fname, bool v)
// fills farm_map and xylimits
{
	setVerbose(v);
	if (verbose){std::cout << "Verbose option on" << std::endl;}
	// modified from Stefan's Farm Manager
	// read in file of premises
	double id, size, x, y;

	std::ifstream f(fname);
	if(!f){std::cout << "Input file not found." << std::endl;}
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
					if (x < xylimits[0]){xylimits[0] = x;}
					else if (x > xylimits[1]){xylimits[1] = x;}
					
					if (y < xylimits[2]){xylimits[2] = y;}
					else if (y > xylimits[3]){xylimits[3] = y;}
					}
				else {
					if (verbose){std::cout << "Initializing xy limits...";}
					xylimits.emplace_back(x); // min x value
					xylimits.emplace_back(x); // max x value
					xylimits.emplace_back(y); // min y value
					xylimits.emplace_back(y); // max y value
					} 

			} // close "if line_vector not empty"
		} // close "while not end of file"
	} // close "if file is open"
	if (verbose)
	{std::cout << "finalized." << std::endl;
	std::cout << "x min = " << xylimits[0] << std::endl;
	std::cout << "x max = " << xylimits[1] << std::endl;
	std::cout << "y min = " << xylimits[2] << std::endl;
	std::cout << "y max = " << xylimits[3] << std::endl;}
	
	f.close();
	if (verbose){std::cout << "File closed" << std::endl;}

	// copy farmlist from farm_map (will be changed as grid is created)
	if (verbose){std::cout << "Copying farms from farm_map to farmList..." << std::endl;}
	for (auto prem: farm_map)
	{
		farmList.push_back(prem.second); // "second" value from map is Farm pointer
	}
	
	double firstx = farmList.front()->get_x();
	double lastx = farmList.back()->get_x();
	if (verbose)
	{
		std::cout << "farmList length: " << farmList.size() << std::endl;

		std::cout << "First farm x: " << firstx << std::endl;
		std::cout << "Last farm x: " << lastx << std::endl;
		std::cout << "Sorting..." << std::endl;
	}

	// sort farmList by x
	std::sort(farmList.begin(),farmList.end(),sortByX); // sorted farms by x-coordinates
	firstx = farmList.front()->get_x();
	lastx = farmList.back()->get_x();
	if (verbose)
	{
		std::cout << "First farm x: " << firstx << std::endl;
		std::cout << "Last farm x: " << lastx << std::endl;
	}

}

Grid_Creator::~Grid_Creator()
{
	for(auto f:farm_map)
	{
		delete f.second; // delete mapped value
	}
}

void Grid_Creator::updateFarmList(std::vector<Farm*>& farmsInCell)
// remove farmsInCell from farmList, assumes matches are sorted in same order
{
	// farmsInCell should already be sorted (subsetted from sorted farmList)
	if(verbose){std::cout << " Farms in cell: " << farmsInCell.size() 
		<< std::endl;}
		
	auto it2 = farmList.begin();
	for(auto it = farmsInCell.begin(); it != farmsInCell.end(); it++)
	// loop through each farm in cell
	{		
		while (it2 != farmList.end()) // while end of farmList not reached
		{
			if(*it2 == *it) // finds first match in farmList to farmInCell
			{
				farmList.erase(it2); // remove from farmList
				break;
			}
			it2++; // 
		}
	}
}

std::vector<Farm*> Grid_Creator::getFarms(std::vector<double> cellSpecs, const unsigned int maxFarms)
// based on cell specs, finds farms in cell and saves pointers to farmsInCell
{
	if(verbose){std::cout << "Getting farms in cell..." << std::endl;}

    // cellSpecs[0] is placeholder for ID number, added when committed
    double x = cellSpecs[1];
    double y = cellSpecs[2];
    double s = cellSpecs[3];
    std::vector<Farm*> inCell;
    
    // look for farms in cell, those falling on grid boundaries are included, will be removed from list when cell is committed to avoid double counting
    
    for (auto i:farmList)
    {
    	if ((i->get_x() >= x) && (i->get_x() <= x+s)
    		&& (i->get_y() >= y) && (i->get_y() <= y+s))
    		{ // farm is within the cell
    		inCell.emplace_back(i);
    		if (inCell.size() > maxFarms){break;} 
    		// saves time on retrieving farms if cell will be split & re-checked anyway
    		}
    }
    // (pointers to) farms in inCell should still be sorted by x-coordinate
    return(inCell);
}

void Grid_Creator::removeParent(std::stack< std::vector<double> >& queue)
// The parent cell is the working cell 1st in the queue, so remove first element
{
    queue.pop();
}

void Grid_Creator::addOffspring(std::vector<double> cellSpecs, std::stack< std::vector<double> >& queue)
// offspring cells are quadrants of parent cell
{
	// cellSpecs[0] is placeholder for ID number
    double x = cellSpecs[1];
    double y = cellSpecs[2];
    double s = cellSpecs[3];
    
    // lower left quadrant: same x/y, side/2
    std::vector<double> lowerLeft = {0, x, y, s/2};
    // lower right quadrant: add side/2 to x, same y, side/2
    std::vector<double> lowerRight = {0, x+s/2, y, s/2};
    // upper left quadrant: same x, add side/2 to y, side/2
    std::vector<double> upperLeft = {0, x, y+s/2, s/2};
    // upper right quadrant: add side/2 to x and y, side/2
    std::vector<double> upperRight = {0, x+s/2, y+s/2, s/2};
    
    // add offspring cells to queue (in reverse order so lower left is first)
    queue.emplace(upperRight);
    queue.emplace(upperLeft);
    queue.emplace(lowerRight);
    queue.emplace(lowerLeft);
}

void Grid_Creator::commitCell(std::vector<double> cellSpecs, std::vector<Farm*>& farmsInCell)
// write cellSpecs as class grid_cell into set allCells
{
    double id, x, y, s;
    id = cellSpecs[0];
    x = cellSpecs[1];
    y = cellSpecs[2];
    s = cellSpecs[3];
    std::vector<Farm*> farms = farmsInCell;
    
    grid_cell* cellToAdd = new grid_cell(id, x, y, s, farms);
    
    allCells.emplace_back(cellToAdd); // add to vector of all committed cells
}

void Grid_Creator::splitCell(std::vector<double> cellSpecs, std::stack< std::vector<double> >& queue)
{
    removeParent(queue);
    addOffspring(cellSpecs,queue);
}

void Grid_Creator::initiateGrid(const unsigned int maxFarms, const int kernelRadius)
// maxFarms: If cell contains at least this many farms, subdivision will continue
// kernelRadius: maximum diffusion kernel radius (minimum cell size)
{
	std::cout << "Max farms set to " << maxFarms << std::endl;
    if(verbose){std::cout << "Splitting into grid cells..." << std::endl;}
 	double cellCount = 0;
    std::stack<std::vector <double>> queue;// temporary list of cells to check for meeting criteria for commitment
    std::vector<Farm*> farmsInCell; // vector of (pointers to) farms in working cell - using vector to get to specific elements
        
    double min_x = xylimits[0];
    double max_x = xylimits[1];
    double min_y = xylimits[2];
    double max_y = xylimits[3];
    
    double side_x = max_x - min_x;
    double side_y = max_y - min_y;
    if(verbose)
    {
    	std::cout << "side_x = " << side_x << std::endl;
    	std::cout << "side_y = " << side_y << std::endl;
    }

    // use whichever diff is larger, x or y
    if (side_y > side_x)
       side_x = side_y; 
    if(verbose){std::cout << "Using larger value " << side_x << std::endl;}
    
    // add cell specifications to temporary vector
    std::vector<double> cellSpecs = {cellCount, min_x, min_y, side_x};
    if(verbose){std::cout << "cellSpecs: " << cellSpecs[0] <<", "<< cellSpecs[1] 
    	<<", "<< cellSpecs[2] <<", "<< cellSpecs[3] << std::endl;}

    // add initial cell to the queue
    queue.emplace(cellSpecs);

    // while there are any items in queue
    while(queue.size()>0)
    {
    if(verbose){std::cout << std::endl << "Queue length = " << queue.size() << std::endl;}
    
    cellSpecs = queue.top(); // set first in queue as working cell
    farmsInCell = getFarms(cellSpecs, maxFarms); // get/write farms in cell (to farmsInCell)
    if(verbose)
    {
    	std::cout << "Farms in cell = " << farmsInCell.size() 
    		<< std::endl;
    	std::cout << "Side length = " << cellSpecs[3] <<
    	", Diameter = " << kernelRadius*2 << ". ";
    }

    if (cellSpecs[3] >= kernelRadius*2) // side >= kernel diameter
    {
    if(verbose){std::cout << "Side bigger than kernel, stepping in..." << std::endl;}
        if (farmsInCell.size() >= maxFarms)
        // if farm density too high
        {
			if(verbose){std::cout << "Too many farms, splitting cell..." << std::endl;}
			splitCell(cellSpecs,queue);
        }
        else if (farmsInCell.size() > 0 && farmsInCell.size() < maxFarms)
        // farm density is below maximum
            {
            	cellSpecs[0] = cellCount;
                commitCell(cellSpecs,farmsInCell);
                cellCount = cellCount+1;
                if (verbose){std::cout << "Cell committed: #" << cellCount;}
                 updateFarmList(farmsInCell);
                removeParent(queue);
            }
        else if (farmsInCell.empty())
        // cell has no farms at all - remove from queue w/o committing
        {
            removeParent(queue);
            if(verbose){std::cout << "No farms, removed cell, queue length = " 
            	<< queue.size() << std::endl;}
        }
    }
    else if ((cellSpecs[3] < kernelRadius*2) // side < kernel diameter
             && (farmsInCell.size() > 0)) // and there are farms in cell
        {
        cellSpecs[0] = cellCount;
        commitCell(cellSpecs,farmsInCell);
        cellCount = cellCount+1;
        if (verbose){std::cout << "Side smaller than kernel. Cell committed: #" << cellCount;}
        updateFarmList(farmsInCell);
        removeParent(queue);
        }
    else if (cellSpecs[3] < kernelRadius*2 // side < kernel diameter
             && farmsInCell.empty()) // and no farms in cell
        {
        removeParent(queue);
        if(verbose){std::cout << "No farms, removed cell, queue length = " 
            	<< queue.size() << std::endl;}
        }
    } // end "while anything in queue"

	printCells();
	makeCellRefs();
}

void Grid_Creator::printCells() const
{
	std::string tabdelim;
	tabdelim.reserve(allCells.size() * 50);
	for(auto it = allCells.begin(); it != allCells.end(); it++)
	{
		tabdelim += (*it)->to_string();
	}
	
	char temp[5];
	std::string ofilename = "cellList_";
	int numCells = allCells.size();
	ofilename += sprintf(temp, "%d\t", numCells);
	ofilename += "cells.txt";
	
	std::ofstream f(ofilename); // will look something like "cellList_932cells.txt"
	if(f.is_open())
	{
		f << tabdelim;
		f.close();
	}
}

double Grid_Creator::shortestCellDist(grid_cell* cell1, grid_cell* cell2) const
// returns shortest distance between cell1 and cell2
{
	double cell1_x, cell1_y, cell2_x, cell2_y; // will use these points to calc distance

// 	double cell1_id = cell1->get_id();
// 	double cell2_id = cell2->get_id();

	double cell1_South = cell1->get_y(); // lower boundary of cell1
	double cell1_North = cell1->get_y()+cell1->get_s(); // upper boundary of cell1
	double cell1_West = cell1->get_x(); // leftmost boundary of cell1
	double cell1_East = cell1->get_x()+cell1->get_s(); // rightmost boundary of cell1
	
	double cell2_South = cell2->get_y(); // lower boundary of cell2
	double cell2_North = cell2->get_y()+cell2->get_s(); // upper boundary of cell2
	double cell2_West = cell2->get_x(); // leftmost boundary of cell2
	double cell2_East = cell2->get_x()+cell2->get_s(); // rightmost boundary of cell2
	
	// In comparing cell positions, due to nestedness of sub-cells, cell 1 could be:
	// Horizontally: W of, E of, or directly above/below all or part of cell2.
	// Vertically: N of, S of, or directly beside all or part of cell2.
	
	// Determine horizontal relationship and set x values accordingly:
// 	if(verbose){std::cout << "Cell " << cell1_id << " is ";}
	
	if (cell1_East <= cell2_West) // cell1 west of cell2
		{
// 		if(verbose){std::cout << "west of and ";}
		cell1_x = cell1_East;
		cell2_x = cell2_West;
		}
	// or cell1 is east of cell2
	else if (cell1_West >= cell2_East)
		{
// 		if(verbose){std::cout << "east of and ";}
		cell1_x = cell1_West;
		cell2_x = cell1_East;
		}
	// or cell1 is directly atop all or part of cell2
	else // if ((cell1_East > cell2_West) && (cell1_West < cell2_East))
		{
// 		if(verbose){std::cout << "vertically aligned with and ";}
		cell1_x = 0;
		cell2_x = 0;
		// only use distance between y values
		}
	
	// Determine vertical relationship and set y values accordingly:
	if (cell1_South >= cell2_North) // cell1 north of cell2
		{
// 		if(verbose){std::cout << "north of cell "<< cell2_id << std::endl;}
		cell1_y = cell1_South;
		cell2_y = cell2_North;
		}
	// or cell1 is below cell2
	else if (cell1_North <= cell2_South)
		{
// 		if(verbose){std::cout << "south of cell "<< cell2_id << std::endl;}
		cell1_y = cell1_North;
		cell2_y = cell2_South;
		}
	// or cell1 is directly beside cell2
	else // if ((cell1_South < cell2_North) && (cell1_North > cell2_South))
		{
// 		if(verbose){std::cout << "horizontally aligned with cell "<< cell2_id << std::endl;}
		cell1_y = 0;
		cell2_y = 0;
		// only use distance between x values
		}	
	
	double xDiff = cell1_x-cell2_x;
	double yDiff = cell1_y-cell2_y;
	double cellDist = sqrt(xDiff*xDiff + yDiff*yDiff);
	
// 	if (verbose){std::cout << cellDist << " between cells " << cell1->get_id() 
// 					  << " and " << cell2->get_id() << std::endl;}

return cellDist;	
}

void Grid_Creator::makeCellRefs()
// fill maps (think vector of vectors) of distances between all pairs of cells
// and kernel values for those distances
{
	for (auto c1:allCells)
		{		
		double whichCell1 = c1->get_id();
		for (auto c2:allCells)
			{
			double whichCell2 = c2->get_id();
			double shortestDist = shortestCellDist(c1, c2);
			cellDists[whichCell1][whichCell2] = shortestDist;
			// distance between c1, c2
			gridCellKernel[whichCell1][whichCell2] = linearDist(shortestDist);
			// kernel value between c1, c2
			}
		}
	if (verbose)
	{
		std::cout << "Dist b/w first and last:" << cellDists[0][255] 
	<< std::endl;
		std::cout << "Kernel b/w first and last:" << gridCellKernel[0][255] 
	<< std::endl;
	 }
}