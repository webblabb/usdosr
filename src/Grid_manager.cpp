// Grid_manager.cpp
//
//  Creates a map of grid_cell objects, determined by local farm density
//	Evaluates infection spread via grid cells
//
//  7 Apr 2014

// included in Grid_manager.h: algorithm, queue, stack, unordered_map, vector
#include <cmath> // std::sqrt
#include <fstream>
#include <iostream>
#include <map> // just for printing in order
#include <sstream>
#include <string>
// included in Grid_manager.h: grid_cell, farm, shared_functions
#include "Grid_manager.h"

Grid_manager::Grid_manager(std::string &fname, bool xyswitch, bool v)
// fills farm_map, farmList, and xylimits
{
	setVerbose(v);
	if (verbose){std::cout << "Verbose option on" << std::endl;}
	// modified from Stefan's Farm Manager
	// read in file of premises
	double id, size, x, y;
	int fcount = 0;

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
				if (!xyswitch){
					str_cast(line_vector[1], y);
					str_cast(line_vector[2], x);
				} else if (xyswitch){
					str_cast(line_vector[1], x);
					str_cast(line_vector[2], y);
				}
				str_cast(line_vector[3], size);

				farm_map[fcount] = new Farm(id, x, y, size); 
				// write pointer to private var farm_map
				fcount++;
				
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

Grid_manager::~Grid_manager()
{
}

void Grid_manager::updateFarmList(std::vector<Farm*>& farmsInCell)
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

std::vector<Farm*> Grid_manager::getFarms(std::vector<double> cellSpecs, const unsigned int maxFarms=0)
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
    	if ((i->get_x() >= x) && (i->get_x() <= x+s) // if within x bounds of cell
    		&& (i->get_y() >= y) && (i->get_y() <= y+s)) // and within y bounds of cell
    		{ // farm is within the cell
    		inCell.emplace_back(i);
    		if (maxFarms!=0 && inCell.size() > maxFarms){break;} 
    		// saves time on retrieving farms if cell will be split & re-checked anyway
    		}
    }
    // (pointers to) farms in inCell should still be sorted by x-coordinate
    return(inCell);
}

void Grid_manager::removeParent(std::stack< std::vector<double> >& queue)
// The parent cell is the working cell 1st in the queue, so remove first element
{
    queue.pop();
}

void Grid_manager::addOffspring(std::vector<double> cellSpecs, std::stack< std::vector<double> >& queue)
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

void Grid_manager::commitCell(std::vector<double> cellSpecs, std::vector<Farm*>& farmsInCell)
// write cellSpecs as class grid_cell into set allCells
{
    double id, x, y, s;
    id = cellSpecs[0];
    x = cellSpecs[1];
    y = cellSpecs[2];
    s = cellSpecs[3];
    std::vector<Farm*> farms = farmsInCell;
    
    grid_cell* cellToAdd = new grid_cell(id, x, y, s, farms);
    
    allCells.emplace(id,cellToAdd); // add to map of all committed cells, with id as key
    assignCellIDtoFarms(id,farmsInCell);
    updateFarmList(farmsInCell);
}

void Grid_manager::splitCell(std::vector<double>& cellSpecs, std::stack< std::vector<double> >& queue)
{
    removeParent(queue);
    addOffspring(cellSpecs,queue);
}

void Grid_manager::assignCellIDtoFarms(double cellID, std::vector<Farm*>& farmsInCell)
{
	for (auto f:farmsInCell){
		f->set_cellID(cellID);
	}
}

