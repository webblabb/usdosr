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
// included in Grid_manager.h: grid_cell, farm, shared_functions, tuple, utility
#include "Grid_manager.h"

Grid_manager::Grid_manager(std::string &fname, bool xyswitch, std::vector<std::string>& in_species,
	std::vector<double>& in_speciesSus, std::vector<double>& in_speciesInf)
// fills farm_map, farmList, and xylimits
{
	verbose = 0; // manual control to turn this down instead of: verboseLevel;

	speciesOnPrems = in_species;
	speciesSus = in_speciesSus;
	speciesInf = in_speciesInf;
	// read in file of premises
	int id, tempsize;
	double x, y;
	std::string fips;
	int fcount = 0;

	std::ifstream f(fname);
	if(!f){std::cout << "Premises file not found." << std::endl;}
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
}

Grid_manager::~Grid_manager()
{
}

std::vector<Farm*> Grid_manager::getFarms(std::tuple<int,double,double,double> cellSpecs, const unsigned int maxFarms/*=0*/)
// based on cell specs, finds farms in cell and saves pointers to farmsInCell
{
	if(verbose==2){std::cout << "Getting farms in cell..." << std::endl;}

    // cellSpecs[0] is placeholder for ID number, added when committed
    double x = std::get<1>(cellSpecs);
    double y = std::get<2>(cellSpecs);
    double s = std::get<3>(cellSpecs);
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
    std::vector<Farm*> farms = farmsInCell;
    
    grid_cell* cellToAdd = new grid_cell(id, x, y, s, farms);
    
    allCells.emplace(id,cellToAdd); // add to map of all committed cells, with id as key
    assignCellIDtoFarms(id,farmsInCell);
    removeFarmSubset(farmsInCell, farmList);
}

void Grid_manager::splitCell(std::tuple<int,double,double,double>& cellSpecs, 
	std::stack< std::tuple<int,double,double,double> >& queue)
{
    removeParent(queue);
    addOffspring(cellSpecs,queue);
}

void Grid_manager::assignCellIDtoFarms(int cellID, std::vector<Farm*>& farmsInCell)
{
	for (auto& f:farmsInCell){
		f->set_cellID(cellID);
	}
}

void Grid_manager::initiateGrid(const unsigned int in_maxFarms, const int kernelRadius)
// maxFarms: If cell contains at least this many farms, subdivision will continue
// kernelRadius: maximum diffusion kernel radius (minimum cell size)
{
	set_maxFarms(in_maxFarms);
	std::cout << "Max farms set to " << maxFarms << std::endl;
    if(verbose>0){std::cout << "Splitting into grid cells..." << std::endl;}
 	int cellCount = 0;
    std::stack<std::tuple<int,double,double,double>> queue;// temporary list of cells to check for meeting criteria for commitment
    std::vector<Farm*> farmsInCell; // vector of (pointers to) farms in working cell - using vector to get to specific elements
        
    double min_x = std::get<0>(xylimits);
    double max_x = std::get<1>(xylimits);
    double min_y = std::get<2>(xylimits);
    double max_y = std::get<3>(xylimits);
    
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
    std::tuple<int,double,double,double> cellSpecs = {cellCount, min_x, min_y, side_x};
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
	if (std::get<3>(cellSpecs) < kernelRadius*2){ // if side < kernel diameter
		farmsInCell = getFarms(cellSpecs); // want ALL farms, so don't include maxFarms as argument
		if (farmsInCell.size() > 0){ // if there are farms in cell, commit
        	std::get<0>(cellSpecs) = cellCount;
        	commitCell(cellSpecs,farmsInCell);
        	cellCount = cellCount+1;
        	if (verbose==2){std::cout << "Side smaller than kernel diameter. Cell committed: #" << cellCount;}
        	removeParent(queue);
        } else { // no farms in cell, remove from queue w/o committing
        	removeParent(queue);
            if(verbose==2){std::cout << "No farms, removed cell, queue length = " << queue.size() << std::endl;}
        }
    // Case B: side length of cell >= kernel diameter, check farm density and split if needed
    } else if (std::get<3>(cellSpecs) >= kernelRadius*2){ // side >= kernel diameter
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
                if (verbose>0){std::cout << "Cell committed: #" << cellCount;}
                removeParent(queue);
            }
        else if (farmsInCell.empty()){
        // cell has no farms at all - remove from queue w/o committing
            removeParent(queue);
            if(verbose==2){std::cout << "No farms, removed cell, queue length = " 
            	<< queue.size() << std::endl;}
        }
    }
    } // end "while anything in queue"

	std::cout << "Grid of "<< allCells.size()<<" cells created, with min side "<<kernelRadius*2<<
	" and max "<<maxFarms<<" farms. Pre-calculating distances..." << std::endl;
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
				std::get<0>(cellSpecs)=stoi(line_vector[0]); //id
				if (verbose==2){std::cout << stoi(line_vector[0]) << ", ";}
				std::get<1>(cellSpecs)=stod(line_vector[1]); //x
				if (verbose==2){std::cout << stod(line_vector[1]) << ", ";}
				std::get<2>(cellSpecs)=stod(line_vector[2]); //y
				if (verbose==2){std::cout << stod(line_vector[2]) << ", ";}
				std::get<3>(cellSpecs)=stod(line_vector[3]); //side
				if (verbose==2){std::cout << stod(line_vector[3]) << ". ";}
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
   
   // write all cells with farms
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

