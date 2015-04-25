//  grid_cell.h
//
// Defines grid_cell objects, each with coordinates of corner, susceptibility/infectiousness,
// vectors of farms within, neighbors, and kernel values to cells in range according to pcell-cell

#ifndef grid_cell_h
#define grid_cell_h

#include "farm.h"
#include "shared_functions.h" // for isWithin function in struct farmInList

class grid_cell
{
    private:
    	int id;
        double x, y, s, maxSus, maxInf; // x and y are coordinates for lower left corner
        std::vector<Farm*> farms;
        std::vector<grid_cell*> neighbors;
        std::unordered_map<int, double> susxKern; // index is int because cells are copied in replicates and referred to by id rather than pointer
    
    public:
		grid_cell(const int, const double, const double, const double, const std::vector<Farm*>);
		~grid_cell();

	// in alphabetical order by function name
        void addNeighbor(grid_cell*);
        bool canInfect(int) const; //inlined
        std::vector<Farm*> get_farms() const; // inlined - not pointers because modifiable copy is needed
		int get_id() const; //inlined
        double get_maxInf() const; //inlined
        double get_maxSus() const; //inlined
		const std::vector<grid_cell*>* get_neighbors(); //inlined
        double get_num_farms() const; // inlined
        double get_s() const; // inlined
        const std::unordered_map<int, double>* get_susxKernel(); //inlined
        double get_x() const; // inlined
        double get_y() const; // inlined
        double kernelTo(int) const; //inlined
        void removeFarmSubset(std::vector<int>&);
		void take_KernelValues(std::unordered_map<int, double>&);

};


inline bool grid_cell::canInfect(int) const {
	return (susxKern.count(id)==1);} 

inline std::vector<Farm*> grid_cell::get_farms() const {
	return farms;}
	
inline int grid_cell::get_id() const {
	return id;}
	
inline double grid_cell::get_maxInf() const {
	return maxInf;}
	
inline double grid_cell::get_maxSus() const {
	return maxSus;}
	
inline const std::vector<grid_cell*>* grid_cell::get_neighbors(){
	return &neighbors;}
	
inline double grid_cell::get_num_farms() const {
	return double(farms.size());}	
	
inline double grid_cell::get_s() const {
    return s;}
    
inline double grid_cell::get_x() const {
	return x;}

inline double grid_cell::get_y() const {
    return y;}

inline double grid_cell::kernelTo(int id) const {
	return susxKern.at(id);}
	
struct farmIDpresent // used in removeFarmSubset function with Farm*
{
	farmIDpresent(const std::vector<int> id_list) : ids(id_list) {} // constructor
	bool operator() (const Farm* f){ // overload operator function
		return (isWithin(f->Farm::get_id(),ids));
	}	
	private:
		std::vector<int> ids; // member
};
	
#endif