void Grid_manager::initiateGrid(const unsigned int in_maxFarms, const int kernelRadius)
// maxFarms: If cell contains at least this many farms, subdivision will continue
// kernelRadius: maximum diffusion kernel radius (minimum cell size)
{
	set_maxFarms(in_maxFarms);
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

	if(verbose){
    	std::cout << "Cell side length = " << cellSpecs[3] << ". ";
    }

	// Case A: side length of cell is smaller than kernel - immediate commit
	if (cellSpecs[3] < kernelRadius*2){ // if side < kernel diameter
		farmsInCell = getFarms(cellSpecs); // want ALL farms, so don't include maxFarms as argument
		if (farmsInCell.size() > 0){ // if there are farms in cell, commit
        	cellSpecs[0] = cellCount;
        	commitCell(cellSpecs,farmsInCell);
        	cellCount = cellCount+1;
        	if (verbose){std::cout << "Side smaller than kernel diameter. Cell committed: #" << cellCount;}
        	removeParent(queue);
        } else { // no farms in cell, remove from queue w/o committing
        	removeParent(queue);
            if(verbose){std::cout << "No farms, removed cell, queue length = " << queue.size() << std::endl;}
        }
    // Case B: side length of cell >= kernel diameter, check farm density and split if needed
    } else if (cellSpecs[3] >= kernelRadius*2){ // side >= kernel diameter
    	if(verbose){std::cout << "Side bigger than kernel, stepping in..." << std::endl;}
    	farmsInCell = getFarms(cellSpecs, maxFarms); // copy up to maxFarms farms in cell to farmsInCell)
    	if(verbose){std::cout << "Farms in cell = " << farmsInCell.size() << std::endl;}
        if (farmsInCell.size() >= maxFarms){
        // if farm density too high, split
			if(verbose){std::cout << "Too many farms, splitting cell..." << std::endl;}
			splitCell(cellSpecs,queue);
        }
        else if (farmsInCell.size() > 0 && farmsInCell.size() < maxFarms){
        // farm density is below maximum, commit
            	cellSpecs[0] = cellCount;
                commitCell(cellSpecs,farmsInCell);
                cellCount = cellCount+1;
                if (verbose){std::cout << "Cell committed: #" << cellCount;}
                removeParent(queue);
            }
        else if (farmsInCell.empty()){
        // cell has no farms at all - remove from queue w/o committing
            removeParent(queue);
            if(verbose){std::cout << "No farms, removed cell, queue length = " 
            	<< queue.size() << std::endl;}
        }
    }
    } // end "while anything in queue"

	std::cout << "Grid created. Pre-calculating distances..." << std::endl;
	makeCellRefs();
}

void Grid_manager::initiateGrid(std::string& cname)
// overloaded (alternate) constructor that reads in external file of cells
// temporarily disabled due to incompatible std::to_string use
{
/*
	// read in file of premises
	std::vector<Farm*> farmsInCell;

	std::ifstream f(cname);
	if(!f){std::cout << "Input file not found." << std::endl;}
	if(f.is_open())
	{
	if (verbose){std::cout << "File open" << std::endl;}
		while(! f.eof())
		{
			std::vector<double> cellSpecs;
			std::string line;
			getline(f, line); // get line from file "f", save as "line"
			std::vector<std::string> line_vector = split(line, '\t'); // separate by tabs
			
			if(! line_vector.empty()) // if line_vector has something in it
			{ // convert each string piece to double
				if (verbose){std::cout << "New line: ";}
				cellSpecs.emplace_back(stod(line_vector[0])); //id
				if (verbose){std::cout << stod(line_vector[0]) << ", ";}
				cellSpecs.emplace_back(stod(line_vector[1])); //x
				if (verbose){std::cout << stod(line_vector[1]) << ", ";}
				cellSpecs.emplace_back(stod(line_vector[2])); //y
				if (verbose){std::cout << stod(line_vector[2]) << ", ";}
				cellSpecs.emplace_back(stod(line_vector[3])); //s
				if (verbose){std::cout << stod(line_vector[3]) << ". ";}
				// line_vector[4] is num farms-ignored (gets reassigned)
				farmsInCell = getFarms(cellSpecs);
				if (verbose){std::cout << farmsInCell.size() << " farms retrieved." << 			
					std::endl;}
				if(farmsInCell.empty()){
					std::cout << "Cell " << cellSpecs[0] << " has no farms - ignoring." << 				
						std::endl;
					// cell will not be added to list
					}
				else if (!farmsInCell.empty()){			
				// save cell with farms within
					allCells[cellSpecs[0]] = new grid_cell(cellSpecs[0], cellSpecs[1], 									
						cellSpecs[2], cellSpecs[3], farmsInCell); 
					assignCellIDtoFarms(cellSpecs[0],farmsInCell);
					updateFarmList(farmsInCell);
					}
			} // close "if line_vector not empty"
		} // close "while not end of file"
	} // close "if file is open"
	f.close();
	if (verbose){std::cout << "File closed" << std::endl;}
	std::cout << allCells.size() << " cells loaded from file. Sample cell: x=" 
		<< allCells.at(1)->get_x() << " y=" << allCells.at(1)->get_y() << " s=" 
		<< allCells.at(1)->get_s() << ", contains " << allCells.at(1)->get_num_farms() << std::endl;
	if (!farmList.empty()){
		Farm* f = farmList[0];
		std::cout << farmList.size() << " unassigned farms, first: " << f->get_id() << ": x=" << f->get_x() <<
			", y=" << f->get_y() << std::endl;
		}
	std::cout << "Grid loaded. Pre-calculating distances..." << std::endl;		
	makeCellRefs();
*/
}