// void Grid_manager::printVector(std::vector<Farm*>& vec, std::string& fname) const
// // temporarily disabled due to incompatible std::to_string use
// {
// /*
// 	std::string tabdelim;
// 	for(auto& it:vec){
// 		double fid = it->Farm::get_id();
// 		tabdelim += std::to_string(fid);
// 		tabdelim += "\n";
// 	}
// 	
// 	std::ofstream f(fname); 
// 	if(f.is_open()){
// 		f << tabdelim;
// 		f.close();
// 	}
// 	std::cout << "Vector printed to " << fname <<std::endl;
// */
// }

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
	if(verbose==2){std::cout << "Cell " << cell1_id << " is ";}
	
	if (cell1_East <= cell2_West) // cell1 west of cell2
		{
 		if(verbose==2){std::cout << "west of and ";}
		cell1_x = cell1_East;
		cell2_x = cell2_West;
		}
	// or cell1 is east of cell2
	else if (cell1_West >= cell2_East)
		{
 		if(verbose==2){std::cout << "east of and ";}
		cell1_x = cell1_West;
		cell2_x = cell1_East;
		}
	// or cell1 is directly atop all or part of cell2
//	else // if ((cell1_East > cell2_West) && (cell1_West < cell2_East))
//		{
 		if(verbose==2){std::cout << "vertically aligned with and ";}
		//cell1_x = 0; // already initialized as 0
		//cell2_x = 0; // already initialized as 0
		// only use distance between y values
//		}
	
	// Determine vertical relationship and set y values accordingly:
	if (cell1_South >= cell2_North) // cell1 north of cell2
		{
 		if(verbose==2){std::cout << "north of cell "<< cell2_id << std::endl;}
		cell1_y = cell1_South;
		cell2_y = cell2_North;
		}
	// or cell1 is below cell2
	else if (cell1_North <= cell2_South)
		{
 		if(verbose==2){std::cout << "south of cell "<< cell2_id << std::endl;}
		cell1_y = cell1_North;
		cell2_y = cell2_South;
		}
	// or cell1 is directly beside cell2
// 	else // if ((cell1_South < cell2_North) && (cell1_North > cell2_South))
// 		{
// 		if(verbose==2){std::cout << "horizontally aligned with cell "<< cell2_id << std::endl;}
		//cell1_y = 0; // already initialized as 0
		//cell2_y = 0; // already initialized as 0
		// only use distance between x values
