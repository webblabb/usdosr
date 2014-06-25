//
//  grid_cell.h
//
// Defines grid_cell objects, each with coordinates of corners and vector of farms within

#ifndef grid_cell_h
#define grid_cell_h

#include <vector>
#include "farm.h" // for class Farm

class grid_cell
{
	struct Point
		{ //Representation of a point
		double x, y; //Coordinates
	
		Point(){};
		Point(const double in_x, const double in_y){
			x = in_x;
			y = in_y;
			}; //Constructor
	};

    private:
        double id, x, y, s, maxSus, maxInf; // x and y are coordinates for lower left corner
        std::vector<Farm*> farms;
		std::vector<Point*> corners;
    
    public:
		grid_cell(const double, const double, const double, const double, const std::vector<Farm*>);
		~grid_cell();
		double get_id() const; //inlined
        double get_x() const; // inlined
        double get_y() const; // inlined
        double get_s() const; // inlined
        std::vector<Farm*> get_farms() const; // inlined
        double get_num_farms() const; // inlined
        std::vector<grid_cell::Point*> get_corners() const; // inlined
        double get_maxSus() const; //inlined
        double get_maxInf() const; //inlined
        
        std::string to_string() const;

};

inline double grid_cell::get_id() const
{
    return id;
}

inline double grid_cell::get_x() const
{
    return x;
}

inline double grid_cell::get_y() const
{
    return y;
}

inline double grid_cell::get_s() const
{
    return s;
}

inline std::vector<Farm*> grid_cell::get_farms() const
{
    return farms;
}

inline double  grid_cell::get_num_farms() const
{
	return double(farms.size());
}

inline std::vector<grid_cell::Point*> grid_cell::get_corners() const
{
	return corners;
}

inline double grid_cell::get_maxSus() const
{
    return maxSus;
}

inline double grid_cell::get_maxInf() const
{
    return maxInf;
}

#endif