void Grid_manager::initiateGrid(double cellSide)
{
    double min_x = xylimits[0];
    double max_x = xylimits[1];
    double min_y = xylimits[2];
    double max_y = xylimits[3];
    
    std::unordered_map<double, std::vector<Farm*>> cellFarmMap;
    std::vector<double> xlist, ylist; // list of each x corner, y corner
	std::vector<int> uniquex; // list of elements of first unique x values
		uniquex.emplace_back(0); // include first value (element/index 0)
	int cellCount = 0;
    // all x points will be from min_x to max_x by cellSide
    // let max be max + cellside for extra wiggle room, cells w/o farms will be excluded later
    for (auto x = min_x; x <= (max_x+cellSide); x+=cellSide) 
    {
    	// all y points will be from min_y to max_y by cellSide
    	for (auto y = min_y; y <= (max_y+cellSide); y+=cellSide)
    	{
    		xlist.emplace_back(x);
    		ylist.emplace_back(y);
    		cellCount++;
        	}
    uniquex.emplace_back(cellCount); // new x-value will start at element cellCount
    }
    // remove last x value (was added on after all finished)
    uniquex.pop_back();
    // assign farms to cells (have whole list already, so don't use getFarms)
    // compare each farm's coordinates to:
    // unique cell x values (increment according to xChanges)
    // y values once x value is found
    
    // indices for moving around the cell list
    int xi = 0; 
    int i = 0;
    int fcount = 0;
        
    for (auto f:farmList)
    {
		double farmx = f->get_x();
		double farmy = f->get_y();
    	bool cellFound = 0;
    	fcount++;
    	
    	while(!cellFound){
    		// if farm x is...
			if (farmx >= xlist[uniquex[xi]] && farmx < xlist[uniquex[xi+1]]) // between this and next unique x value
			{ // move ahead and narrow down y value
				i = uniquex[xi];
				while (!cellFound){
				if (farmy >= ylist[i] && farmy < ylist[i+1])
				// within y range - this is the cell
				{
					cellFarmMap[i].emplace_back(f);
					cellFound = 1;
				}
				else if (farmy >= ylist[i]){i++;}
				else if (farmy < ylist[i]){std::cout << "Farm in previous x value range, "; i--;}
				else {std::cout << "Farm y range not found." << std::endl;}
				} // end 2nd while cell not found
			}
			else if (farmx >= xlist[uniquex[xi+1]]) // or if farm x >= next larger x value, increase xi
			{xi++;}
			else if (farmx < xlist[i]) // or if farm x < this x value
			{std::cout << "Farm in previous x value range, "; xi--;}
			else {std::cout << "Farm x range not found." << std::endl;}
		} // end 1st while cell not found
    } // end for each farm
    std::cout << "Done placing farms." << std::endl;
   
   // write all cells with farms
   bool printNumFarms = 1;
   std::string allLinesToPrint;
   int actualCellCount = 0;
   for (auto c=0; c!=cellCount; c++)
   {
	   if (cellFarmMap[c].size()>0){ // if there are any farms in this cell:
	   		allCells[actualCellCount] = new grid_cell(actualCellCount, xlist[c], ylist[c], cellSide, cellFarmMap[c]);
	   		assignCellIDtoFarms(actualCellCount,cellFarmMap[c]);
	   		actualCellCount++;
	   		if (printNumFarms){
	   			char temp[5];
	   			//sprintf(temp, "%u\n", cellFarmMap[c].size()); // use this on sweatshop computers
	   			sprintf(temp, "%lu\n", cellFarmMap[c].size());
	   			allLinesToPrint += temp;
	   		}
	   }
   }
   if (printNumFarms){
   		std::string ofilename = "farmsPerUnifCell.txt";
   		std::ofstream f(ofilename);
	if(f.is_open())
	{
		f << allLinesToPrint;
		f.close();
	}
	}

	std::cout << "Grid loaded with " << actualCellCount << " uniform cells. Pre-calculating distances..." << std::endl;		
	makeCellRefs();
}

