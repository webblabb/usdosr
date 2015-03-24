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
#include <set> // for comparing otherwise unsorted lists of infected farms, pw vs gridding
#include <sstream>
#include <string>
#include <ctime> // for timing
// included in Grid_manager.h: grid_cell, farm, shared_functions, tuple, utility
#include "Grid_manager.h"

Grid_manager::Grid_manager(std::string &fname, bool xyswitch, std::vector<std::string>& in_species,
	std::vector<double>& in_speciesSus, std::vector<double>& in_speciesInf, bool in_pairwise)
// fills farm_map, farmList, and xylimits
{
	verbose = verboseLevel; // manual control to turn this down instead of: verboseLevel;

	speciesOnPrems = in_species;
	speciesSus = in_speciesSus;
	speciesInf = in_speciesInf;
	pairwiseOn = in_pairwise;
	committedFarms = 0;
	txEvaluated = 0;
	farm_map.reserve(850000);
	allCells.reserve(800);
	
	// read in file of premises
	int id, tempsize;
	double x, y;
	std::string fips;
	int fcount = 0;

	std::ifstream f(fname);
	if(!f){std::cout << "Premises file not found. Exiting..." << std::endl; exit(EXIT_FAILURE);}
	if(f.is_open())
	{
	if (verbose>0){std::cout << "Premises file open." << std::endl;}
		while(! f.eof())
		{
			std::string line;
			getline(f, line); // get line from file "f", save as "line"
			std::vector<std::string> line_vector = split(line, '\t'); // separate by tabs
			
			if(! line_vector.empty()) // if line_vector has something in it
			{
				str_cast(line_vector[0], id);
				fips = line_vector[1];
				if (!xyswitch){
					str_cast(line_vector[2], y);
					str_cast(line_vector[3], x);
				} else if (xyswitch){
					str_cast(line_vector[2], x);
					str_cast(line_vector[3], y);
				}
				// write farm pointer to private var farm_map
				farm_map[id] = new Farm(id, fips, x, y); 
				fcount++;
				// add species counts - check that number of columns is as expected
				if(line_vector.size() < 4+speciesOnPrems.size()){
					std::cout<<"ERROR (premises file & config 44-46) at line"<<std::endl;
					std::cout<< line <<std::endl;
					std::cout<< ": number of columns with animal populations don't match up with list of species provided."<<std::endl;
					std::cout<<"Exiting..."<<std::endl;
					exit(EXIT_FAILURE);
				}
				int colcount = 4; // populations should start at column 4
				for (auto& sp:speciesOnPrems){ // for each species
					str_cast(line_vector[colcount], tempsize); // assign number in column "colcount" to "tempsize"
					farm_map.at(id)->Farm::set_speciesCount(sp,tempsize); // set number for species at premises
					// if there are animals of this species, add to fips-species list to sort by population later
					if (tempsize>0){fipsSpeciesMap[fips][sp].emplace_back(farm_map.at(id));}
					colcount++;
				}
				double premSus = getFarmSus(farm_map.at(id));
				farm_map.at(id)->Farm::set_sus(premSus);
				double premInf = getFarmInf(farm_map.at(id));
				farm_map.at(id)->Farm::set_inf(premInf);
			
				// write farm pointer to fips map
				FIPSmap[fips].emplace_back(farm_map.at(id));
				
				// compare/replace limits of xy plane
				if (fcount>1){// if this is not the first farm
					if (x < std::get<0>(xylimits)){std::get<0>(xylimits) = x;} // x min
					else if (x > std::get<1>(xylimits)){std::get<1>(xylimits) = x;} // x max
					
					if (y < std::get<2>(xylimits)){std::get<2>(xylimits) = y;} // y min
					else if (y > std::get<3>(xylimits)){std::get<3>(xylimits) = y;} // y max
					}
				else {
					if (verbose>0){std::cout << "Initializing xy limits...";}
					xylimits = std::make_tuple(x,x,y,y);
					// initialize min & max x value, min & max y value
					} 

			} // close "if line_vector not empty"
		} // close "while not end of file"
	} // close "if file is open"
	if (verbose==2){
	std::cout << "x min = " << std::get<0>(xylimits) << std::endl;
	std::cout << "x max = " << std::get<1>(xylimits) << std::endl;
	std::cout << "y min = " << std::get<2>(xylimits) << std::endl;
	std::cout << "y max = " << std::get<3>(xylimits) << std::endl;}
	
	f.close();
	if (verbose>0){std::cout << fcount << " farms in " << FIPSmap.size() 
		<< " counties loaded. Premises file closed." << std::endl;}

	// copy farmlist from farm_map (will be changed as grid is created)
	if (verbose==2){std::cout << "Copying farms from farm_map to farmList..." << std::endl;}
	for (auto& prem: farm_map) {farmList.emplace_back(prem.second);} // "second" value from map is Farm pointer
	 
	// sort farmList by ID for faster matching/subset removal
	std::sort(farmList.begin(),farmList.end(),sortByID);
	// sort within each FIPS map element by farm size (population) for each species
	// (for use in Shipment manager, but this pre-calculates to save run time)
	for (auto& sp:speciesOnPrems){
		std::sort(fipsSpeciesMap[fips][sp].begin(),fipsSpeciesMap[fips][sp].end(),comparePop(sp)); // comparePop struct defined in Grid_manager.h
	}	
	
	// to do: calculate 
}