//		}	
	
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
{	
	for (auto whichCell1=0; whichCell1 != allCells.size(); whichCell1++){		
		for (auto whichCell2 = whichCell1; whichCell2 != allCells.size(); whichCell2++){
			// get distance between grid cells 1 and 2...
			// if comparing to self, set distance=0
			double shortestDist2 = 0; 
			if (whichCell2 != whichCell1) {
			shortestDist2 = shortestCellDist2(allCells.at(whichCell1), allCells.at(whichCell2)); 
			}
			// kernel value between c1, c2
			double gridValue = kernelsq(shortestDist2);
			
			// if grid Value is > 0, record in gridCellKernel and as kernel neighbors
			if (gridValue > 0){
				gridCellKernel[whichCell1][whichCell2] = gridValue;
				// is max cell-cell prob>0?
				double maxI = IDsToCells(whichCell1)->grid_cell::get_maxInf();
				double maxS = IDsToCells(whichCell2)->grid_cell::get_maxSus();
				int n1 = IDsToCells(whichCell1)->grid_cell::get_num_farms();
				int n2 = IDsToCells(whichCell2)->grid_cell::get_num_farms();
				if(n1>n2){n2=n1;}
								
				double pcellcell = 1-exp(-maxI * n2 * maxS * gridValue);
				if (pcellcell > 0){
					kernelNeighbors[whichCell1].emplace_back(IDsToCells(whichCell2));
					if (whichCell1 != whichCell2){ // this was a big bug - double counting self as neighbor
					kernelNeighbors[whichCell2].emplace_back(IDsToCells(whichCell1));}
				} // end if prob cell-cell >0
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
// in_focalFarms is either all infectious (infectOut=1) or all susceptible (infectOut=0) premises
// in_focalFarms is either all susceptible (infectOut=1) or all infectious (infectOut=0) premises
{
	//compsMade and cellCompsMade keep track of what's been compared and alerts if repeats occur
// 	std::unordered_map<double,std::vector<double>> compsMade;
// 	std::unordered_map<double,std::vector<double>> cellCompsMade; // key farm, vector of cells

	infectedFarms.clear();
	// make map of focalp farms indexed by cells
	std::unordered_map<double, std::vector<Farm*>> focalCellMap;
	for (auto& ff:in_focalFarms){ // ff is Farm*
		focalCellMap[ff->Farm::get_cellID()].emplace_back(ff);
	}
	// make map of comp farms indexed by cells
	std::unordered_map<double, std::vector<Farm*>> compCellMap;
	for (auto& cf:in_compFarms){ // cf is Farm*
		compCellMap[cf->Farm::get_cellID()].emplace_back(cf);
	}
	if(verbose==2){
		std::cout << compCellMap.size() << " cells contain " << in_compFarms.size() << " comparison farms: ";
	}
		
   for (auto& fc1:focalCellMap){ 
    int fcID = fc1.first; // cell id
    if(verbose==2){std::cout << "Focal cell set to " << fcID << std::endl;}
	// get neighbor cells of focal cell (includes self)
	std::vector<grid_cell*>& neighborsOfFocal = kernelNeighbors.at(fcID);
	
	std::vector<grid_cell*> neighborsToCheck; neighborsToCheck.clear();
	for (auto& n: neighborsOfFocal){
		if (compCellMap.count( n->grid_cell::get_id() )>0){ // if this neighbor cell has comparison farms
			neighborsToCheck.emplace_back(n);}
		} // end for each neighbor cell
			if(verbose==2 && neighborsToCheck.size()<allCells.size()){std::cout << neighborsToCheck.size() << " neighbor cells to check." << std::endl;}	

	for (auto& f1:fc1.second){ // for each focal farm in this cell
		double farmFoc = getFarmInf(f1); // infectiousness value for focal farm
			if (!infectOut){farmFoc = getFarmSus(f1);} // susceptibility value for focal farm
			  
		for (auto& c2:neighborsToCheck){	// loop through each neighbor cell (c2 as in "cell 2")
			int compCellID = c2->grid_cell::get_id();
		
/*
			if (cellCompsMade.count(f1->get_id())>0){
				std::vector<double> tempList = cellCompsMade.at(f1->get_id()); // list of cell IDs at farm id key
				for (auto t:tempList){
					if(t==c2->get_id()){std::cout << std::endl << "Repeated farm-cell comparison. Cell: " << c2->get_id();}
				}
			} else {
				cellCompsMade[f1->get_id()].emplace_back(c2->get_id());
			} 
*/

			// get # of eligible farms in comp cell
			std::vector<Farm*>& compFarmList = compCellMap.at(compCellID); // added &, shaved 100sec/rep
			double compNumFarms = compFarmList.size();
			
			// put cell IDs in order, to look up cell-cell kernel value
			std::vector<int> ids = orderNumbers(fcID,compCellID); // in shared_functions.h
			double gridKernValue = 0; // default kernel value is 0
			if (gridCellKernel.at(ids[0]).count(ids[1]) == 1){ // something exists for this cell pair
				gridKernValue = gridCellKernel.at(ids[0]).at(ids[1]);}
			else {
				std::cout << std::endl << "Neighbor " << compCellID << " of farm "<< fcID
				<<" has no stored kernel value. ";}

			// maximum transmission values of cells
			double maxComp = c2->grid_cell::get_maxSus(); // max susceptibility of any farm in comparison cell
			// if calculating spread TO focal cell, reassign values
			if (!infectOut){maxComp = c2->grid_cell::get_maxInf();}

			double s = 1; // on/off switch, 1 = on (single infection hasn't happened yet)
			 // 1st "prob5" in MT's Fortran code:
			double farmToCellProb = 1 - exp(-farmFoc * compNumFarms*maxComp * gridKernValue);

			double random1 = unif_rand();
// Grid checkpoint A
			if (random1 < farmToCellProb){ // if farm to cell succeeds
				int f2count = 0; // how many farms in comparison cell have been checked
				// "prob6" in MT's Fortran code:
				// focal farm inf/sus * max sus/inf in comp cell * grid kernel
				double indivFarmMaxProb = 1 - exp(-farmFoc * maxComp * gridKernValue); 
		
			 for (auto& f2:compFarmList){
/*				
					if (f1->get_id()==f2->get_id()){std::cout<< "Self comparison. ";}
					if (compsMade.count(f1->get_id())>0){
						std::vector<double> tempList = compsMade.at(f1->get_id());
						for (auto t:tempList){
						if(t==f2->get_id()){std::cout << "Repeated farm-farm comparison. ";}
						}
					} else {
						compsMade[f1->get_id()].emplace_back(f2->get_id());
					}
*/ 
				f2count++;
				// 2nd "prob5" in MT's Fortran code: - replaces farmToCell while stepping through
				// # farms left in cell * farm(a) infectiousness * farm(b) susceptibility * grid kernel
				double remainingFarmsMaxProb =1-(s*exp(-farmFoc * (compNumFarms+1-f2count)*maxComp * gridKernValue));

				double random2 = unif_rand(); // "prob4" in MT's Fortran code
// Grid checkpoint B
				if (random2 < indivFarmMaxProb/remainingFarmsMaxProb){	
					// if (one max susceptible)/(entrance prob accounting for # of farms checked) succeeds
					s = 0; // remainingFarmProb recalculates to 1 for remainder of loop
					// get actual distances between farms
					double f1x = f1 -> Farm::get_x();
					double f1y = f1 -> Farm::get_y();
					double f2x = f2 -> Farm::get_x();
					double f2y = f2 -> Farm::get_y();
					double xdiff = (f1x - f2x);
					double ydiff = (f1y - f2y);
// 					double distBWfarms = sqrt(xdiff*xdiff + ydiff*ydiff);
//					double kernelBWfarms = kernel(distBWfarms);
					double distBWfarmssq = xdiff*xdiff + ydiff*ydiff;
					double kernelBWfarms = kernelsq(distBWfarmssq); // kernelsq calculates kernel based on distance squared
					// get individual infectiousness/susceptibility values
					double farmComp = getFarmSus(f2); // susceptible farm in comparison cell (farmInf already defined from focal cell)
				if (!infectOut){	
					farmComp = getFarmInf(f2); // infectious farm in comparison cell (farmSus already defined from focal cell)
				}
					// calculate probability between these specific farms
					double betweenFarmsProb = 1-exp(-farmFoc * farmComp * kernelBWfarms); // prob tx between this farm pair
					// "prob3" in MT's Fortran code
// Grid checkpoint C
					if (random2 < betweenFarmsProb/remainingFarmsMaxProb){
						// infect
						int compFarmID = f2->Farm::get_id();
						if(verbose==2){std::cout << "Farm infected. ";}
//						s = 0; // remainingFarmProb recalculates to 1 for remainder of loop
						if (infectedFarms.count(compFarmID)==0){ // if this farm hasn't been infected
							infectedFarms[compFarmID].emplace_back(1);
							//infectedFarms[f2->get_id()].emplace_back(f2x);
							//infectedFarms[f2->get_id()].emplace_back(f2y);
						} else {
							infectedFarms.at(compFarmID)[0]++; // would be infected again - keep count
						} // end if farm is already on infected list
		
					} // end "if actual tx occurs"
				} // end "if trans between indiv farms at max"
				} // end for each comparison cell farm
			} else { // otherwise if p(farm->cell) fails
				farmtocellskips++;
				farmsinskippedcells += compNumFarms;
			} // end if p(farm->cell) fails
		} // end for loop through comparison cells
	  } // end for each focal farm
	} // end for each focal cell
	if(verbose==2){
		std::cout<<"Infections this time step (gridding): "<<infectedFarms.size()<<std::endl;
	}
}

std::vector<Farm*> Grid_manager::get_infectedFarms() const
{
	std::vector<Farm*> toReturn;
	for (auto& p:infectedFarms){toReturn.emplace_back(farm_map.at(p.first));}
	return(toReturn);	
}

double Grid_manager::getFarmSus(Farm* f)
{
	// calculates species-specific susceptibility for a premises
	double premSus = 0;
	int i = 0; // use to keep up with speciesOnPrems element
	for (auto& sp:speciesOnPrems){
		double count = f->get_size(sp); // i.e. get_size("beef") gets # of beef cattle on premises
		double spSus = count * speciesSus[i]; // multiply by stored susceptibility value for this species/type
		premSus += spSus; // add this species to the total for this premises
		i++;	
	}
	return premSus;
}

double Grid_manager::getFarmInf(Farm* f)
{
	// calculates species-specific infectiousness for a premises
	double premInf = 0;
	int i = 0; // use to keep up with speciesOnPrems element
	for (auto& sp:speciesOnPrems){
		double count = f->get_size(sp); // i.e. get_size("beef") gets # of beef cattle on premises
		double spInf = count * speciesInf[i]; // multiply by susceptibility value for this species/type
		premInf += spInf; // add this species to the total for this premises
		i++;	
	}
	return premInf;
}