std::string Grid_manager::to_string(grid_cell& gc) const 
// overloaded to_string function, makes tab-delim string (one line) specifically for cell
{
	std::string toPrint;
	char temp[20];
	std::vector<double> vars;
		vars.resize(5);
		vars[0] = gc.get_id();
		vars[1] = gc.get_x();
		vars[2] = gc.get_y();
		vars[3] = gc.get_s();
		vars[4] = gc.get_num_farms();

	for(auto it = vars.begin(); it != vars.end(); it++)
	{
		sprintf(temp, "%f\t", *it);
		toPrint += temp;
	}
	
	toPrint.replace(toPrint.end()-1, toPrint.end(), "\n");
	
	return toPrint;
}

void Grid_manager::printCells(std::string& pfile) const
// input is premises file, to be included in cell file name
{
	// sort cells by ID (ensures farm reassignment will be in same order as when created)
	std::map<double, grid_cell*> orderedCells(allCells.begin(),allCells.end());
	double firstcell = orderedCells.begin()->first;
	if(verbose){std::cout << "First cell: " << firstcell << std::endl;}

	std::string tabdelim;
	tabdelim.reserve(orderedCells.size() * 50);
	for(auto it:orderedCells){
		tabdelim += to_string(*(it.second));
	}
	
	std::string ofilename ="unif";
 	char temp[5]; // make temporary storage
// 	sprintf(temp, "%i", maxFarms); // assign max # of farms as string
// 	ofilename += temp; // add to end of filename
// 	ofilename += "f_"; // add to end of filename
	int numCells = orderedCells.size(); // get number of cells
		sprintf(temp, "%i", numCells); // assign # of cells as string
	ofilename += temp;
	ofilename += "c_";
	ofilename += pfile;
	
	std::ofstream f(ofilename); 
	// will look something like "max15f_932c_USprems.txt"
	if(f.is_open()){
		f << tabdelim;
		f.close();
	}
	std::cout << "Cells printed to " << ofilename <<std::endl;

}