Grid_manager::~Grid_manager()
{
	for (auto f:farm_map){delete f.second;}
	for (auto gc:allCells){delete gc.second;}
}

std::vector<Farm*> Grid_manager::getFarms(std::tuple<int,double,double,double> cellSpecs, const unsigned int maxFarms/*=0*/)
// based on cell specs, finds farms in cell and saves pointers to farmsInCell
// to do: reformat data structure to a range tree for faster point-in-range searching
{
	if(verbose==2){std::cout << "Getting farms in cell..." << std::endl;}

    // cellSpecs[0] is placeholder for ID number, added when committed
    double x = std::get<1>(cellSpecs);
    double y = std::get<2>(cellSpecs);
    double s = std::get<3>(cellSpecs);
    std::vector<Farm*> inCell;
    inCell.reserve(2001);
    // look for farms in cell, those falling on grid boundaries are included, will be removed from list when cell is committed to avoid double counting
    
    for (auto i:farmList){
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

void Grid_manager::removeParent(std::stack< std::tuple<int,double,double,double> >& queue)
// The parent cell is the working cell 1st in the queue, so remove first element
{
    queue.pop();
}

void Grid_manager::addOffspring(std::tuple<int,double,double,double> cellSpecs, 
	std::stack< std::tuple<int,double,double,double> >& queue)
// offspring cells are quadrants of parent cell
{
	// cellSpecs[0] is placeholder for ID number
    double x = std::get<1>(cellSpecs);
    double y = std::get<2>(cellSpecs);
    double s = std::get<3>(cellSpecs);
    
    // lower left quadrant: same x/y, side/2
    auto lowerLeft = std::make_tuple(0, x, y, s/2);
    // lower right quadrant: add side/2 to x, same y, side/2
    auto lowerRight = std::make_tuple(0, x+s/2, y, s/2);
    // upper left quadrant: same x, add side/2 to y, side/2
    auto upperLeft = std::make_tuple(0, x, y+s/2, s/2);
    // upper right quadrant: add side/2 to x and y, side/2
    auto upperRight = std::make_tuple(0, x+s/2, y+s/2, s/2);
    
    // add offspring cells to queue (in reverse order so lower left is first)
    queue.emplace(upperRight);
    queue.emplace(upperLeft);
    queue.emplace(lowerRight);
    queue.emplace(lowerLeft);
}

void Grid_manager::commitCell(std::tuple<int,double,double,double> cellSpecs, std::vector<Farm*>& farmsInCell)
// write cellSpecs as class grid_cell into set allCells
{
	int id;
    double x, y, s;
    id = std::get<0>(cellSpecs);
    x = std::get<1>(cellSpecs);
    y = std::get<2>(cellSpecs);
    s = std::get<3>(cellSpecs);
    
    grid_cell* cellToAdd = new grid_cell(id, x, y, s, farmsInCell);
    
    allCells.emplace(id,cellToAdd); // add to map of all committed cells, with id as key
    committedFarms += farmsInCell.size();
    assignCellIDtoFarms(id,farmsInCell);
    removeFarmSubset(farmsInCell, farmList); // in shared_functions.cpp
}

void Grid_manager::splitCell(std::tuple<int,double,double,double>& cellSpecs, 
	std::stack< std::tuple<int,double,double,double> >& queue)
{
    queue.pop();
    addOffspring(cellSpecs,queue);
}

void Grid_manager::assignCellIDtoFarms(int cellID, std::vector<Farm*>& farmsInCell)
{
	for (auto& f:farmsInCell){
		f->Farm::set_cellID(cellID);
	}
}

void Grid_manager::initiateGrid(const unsigned int in_maxFarms, const int minCutoff)
// maxFarms: If cell contains at least this many farms, subdivision will continue
// minCutoff: minimum cell size
{
	set_maxFarms(in_maxFarms);
	std::cout << "Max farms set to " << maxFarms << std::endl;
    if(verbose>0){std::cout << "Splitting into grid cells..." << std::endl;}
 	int cellCount = 0;
    std::stack<std::tuple<int,double,double,double>> queue;// temporary list of cells to check for meeting criteria for commitment
    std::vector<Farm*> farmsInCell; // vector of (pointers to) farms in working cell - using vector to get to specific elements
        
    double min_x = std::get<0>(xylimits)-0.1;
    double max_x = std::get<1>(xylimits)+0.1;
    double min_y = std::get<2>(xylimits)-0.1;
    double max_y = std::get<3>(xylimits)+0.1;
    
    double side_x = max_x - min_x;
    double side_y = max_y - min_y;
    if(verbose==2)
    {
    	std::cout << "side_x = " << side_x << std::endl;
    	std::cout << "side_y = " << side_y << std::endl;
    }

    // use whichever diff is larger, x or y
    if (side_y > side_x)
       side_x = side_y; 
    if(verbose==2){std::cout << "Using larger value " << side_x << std::endl;}
    
    // add cell specifications to temporary tuple
    std::tuple<int,double,double,double> cellSpecs = std::make_tuple(cellCount, min_x, min_y, side_x);
    if(verbose==2){std::cout << "cellSpecs: " << std::get<0>(cellSpecs) <<", "<< std::get<1>(cellSpecs) 
    	<<", "<< std::get<2>(cellSpecs) <<", "<< std::get<3>(cellSpecs) << std::endl;}

    // add initial cell to the queue
    queue.emplace(cellSpecs);

    // while there are any items in queue
    while(queue.size()>0)
    {
    if(verbose==2){std::cout << std::endl << "Queue length = " << queue.size() << std::endl;}
    
    cellSpecs = queue.top(); // set first in queue as working cell

	if(verbose==2){
    	std::cout << "Cell side length = " << std::get<3>(cellSpecs) << ". ";
    }

	// Case A: side length of cell is smaller than kernel - immediate commit
	if (std::get<3>(cellSpecs) < minCutoff){ // if side < kernel diameter
		farmsInCell = getFarms(cellSpecs); // want ALL farms, so don't include maxFarms as argument
		if (farmsInCell.size() > 0){ // if there are farms in cell, commit
        	std::get<0>(cellSpecs) = cellCount;
        	commitCell(cellSpecs,farmsInCell);
        	cellCount = cellCount+1;
        	if (verbose==2){std::cout << "Side smaller than kernel diameter. Cell committed: #" << cellCount;}
        	queue.pop(); // remove parent cell from front of queue
        } else { // no farms in cell, remove from queue w/o committing
        	queue.pop(); // remove parent cell from front of queue
            if(verbose==2){std::cout << "No farms, removed cell, queue length = " << queue.size() << std::endl;}
        }
    // Case B: side length of cell >= minimum, check farm density and split if needed
    } else if (std::get<3>(cellSpecs) >= minCutoff){ // side >= kernel diameter
    	if(verbose==2){std::cout << "Side bigger than kernel, stepping in..." << std::endl;}
    	farmsInCell = getFarms(cellSpecs, maxFarms); // copy up to maxFarms farms in cell to farmsInCell)
    	if(verbose==2){std::cout << "Farms in cell = " << farmsInCell.size() << std::endl;}
        if (farmsInCell.size() >= maxFarms){
        // if farm density too high, split
			if(verbose==2){std::cout << "Too many farms, splitting cell..." << std::endl;}
			splitCell(cellSpecs,queue);
        }
        else if (farmsInCell.size() > 0 && farmsInCell.size() < maxFarms){
        // farm density is below maximum, commit
            	std::get<0>(cellSpecs) = cellCount;
                commitCell(cellSpecs,farmsInCell);
                cellCount = cellCount+1;
                if (verbose>1){std::cout << "Cell committed: #" << cellCount<<std::endl;}
                queue.pop(); // remove parent cell from front of queue
            }
        else if (farmsInCell.empty()){
        // cell has no farms at all - remove from queue w/o committing
            queue.pop(); // remove parent cell from front of queue
            if(verbose==2){std::cout << "No farms, removed cell, queue length = " 
            	<< queue.size() << std::endl;}
        }
    }
    } // end "while anything in queue"

	std::cout << "Grid of "<< allCells.size()<<" cells created, with min side "<<minCutoff<<
	" and max "<<maxFarms<<" farms. Pre-calculating distances..." << std::endl;
	if (farm_map.size()!=committedFarms){
		std::cout<<"ERROR: "<<committedFarms<<" farms committed, expected "
		<<farm_map.size()<<std::endl; 
		for (auto& dropped:farmList){std::cout<<"ID "<<dropped->get_id()<<", x "
			<<dropped->get_x()<<", y "<<dropped->get_y()<<std::endl;}
		exit(EXIT_FAILURE);
		}
	makeCellRefs();
	if(verbose>0){std::cout << "Grid initiated using density parameters. ";}
	
}

void Grid_manager::initiateGrid(std::string& cname)
// overloaded (alternate) constructor that reads in external file of cells
{
	// read in file of premises
	std::vector<Farm*> farmsInCell;

	std::ifstream f(cname);
	if(!f){std::cout << "Input file not found. Exiting..." << std::endl; exit(EXIT_FAILURE);}
	if(f.is_open())
	{
	if (verbose>0){std::cout << "File open" << std::endl;}
		while(! f.eof())
		{
			std::tuple<int,double,double,double> cellSpecs;
			std::string line;
			getline(f, line); // get line from file "f", save as "line"
			std::vector<std::string> line_vector = split(line, '\t'); // separated by tabs
			
			if(! line_vector.empty()) // if line_vector has something in it
			{ // convert each string piece to double
				if (verbose==2){std::cout << "Reading cell: ";}
				std::get<0>(cellSpecs)=stringToNum<int>(line_vector[0]); //id
				if (verbose==2){std::cout << std::get<0>(cellSpecs) << ", ";}
				std::get<1>(cellSpecs)=stringToNum<double>(line_vector[1]); //x
				if (verbose==2){std::cout << std::get<1>(cellSpecs) << ", ";}
				std::get<2>(cellSpecs)=stringToNum<double>(line_vector[2]); //y
				if (verbose==2){std::cout << std::get<2>(cellSpecs) << ", ";}
				std::get<3>(cellSpecs)=stringToNum<double>(line_vector[3]); //side
				if (verbose==2){std::cout << std::get<3>(cellSpecs) << ". ";}
				// line_vector[4] is num farms-ignored (gets reassigned)
				farmsInCell = getFarms(cellSpecs);
				if (verbose==2){std::cout << farmsInCell.size() << " farms assigned to cell." << 			
					std::endl;}
				if(farmsInCell.empty()){
					std::cout << "Cell " << std::get<0>(cellSpecs) << " has no farms - ignoring." << 				
						std::endl;
					// cell will not be added to list
					}
				else if (!farmsInCell.empty()){			
				// save cell with farms within
					allCells[std::get<0>(cellSpecs)] = new grid_cell(std::get<0>(cellSpecs), 
						std::get<1>(cellSpecs), std::get<2>(cellSpecs), std::get<3>(cellSpecs), farmsInCell); 
					assignCellIDtoFarms(std::get<0>(cellSpecs),farmsInCell);
					removeFarmSubset(farmsInCell, farmList);
					}
			} // close "if line_vector not empty"
		} // close "while not end of file"
	} // close "if file is open"
	f.close();
	if (verbose>0){std::cout << "File closed" << std::endl;}
	std::cout << allCells.size() << " cells loaded from file."<<std::endl;
	if (!farmList.empty()){
		Farm* f = farmList[0];
		std::cout << farmList.size() << " unassigned farms, first: " << f->get_id() << ": x=" << f->get_x() <<
			", y=" << f->get_y() << std::endl;
		}
	makeCellRefs();
}

void Grid_manager::initiateGrid(double cellSide)
{
    double min_x = std::get<0>(xylimits);
    double max_x = std::get<1>(xylimits);
    double min_y = std::get<2>(xylimits);
    double max_y = std::get<3>(xylimits);
    
    std::unordered_map<int, std::vector<Farm*>> cellFarmMap;
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
    
    std::vector<Farm*> farmListByX(farmList); // farmListByX is a copy of farmList
    std::sort(farmListByX.begin(), farmListByX.end(), sortByX);
    // indices for moving around the cell list
    int xi = 0; 
    int i = 0;
    int fcount = 0;
    
    std::vector<int> seedFarms;
        
    for (auto& f:farmListByX)
    {
		double farmx = f->Farm::get_x();
		double farmy = f->Farm::get_y();
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
   
   // commit all cells with farms
   bool printNumFarms = 0;
   std::string allLinesToPrint;
   int actualCellCount = 0;
   for (auto c=0; c!=cellCount; c++)
   {
	   if (cellFarmMap[c].size()>0){ // if there are any farms in this cell:
	   		allCells[actualCellCount] = new grid_cell(actualCellCount, xlist[c], ylist[c], cellSide, cellFarmMap[c]);
	   		assignCellIDtoFarms(actualCellCount,cellFarmMap[c]);
	   		actualCellCount++;
// 	   		if (printNumFarms){
// 	   			char temp[5];
// 	   			//sprintf(temp, "%u\n", cellFarmMap[c].size()); // use this on sweatshop computers
// 	   			sprintf(temp, "%lu\n", cellFarmMap[c].size());
// 	   			allLinesToPrint += temp;
// 	   		}
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
// temporarily disabled due to incompatible std::to_string use
{/*
	// sort cells by ID (ensures farm reassignment will be in same order as when created)
	std::map<double, grid_cell*> orderedCells(allCells.begin(),allCells.end());
	double firstcell = orderedCells.begin()->first;

	std::string tabdelim;
	tabdelim.reserve(orderedCells.size() * 50);
	for(auto it:orderedCells){
		tabdelim += to_string(*(it.second));
	}
	
	std::string ofilename;
//	ofilename ="unif";
 	char temp[5]; // make temporary storage
	sprintf(temp, "%i", maxFarms); // assign max # of farms as string
	ofilename += temp; // add to end of filename
	ofilename += "f_"; // add to end of filename
	int numCells = orderedCells.size(); // get number of cells
		sprintf(temp, "%i", numCells); // assign # of cells as string
	ofilename += temp;
	ofilename += "c_";
	ofilename += pfile;
	
	std::ofstream f(ofilename); 
	// will look something like "15f_932c_USprems.txt"
	if(f.is_open()){
		f << tabdelim;
		f.close();
	}
	std::cout << "Cells printed to " << ofilename <<std::endl;
*/
}

double Grid_manager::shortestCellDist2(grid_cell* cell1, grid_cell* cell2)
// returns shortest distance^2 between cell1 and cell2
{
	double cellDist2 = 0; // squared distance between cells
 	int cell1_id = cell1->grid_cell::get_id();
 	int cell2_id = cell2->grid_cell::get_id();
  if (cell1_id != cell2_id){ // else if comparing to self, distance is already 0 	
	double cell1_x = 0;
	double cell1_y = 0;
	double cell2_x = 0;
	double cell2_y = 0; // will use these points to calc distance

	double cell1_South = cell1->grid_cell::get_y(); // lower boundary of cell1
	double cell1_North = cell1->grid_cell::get_y()+cell1->grid_cell::get_s(); // upper boundary of cell1
	double cell1_West = cell1->grid_cell::get_x(); // leftmost boundary of cell1
	double cell1_East = cell1->grid_cell::get_x()+cell1->grid_cell::get_s(); // rightmost boundary of cell1
	
	double cell2_South = cell2->grid_cell::get_y(); // lower boundary of cell2
	double cell2_North = cell2->grid_cell::get_y()+cell2->grid_cell::get_s(); // upper boundary of cell2
	double cell2_West = cell2->grid_cell::get_x(); // leftmost boundary of cell2
	double cell2_East = cell2->grid_cell::get_x()+cell2->grid_cell::get_s(); // rightmost boundary of cell2
	
	// In comparing cell positions, due to nestedness of sub-cells, cell 1 could be:
	// Horizontally: W of, E of, or directly above/below all or part of cell2.
	// Vertically: N of, S of, or directly beside all or part of cell2.
	
	// Determine horizontal relationship and set x values accordingly:
	if(verbose>1){std::cout << "Cell " << cell1_id << " is ";}
	
	if (cell1_East <= cell2_West) // cell1 west of cell2
		{
 		if(verbose>1){std::cout << "west of and ";}
		cell1_x = cell1_East;
		cell2_x = cell2_West;
		}
	// or cell1 is east of cell2
	else if (cell1_West >= cell2_East)
		{
 		if(verbose>1){std::cout << "east of and ";}
		cell1_x = cell1_West;
		cell2_x = cell1_East;
		}
	// or cell1 is directly atop all or part of cell2
	else // if ((cell1_East > cell2_West) && (cell1_West < cell2_East))
		{
 		if(verbose>1){std::cout << "vertically aligned with and ";}
		//cell1_x = 0; // already initialized as 0
		//cell2_x = 0; // already initialized as 0
		// only use distance between y values
		}
	
	// Determine vertical relationship and set y values accordingly:
	if (cell1_South >= cell2_North) // cell1 north of cell2
		{
 		if(verbose>1){std::cout << "north of cell "<< cell2_id << std::endl;}
		cell1_y = cell1_South;
		cell2_y = cell2_North;
		}
	// or cell1 is below cell2
	else if (cell1_North <= cell2_South)
		{
 		if(verbose>1){std::cout << "south of cell "<< cell2_id << std::endl;}
		cell1_y = cell1_North;
		cell2_y = cell2_South;
		}
	// or cell1 is directly beside cell2
 	else // if ((cell1_South < cell2_North) && (cell1_North > cell2_South))
		{
 		if(verbose>1){std::cout << "horizontally aligned with cell "<< cell2_id << std::endl;}
		//cell1_y = 0; // already initialized as 0
		//cell2_y = 0; // already initialized as 0
		// only use distance between x values
		}	
	
	double xDiff = abs(cell1_x-cell2_x);
	double yDiff = abs(cell1_y-cell2_y);
	std::vector<double> orderedDiffs = orderNumbers(xDiff,yDiff); // in shared_functions.h
	if (storedDists.count(orderedDiffs[0])==1 && 
		storedDists.at(orderedDiffs[0]).count(orderedDiffs[1])==1){
		cellDist2 = storedDists.at(orderedDiffs[0]).at(orderedDiffs[1]);
	} else {
		cellDist2 = xDiff*xDiff + yDiff*yDiff;
 		storedDists[orderedDiffs[0]][orderedDiffs[1]] = cellDist2;
   	}
  } // end if cells 1 and 2 are different

return cellDist2;	
}

void Grid_manager::makeCellRefs()
// Fills maps (think vector of vectors) of calculations:
// gridCellKernel: kernel values between cell 1 (lower number) and cell 2 (higher number),
// kernelNeighbors: cells for which kernel value > 0 for cell 1.
// Although all the ID referencing seems a bit much, this is one way to ensure the order of the cells checked
{	
	for (unsigned int whichCell1=0; whichCell1 != allCells.size(); whichCell1++){		
		for (unsigned int whichCell2 = whichCell1; whichCell2 != allCells.size(); whichCell2++){
			// get distance between grid cells 1 and 2...
			// if comparing to self, distance=0
			double shortestDist2 = 0; 
			if (whichCell2 != whichCell1) {
			shortestDist2 = shortestCellDist2(allCells.at(whichCell1), allCells.at(whichCell2)); 
			}
			
			// to do: save adjacent neighbors
			
			// kernel value between c1, c2
			double gridValue = kernelsq(shortestDist2);
			
			// if grid Value is > 0, record in gridCellKernel and as kernel neighbors
			if (gridValue > 0){
				gridCellKernel[whichCell1][whichCell2] = gridValue; // store cell-cell kernel

				// store kernel * max sus (part of all prob calculations)
				double maxS2 = allCells.at(whichCell2)->grid_cell::get_maxSus();
				susxKern[allCells.at(whichCell1)][allCells.at(whichCell2)] = maxS2 * gridValue;
				if(verbose>1){
					std::cout << "Stored sus*kernel between cells "<<whichCell1<<" & "
					<<whichCell2<<", grid value: "<<gridValue<<std::endl;}
				// determine if pcell-cell >0 (in range)
				double maxI1 = allCells.at(whichCell1)->grid_cell::get_maxInf();
				int n2 = allCells.at(whichCell2)->grid_cell::get_num_farms();
				double pcellcell12 = 1-exp(-maxI1 * n2 * maxS2 * gridValue);
				if (pcellcell12 > 0){
					inRange[allCells.at(whichCell1)].emplace_back(allCells.at(whichCell2));
					if(verbose>1){std::cout << "Stored as in range: cells "<<whichCell1<<" & "<<whichCell2<<std::endl;}
					} else {if(verbose>1){std::cout << "Out of range: cells "<<whichCell1<<" & "<<whichCell2<<std::endl;}}
				// if not comparing to self, calc/store other direction (this was a big bug - double counting self as neighbor)	
				if (whichCell1 != whichCell2){ 
					double maxS1 = allCells.at(whichCell1)->grid_cell::get_maxSus();
					susxKern[allCells.at(whichCell2)][allCells.at(whichCell1)] = maxS1 * gridValue;
					if(verbose>1){std::cout << "Stored sus*kernel between cells "<<whichCell2<<" & "<<whichCell1<<std::endl;}
					double maxI2 = allCells.at(whichCell2)->grid_cell::get_maxInf();
					int n1 = allCells.at(whichCell1)->grid_cell::get_num_farms();
					double pcellcell21 = 1-exp(-maxI2 * n1 * maxS1 * gridValue);
					if (pcellcell21 > 0){
						inRange[allCells.at(whichCell2)].emplace_back(allCells.at(whichCell2));
						if(verbose>1){std::cout << "Stored as in range: cells "<<whichCell2<<" & "<<whichCell1<<std::endl;}
					} else {if(verbose>1){std::cout << "Out of range: cells "<<whichCell2<<" & "<<whichCell1<<std::endl;}}
				}
			} // end if gridValue > 0
		} // end for each cell2
	} // end for each cell1
	
 	if (verbose>0){
 		std::cout << "Kernel distances and neighbors recorded." << std::endl;
 	}
}

void Grid_manager::printGridKernel() const // only values > 0 stored
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
		cells_dists[0] = it.first; // cell 1
		cells_dists[1] = it2.first; // cell 2
		cells_dists[2] = it2.second; // kernel value
		
		for (auto i:cells_dists){
			sprintf(temp, "%f\t", i);
			tabdelim += temp;
		}
		tabdelim.replace(tabdelim.end()-1, tabdelim.end(), "\n"); // add line break
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

std::vector<grid_cell*> Grid_manager::IDsToCells(std::vector<int> cellIDs)
{
	std::vector<grid_cell*> neighborCells;
	for (auto& i:cellIDs)
	{
		neighborCells.emplace_back(allCells.at(i));
	}
	return neighborCells;
}

grid_cell* Grid_manager::IDsToCells(int cellID)
//overloaded to accept single ID also
{
	return allCells.at(cellID);
}

void Grid_manager::stepThroughCells(std::vector<Farm*>& in_focalFarms, std::vector<Farm*>& in_compFarms)
// in_focalFarms is all infectious premises
// in_compFarms is all susceptible premises
{
	//compsMade and cellCompsMade keep track of what's been compared and alerts if repeats occur
// 	std::unordered_map<double,std::vector<double>> compsMade;
// 	std::unordered_map<double,std::vector<double>> cellCompsMade; // key farm, vector of cells

	exposedFarms.clear();
	exposedFarms.reserve(in_compFarms.size());
	// make map of focal farms indexed by cells
	std::unordered_map<grid_cell*, std::vector<Farm*>> focalCellMap;
	for (auto& ff:in_focalFarms){ // ff is Farm*
		focalCellMap[allCells.at(ff->Farm::get_cellID())].emplace_back(ff);
	}
	if(verbose>1){std::cout << "Focal farm map assembled."<<std::endl;}
	
	// make map of comp farms indexed by cells
	std::unordered_map<grid_cell*, std::vector<Farm*>> compCellMap;
	for (auto& cf:in_compFarms){ // cf is Farm*
		compCellMap[allCells.at(cf->Farm::get_cellID())].emplace_back(cf);
	}
	if(verbose>1){std::cout << "Comparison farm map assembled."<<std::endl;}

	if(verbose>1){
		std::cout << compCellMap.size() << " cells contain " << in_compFarms.size() << " comparison farms: ";
	}
	
	// make map of farms in cells that are both in range and have comparison farms
	
	// loop through each focal cell
	for (auto& fc:focalCellMap){ 
	if (inRange.count(fc.first)>0){ // if any comparison cells are in range
		std::unordered_map<grid_cell*, std::vector<Farm*>> cellsToCheck; 
		std::vector<grid_cell*>& inRangeOfFocal = inRange.at(fc.first);
		if ( inRangeOfFocal.size()<allCells.size() ){
		// loop through all cells in range of kernel (includes self)
		for (auto& cc:inRangeOfFocal){
			// proceed only if cell contains comparison farms
			if (compCellMap.count(cc)>0){ // if in-range cell is in compCellMap
				cellsToCheck[cc]=compCellMap.at(cc); // add to list
			}
		}
		} else if ( inRangeOfFocal.size()==allCells.size() ){
			cellsToCheck.swap(compCellMap);
		}
		if(verbose==2 && cellsToCheck.size()<allCells.size()){std::cout << cellsToCheck.size() << " neighbor cells to check." << std::endl;}	

	// for each focal farm
	for (auto& f1:in_focalFarms){ 
		for (auto& c2:cellsToCheck){
			std::vector<Farm*>& farmsToCheck = c2.second;
			txEvaluated += farmsToCheck.size();
			// Evaluation via gridding
//			std::vector<Farm*> farmToCellExposures = countdownEval(f1,farmsToCheck);
			std::vector<Farm*> farmToCellExposures = binomialEval(f1,farmsToCheck);
			for (auto& fce:farmToCellExposures){ // add to infections from other farm-cell comparisons
				exposedFarms[fce].emplace_back(f1); // exposed farm is key, value is exposure source
				}

			if (pairwiseOn){
				double i = f1->get_inf();
				double pmax = 1 - exp(-i * susxKern[fc.first][c2.first]);
				pairwise pw(f1,farmsToCheck,pmax); // makes all pw calculations
				// check if same farms were infected in gridding and pairwise
				std::string pwCheck = pw.compare(farmToCellExposures); // tab delimited: # in agreement, # gridding only, # pairwise only	
				std::string outfile = "PWcompare.txt";
				printLine(outfile,pwCheck); //shared.cpp	
			} // end if pairwiseOn

		} // end for loop through comparison cells
	  } // end for each focal farm
	} // end if any comp cells are in range
	} // end for each focal cell
	if(verbose>1){
		std::cout<<"Exposures this time step (gridding): "<<exposedFarms.size()<<std::endl;
		std::cout<<"Total farms evaluated: "<<txEvaluated<<std::endl;
	}	
}

std::vector<Farm*> Grid_manager::countdownEval(Farm* focalFarm, std::vector<Farm*> compFarms)
{
	std::vector<Farm*> exposedFarmsInCell;
	exposedFarmsInCell.reserve(compFarms.size());
	
	double focalInf = focalFarm->Farm::get_inf();
	grid_cell* focalCell = allCells.at(focalFarm->Farm::get_cellID());
	grid_cell* compCell = allCells.at(compFarms[0]->Farm::get_cellID());
	int N = compFarms.size();
	double pcell = 1 - exp(-focalInf * susxKern[focalCell][compCell] * N); // susxKern is max susceptibility in comp cell times kernel @ distance betweeen focal and comp cell

	double s = 1; // on/off switch, 1 = on (single hypothetical infection hasn't happened yet)
	double random1 = unif_rand();
// Grid checkpoint A
	if (random1 <= pcell){ // if farm to cell succeeds
 		int f2count = 1; // how many farms in comparison cell have been checked
		// "prob6" in MT's Fortran code:
		double pmax = 1 - exp(-focalInf * susxKern[focalCell][compCell]);
		for (auto& f2:compFarms){
			double pcellAdj =1-(s*exp(-focalInf * susxKern[focalCell][compCell] * (N+1-f2count)));
			double random2 = unif_rand(); // "prob4" in MT's Fortran code
// Grid checkpoint B
			if (random2 <= pmax/pcellAdj){
			// if (one max susceptible)/(entrance prob accounting for # of farms checked) succeeds
			s = 0; // remainingFarmProb recalculates to 1 for remainder of loop
			// get actual distances between farms
			double f1x = focalFarm -> Farm::get_x();
			double f1y = focalFarm -> Farm::get_y();
			double f2x = f2 -> Farm::get_x();
			double f2y = f2 -> Farm::get_y();
			double xdiff = (f1x - f2x);
			double ydiff = (f1y - f2y);
			double distBWfarmssq = xdiff*xdiff + ydiff*ydiff;
			double kernelBWfarms = kernelsq(distBWfarmssq); // kernelsq calculates kernel based on distance squared
			double compSus = f2->Farm::get_sus(); // susceptible farm in comparison cell (farmInf already defined from focal cell)

			// calculate probability between these specific farms
			// "prob3" in MT's Fortran code
			double ptrue = 1-exp(-focalInf * compSus * kernelBWfarms);
// Grid checkpoint C
			if (random2 <= ptrue/pcellAdj){
				// infect
				if(verbose>1){
					std::cout << "Infection @ distance: ";
					std::cout << std::sqrt(distBWfarmssq)/1000 << ", prob "<<ptrue<<std::endl;
				}
				exposedFarmsInCell.emplace_back(f2);
			}
		 } // end "if farm hypothetically exposed"
		 f2count++;
		} // end "for each comparison farm"
	} // end "if >1 hypothetical infection"
	return exposedFarmsInCell;
}

std::vector<Farm*> Grid_manager::binomialEval(Farm* focalFarm, std::vector<Farm*> compFarms)
{
	// calculate pmax & N
	// draw from binomial
	// randomly choose that many farms
	// evaluate adjusted probs
	std::vector<Farm*> exposedFarmsInCell;
	exposedFarmsInCell.reserve(compFarms.size());
	
	double focalInf = focalFarm->Farm::get_inf();
	grid_cell* focalCell = allCells.at(focalFarm->Farm::get_cellID());
	grid_cell* compCell = allCells.at(compFarms[0]->Farm::get_cellID());
	double pmax = 1 - exp(-focalInf * susxKern[focalCell][compCell]); // susxKern is max susceptibility in comp cell times kernel @ distance betweeen focal and comp cell
	int N = compFarms.size();

	// draw number of hypothetical farms exposed, from binomial
	int numExp = draw_binom(N,pmax);
	
	if (numExp == 0){ // no infected premises in this cell
	} else if (numExp > 0){
		// randomly choose numExp farms
		std::vector<Farm*> hypExposed = random_unique(compFarms,numExp); 
		// evaluate each of the randomly selected farms
		for (auto& f2:hypExposed){
			// calc actual probabilities
			double f1x = focalFarm -> Farm::get_x();
			double f1y = focalFarm -> Farm::get_y();
			double f2x = f2 -> Farm::get_x();
			double f2y = f2 -> Farm::get_y();
			double xdiff = (f1x - f2x);
			double ydiff = (f1y - f2y);
			double distBWfarmssq = xdiff*xdiff + ydiff*ydiff;
			double kernelBWfarms = kernelsq(distBWfarmssq); // kernelsq calculates kernel based on distance squared
			double compSus = f2->Farm::get_sus(); // susceptible farm in comparison cell (farmInf already defined from focal cell)

			// calculate probability between these specific farms
			double ptrue = 1-exp(-focalInf * compSus * kernelBWfarms); // prob tx between this farm pair

			double random = unif_rand();
			if (random <= ptrue/pmax){ // actual infection
				if(verbose>1){
					std::cout << "Infection @ distance: ";
					std::cout << std::sqrt(distBWfarmssq)/1000 << ", prob "<<ptrue<<std::endl;
				}
				exposedFarmsInCell.emplace_back(f2);
			}
		 } // end "for each hypothetically exposed farm"
		} // end "if any hypothetically exposed farms"
	return exposedFarmsInCell;
}

std::vector<Farm*> Grid_manager::get_exposedFarms() const
{
	std::vector<Farm*> toReturn;
	toReturn.reserve(exposedFarms.size());
	for (auto& p:exposedFarms){toReturn.emplace_back(p.first);}
	return(toReturn);	
}

double Grid_manager::getFarmSus(Farm* f)
{
	// used in grid initialization
	// calculates species-specific susceptibility for a premises
	// USDOSv1 uses scaling factor (for q, susceptibility)
	// 2.086 x 10^-7, or that times sum of all US cattle: 19.619
	double adjQ = 0.0000002086;
	double premSus = 0;
	int i = 0; // use to keep up with speciesOnPrems element
	for (auto& sp:speciesOnPrems){
		double count = f->get_size(sp); // i.e. get_size("beef") gets # of beef cattle on premises
		double spSus = adjQ*speciesSus[i]*pow(count,speciesSus[i]); // multiply by stored susceptibility value for this species/type
		premSus += spSus; // add this species to the total for this premises
		i++;	
	}
	return premSus;
}

double Grid_manager::getFarmInf(Farm* f)
{
	// used in grid initialization
	// calculates species-specific infectiousness for a premises
	// USDOSv1 uses scaling factor (for p, transmissibility)
	// 2.177 x 10^-7, or that times sum of all US cattle: 20.483
	double adjP = 0.0000002177;
	double premInf = 0;
	int i = 0; // use to keep up with speciesOnPrems element
	for (auto& sp:speciesOnPrems){
		double count = f->get_size(sp); // i.e. get_size("beef") gets # of beef cattle on premises
		double spInf = adjP*speciesInf[i]*pow(count,speciesInf[i]); // susceptibility value for this species/type
		// confirm how to combine species! temporary solution:
		premInf += spInf; // add this species to the total for this premises
		i++;	
	}
	return premInf;
}