double Grid_manager::shortestCellDist(grid_cell* cell1, grid_cell* cell2)
// returns shortest distance between cell1 and cell2
{
	double cellDist = 0;
 	double cell1_id = cell1->get_id();
 	double cell2_id = cell2->get_id();
  if (cell1_id != cell2_id){ // else if comparing to self, distance is already 0 	
	double cell1_x, cell1_y, cell2_x, cell2_y; // will use these points to calc distance

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
	
	double xDiff = abs(cell1_x-cell2_x);
	double yDiff = abs(cell1_y-cell2_y);
	std::vector<double> orderedDiffs = orderNumbers(xDiff,yDiff);
	if (storedDists.count(orderedDiffs[0])==1 && 
		storedDists.at(orderedDiffs[0]).count(orderedDiffs[1])==1){
		cellDist = storedDists.at(orderedDiffs[0]).at(orderedDiffs[1]);
		std::cout << ".";
	} else {
		cellDist = sqrt(xDiff*xDiff + yDiff*yDiff);
 		storedDists[orderedDiffs[0]][orderedDiffs[1]] = cellDist;
 		std::cout << "Adding dist " << cellDist << ". ";
   	}
  }
// 	if (verbose){std::cout << cellDist << " between cells " << cell1->get_id() 
// 					  << " and " << cell2->get_id() << std::endl;}

return cellDist;	
}

void Grid_manager::makeCellRefs()
// Fills maps (think vector of vectors) of calculations:
// gridCellKernel: kernel values between cell 1 (lower number) and cell 2 (higher number),
// kernelNeighbors: cells for which kernel value > 0 for cell 1.
// Assumes cell IDs are sequential from 0 to allCells.size()-1
{
	double whichCell1, shortestDist, gridValue;
	
	for (auto c1:allCells){		
		whichCell1 = c1.first; // get ID number of first cell
		for (auto whichCell2 = whichCell1; whichCell2 != allCells.size(); whichCell2++){
			// get distance between grid cells 1 and 2...
			// if comparing to self, set distance=0
			if (whichCell2 == whichCell1){shortestDist = 0; 
			} else {
			shortestDist = shortestCellDist(c1.second, allCells.at(whichCell2)); 
			}
			// kernel value between c1, c2
			gridValue = kernel(shortestDist);
			// if grid Value is > 0, record in gridCellKernel and as kernel neighbors
			if (gridValue > 0){
				gridCellKernel[whichCell1][whichCell2] = gridValue;
				kernelNeighbors[whichCell1].emplace_back(whichCell2);
				kernelNeighbors[whichCell2].emplace_back(whichCell1);
			} // end if gridValue > 0
		} // end for each cell2
	} // end for each cell1
 	if (verbose){
 		std::cout << "Kernel distances and neighbors recorded." << std::endl;
 	}
}

void Grid_manager::printGridKernel() const // only prints values > 0
// temporarily disabled due to incompatible std::to_string use
{
/*
	// sort cell pairs by ID
	std::map<double, std::unordered_map<double, double>> orderedGKs(gridCellKernel.begin(),gridCellKernel.end());
	double firstcell = orderedGKs.begin()->first;
	if(verbose){std::cout << "First cell: " << firstcell << std::endl;}

	std::vector <double> cells_dists;
		cells_dists.resize(3); // temp storage for cell1, cell2, kernel value
	char temp[20];
	
	std::string tabdelim; // full output file
		//tabdelim.reserve(orderedGKs.size() * 50);
	for(auto it:orderedGKs){ // for each cell1 (first key)
	// "it" is the map for cell1
	std::unordered_map<double, double> cell1map = it.second;
	for(auto it2:cell1map){
	// it2 is each cell2 in the cell1 map
	 if (it2.second > 0){ // if kernel value > 0
		cells_dists[0] = it.first; // cell 1
		cells_dists[1] = it2.first; // cell 2
		cells_dists[2] = it2.second; // kernel value
		
		for (auto i:cells_dists){
			sprintf(temp, "%f\t", i);
			tabdelim += temp;
		}
		tabdelim.replace(tabdelim.end()-1, tabdelim.end(), "\n"); // add line break
	  } // end if kernel value > 0
	} // end for each cell2 in cell1 map
	} // end for each cell1 map
	
	int numCells = orderedGKs.size();
	std::string ofilename = "Kernel_";
	std::string snum = std::to_string(numCells);
	ofilename += snum;
	ofilename += "cells.txt";
	
	std::ofstream f(ofilename); // will look something like "Kernel_932cells.txt"
	if(f.is_open()){
		f << tabdelim;
		f.close();
	}
	*/
}

std::vector<grid_cell*> Grid_manager::IDsToCells(std::vector<double> cellIDs)
{
	std::vector<grid_cell*> neighborCells;
	for (auto i:cellIDs)
	{
		neighborCells.emplace_back(allCells.at(i));
	}
	return neighborCells;
}

grid_cell* Grid_manager::IDsToCells(double cellID)
//overloaded to accept single ID also
{
	return allCells.at(cellID);
}

std::vector<grid_cell*> Grid_manager::posKernelNeighborsOf(double cellID)
// get neighboring cells that have kernel values > 0
{
	return IDsToCells(kernelNeighbors.at(cellID));
}

void Grid_manager::stepThroughCells(std::vector<Farm*>& in_focalFarms, std::vector<Farm*>& in_compFarms)
// in_focalFarms is either all infectious (infectOut=1) or all susceptible (infectOut=0) premises
// in_focalFarms is either all susceptible (infectOut=1) or all infectious (infectOut=0) premises
{
	totalinfections = 0; // number of infections in this timestep
	// get cells of focal farms...
	std::unordered_map<double, std::vector<Farm*>> focalCellMap; // ...create a map to store them in
	for (auto ff:in_focalFarms){ // ff is Farm*
		// copy farm ids to key cellID
		focalCellMap[ff->get_cellID()].emplace_back(ff);
	}
	if (verbose){
		std::cout << std::endl << focalCellMap.size() << " cells contain focal farms. ";
	}
	// get cells of comp farms...
	std::unordered_map<double, std::vector<Farm*>> compCellMap; // ...create a map to store them in
	for (auto cf:in_compFarms){ // cf is Farm*
		// copy farm ids to key cellID
		compCellMap[cf->get_cellID()].emplace_back(cf);
		if(verbose){std::cout << "compCell " << cf->get_cellID() << " has " 
			<< compCellMap[cf->get_cellID()].size() << " farms" << std::endl;}
	}
	if(verbose){
		std::cout << std::endl << compCellMap.size() << " cells contain comparison farms: ";
		for (auto c:compCellMap){std::cout << c.first << " ";}
	}
	
	for (auto c1:focalCellMap)
	{ // loop through each cell (c1 as in "cell 1")
		grid_cell* currentCell = IDsToCells(c1.first); // set current cell
		if (verbose){std::cout << std::endl << "Focal cell set to " << currentCell->get_id();}
		std::vector<grid_cell*> neighborsOfFocal = posKernelNeighborsOf(currentCell->get_id());		
		if (verbose){std::cout << ", with " << neighborsOfFocal.size() << " neighbor cells. " ;}
		// neighbors include comparison to self to include other farms in same cell
		double focalNumFarms = currentCell->get_num_farms(); // how many farms in focal cell
		if (verbose){std::cout << "Focal cell: " << focalNumFarms << " farms. ";}
		
		// loop through each neighbor cell
		for (auto c2:neighborsOfFocal){	// loop through each cell (c2 as in "cell 2")
			if(compCellMap.count( c2->get_id() )>0){ // if this cell exists in the comp cell map (has eligible farms)
				double compNumFarms = (compCellMap.at( c2->get_id() )).size(); // how many farms in comparison cell	
				if (verbose){std::cout << " Comparison cell " << c2->get_id() <<": " << compNumFarms << " farms.";}
				// identify which cell ID is smaller/larger for grid value lookup, min comes first
				std::vector<double> ids = orderNumbers(currentCell->get_id(),c2->get_id());
				// look up grid value
				double gridKernValue = 0; // default kernel value is 0
				if ((gridCellKernel.count(ids[0]) == 1) && // something exists for first cell
				   (gridCellKernel.at(ids[0]).count(ids[1]) == 1)){ // something exists for first and second cell
						gridKernValue = gridCellKernel.at(ids[0]).at(ids[1]);}
				
				if (verbose){std::cout << std::endl << "Kernel: " << gridKernValue;}

				// maximum transmission of cells, initialized assuming infectOut is true
				double maxInf = currentCell->get_maxInf(); // max infectiousness of any farm in currentCell
				double maxSus = c2->get_maxSus(); // max susceptibility of any farm in comparison cell
				if (!infectOut){// if calculating spread TO focal cell, reassign values
					maxSus = currentCell->get_maxSus();
					maxInf = c2->get_maxInf();
				}
	
				double s = 1; // on/off switch, 1 = on
				// get farms in each cell
				std::vector<Farm*> focalFarmList = focalCellMap.at(currentCell->get_id());
				std::vector<Farm*> compFarmList = c2 -> get_farms();
		
				int f2count = 0; // how many farms in comparison cell have been checked
				double farmSus = 0;
				double farmInf = 0;
				double farmToCellProb = 0; // 1st "prob5" in MT's Fortran code, not divided out
				double indivFarmMaxProb = 0; // "prob6" in MT's Fortran code
				double remainingFarmsMaxProb = 0; // 2nd "prob5" in MT's Fortran code - replaces farmToCell while stepping through

				double f1x, f1y, f2x, f2y, xdiff, ydiff, distBWfarms, kernelBWfarms; // vars for farm pairs if evaluated
				double betweenFarmsProb = 0; // "prob3" in MT's Fortran code

				for (auto f1:focalFarmList){ // for each farm in focal cell
		
					if (infectOut){
						farmInf = getFarmInf(f1); // infectiousness value for farm in focal cell
						farmToCellProb = 1 - exp(-farmInf * compNumFarms*maxSus * gridKernValue);
					} else if (!infectOut){
						farmSus = getFarmSus(f1); // susceptibility value for farm in focal cell
						farmToCellProb = 1 - exp(-farmSus * compNumFarms*maxInf * gridKernValue);		
					}
								
					double random1 = unif_rand();
					if (random1 < farmToCellProb){ // if farm to cell succeeds
						if (infectOut){
							indivFarmMaxProb = 1 - exp(-farmInf * maxSus * gridKernValue); 
							// infectiousness of focal farm * max susceptibility in comp cell * grid kernel
						} else if (!infectOut){
							indivFarmMaxProb = 1 - exp(-farmSus * maxInf * gridKernValue); 		
							// susceptibility of focal farm * max infectiousness in comp cell * grid kernel
						}
			
					if (indivFarmMaxProb > 0){ // only continue if indivMax > 0 
						for (auto f2:compFarmList){
							f2count++;
							if (infectOut){			
								remainingFarmsMaxProb =1-(s*exp(-farmInf * (compNumFarms+1-f2count)*maxSus * gridKernValue));
							} else if (!infectOut){	
								remainingFarmsMaxProb =1-(s*exp(-farmSus * (compNumFarms+1-f2count)*maxInf * gridKernValue));
							}
							// # farms left in cell * farm(a) infectiousness * farm(b) susceptibility * grid kernel

							//if(verbose && f2count==1){std::cout << " remMaxProb: " << remainingFarmsMaxProb << " ";}
							double random2 = unif_rand(); // "prob4" in MT's Fortran code
							if (random2 < indivFarmMaxProb/remainingFarmsMaxProb){	
							//if(verbose){std::cout << random2 << " < " << indivFarmMaxProb/remainingFarmsMaxProb;}

							// if (one max susceptible)/(number of farms using specific sus-inf values) succeeds
								s = 0; // remainingFarmProb recalculates to 1 for remainder of loop
								// get actual distances between farms
								f1x = f1 -> get_x();
								f1y = f1 -> get_y();
								f2x = f2 -> get_x();
								f2y = f2 -> get_y();
								xdiff = (f1x - f2x);
								ydiff = (f1y - f2y);
								distBWfarms = sqrt(xdiff*xdiff + ydiff*ydiff);
								// add kernel choice option
								kernelBWfarms = kernel(distBWfarms);
								// get individual infectiousness/susceptibility values
								if (infectOut){			
									farmSus = getFarmSus(f2); // susceptible farm in comparison cell (farmInf already defined from focal cell)
								} else if (!infectOut){	
									farmInf = getFarmInf(f2); // infectious farm in comparison cell (farmSus already defined from focal cell)
								}
								// calculate probability between these specific farms
								betweenFarmsProb = 1-exp(-farmSus * farmInf * kernelBWfarms); // prob tx between this farm pair
								// "prob3" in MT's Fortran code
								if (random2 < betweenFarmsProb/remainingFarmsMaxProb){
								// why don't we account for the indivFarmMaxProb? b/c we use the same random number?
									// infect
									totalinfections++;
									if (verbose){std::cout << " Farm infected. ";}
									}
								} // end "if trans between indiv farms at max"
							} // end for each comparison cell farm
							f2count = 0;
					} // end if indivMaxProb > 0
							
					} else { // otherwise if p(farm->cell) fails
						farmtocellskips++;
						farmsinskippedcells += compNumFarms;
						} // end if p(farm->cell) fails
						
				} // end for each focal cell farm			
			} // end if comp cell has eligible farms
		} // end for loop through comparison cells
	} // end loop through focal cells
	std::cout << std::endl
		<< "Farm to cell skips: " << farmtocellskips << " (avoided " << farmsinskippedcells << " comparisons)" << std::endl
		<< "Total infections (gridding): " << totalinfections << std::endl;
}

// input is proportion of focal farms (random), all remaining farms are comparison
std::vector <std::vector<Farm*>> Grid_manager::fakeFarmStatuses(double propFocal)
{ 
 	std::vector <Farm*> focal, comp; // two vectors of focal/comp farms

	for (auto i=0; i!=farm_map.size(); i++){
		double randomnum = unif_rand();
		if (randomnum <= propFocal){
			focal.emplace_back(farm_map.at(i));
		} else {
			comp.emplace_back(farm_map.at(i));
		}
	}
	std::vector <std::vector <Farm*>> farmsToReturn;
 		farmsToReturn.emplace_back(focal);
 		farmsToReturn.emplace_back(comp);
 	std::cout << "Returning " << focal.size() << " focal farms and " << comp.size() << " comparison farms." << std::endl;
 	return farmsToReturn;
 }
 
// input is both focal and comp farms (essentially random)
std::vector <std::vector<Farm*>> Grid_manager::fakeFarmStatuses(double numFocal, double numComp)
{ 
 	std::vector <Farm*> focal, comp; // two vectors of focal/comp farms

	for (auto i=0; i!=numFocal; i++)
		{focal.emplace_back(farm_map.at(i));}
	for (auto j=numFocal; j!=numFocal+numComp; j++)
		{comp.emplace_back(farm_map.at(j));}
	
/*	
	std::unordered_map<double, Farm*> focalAndComp; // map of all farms to be used
		focalAndComp.reserve(numFocal+numComp);
	// while finished size not reached
	while (focalAndComp.size() < (numFocal+numComp)){
		int tempFarmID = rand() % farm_map.size(); // pick random ID
		if(focalAndComp.count(tempFarmID)==0){ // if this key (farm ID) doesn't already exist in the map...
			focalAndComp[tempFarmID] = farm_map[tempFarmID]; // add to map (map size updates automatically)
		} // end if farm not already in map
	} // end while map not at finished size

 	std::vector <Farm*> focal, comp; // split into vectors of focal/comp farms
 	
 	int count=0;
 	for (auto farm:focalAndComp){
 		count++;
 		if (count <= numFocal){focal.emplace_back(farm.second);} // add Farm* to focal
 		else if (count > numFocal){comp.emplace_back(farm.second);} // add Farm* to comp
 	}
*/	
 	std::vector <std::vector <Farm*>> farmsToReturn;
 		farmsToReturn.emplace_back(focal);
 		farmsToReturn.emplace_back(comp);
 	return farmsToReturn;
 }