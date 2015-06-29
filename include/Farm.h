#ifndef FARM_H
#define FARM_H

#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>
#include "Point.h"

class County;
class State;
class Farm_type;

/// Describes a premises - one of these objects is created for each premises row read in
/// from the premises file.
class Farm
{
	protected: // allows access from derived class Prem_status
		int id,	///< Unique integer identifier read from premises file
			cellID; ///< Integer identifier of grid_cell assigned to this premises during grid creation
		double x_coordinate, ///< x-coordinate from projected longitude (same units as local spread kernel)
			y_coordinate, ///< y-coordinate from projected latitude (same units as local spread kernel)
			sus, ///< Calculated total susceptibility of this premises
			inf; ///< Calculated total infectiousness of this premises	
        Point position;
		County* parent_county;
		State* parent_state;
		Farm_type* farm_type;
		std::string fips; ///< County identifier (FIPS code)
		std::unordered_map< std::string, int > speciesCounts; ///< Numbers of animals of each type, keyed by types

	public:
		Farm(int, double, double, std::string);
		~Farm();
		int get_id() const; // inlined
		int get_cellID() const; // inlined
		Farm_type* get_farm_type() const; //inlined
		double get_x() const; // inlined
		double get_y() const; // inlined
		double get_sus() const; // inlined
		double get_inf() const; // inlined
 		std::string get_fips() const; // inlined
 		const std::unordered_map< std::string, int >* get_spCounts(); // inlined
 		int get_size(const std::string species) const;
		County* get_parent_county() const; //Inlined
 		State* get_parent_state() const; //Inlined
 		int get_n_shipments() const;
		void set_cellID(const int cellID);
		void set_farm_type(Farm_type* in_type);
 		void set_speciesCount(const std::string, int);
 		void set_sus(const double);
 		void set_inf(const double);
		void set_parent_county(County* in_county);
};

inline int Farm::get_id() const
{
	return id;
}
inline int Farm::get_cellID() const
{
	return cellID;
}
inline Farm_type* Farm::get_farm_type() const
{
    return farm_type;
}
inline double Farm::get_x() const
{
	return x_coordinate;
}
inline double Farm::get_y() const
{
	return y_coordinate;
}
inline double Farm::get_sus() const
{
	return sus;
}
inline double Farm::get_inf() const
{
	return inf;
}
inline std::string Farm::get_fips() const
{
	return fips;
}
const inline std::unordered_map< std::string, int >* Farm::get_spCounts()
{
	return &speciesCounts;
}

/// Derived class inheriting from Farm, containing additional info on infection statuses
/// and deleted at the end of each replicate. One object of this type is created for each
/// premises when it has any kind of status change (either disease or implemented control,
/// i.e. prophylactic vaccination).
class Prem_status: public Farm
{
	private:
		std::unordered_map< std::string, int > statuses; /// Map with key: control status, value: int level for that status (used by Control_actions)
		std::unordered_map< std::string, int > start; /// Start times for disease statuses (used by Status_manager)
		std::unordered_map< std::string, int > end; /// End times for disease statuses (used by Status_manager)
		std::string diseaseStatus; /// Current disease status (used by Status_manager)

	public:
		Prem_status(Farm*);
		~Prem_status();

 		int get_start(std::string) const; //inlined
 		int get_end(std::string) const; //inlined
 		std::string get_diseaseStatus() const; //inlined
 		bool beenExposed() const; //inlined
 		void set_control_status(const std::string, const int); //inlined - set level for control status
 		void set_start(const std::string, const int); //inlined - set start time for disease status
 		void set_end(const std::string, const int); //inlined - set end time for disease status
		void set_diseaseStatus(std::string&); //inlined

};

inline int Prem_status::get_start(std::string s) const
{
	return start.at(s);
}
inline int Prem_status::get_end(std::string s) const
{
	return end.at(s);
}
inline std::string Prem_status::get_diseaseStatus() const
{
	return diseaseStatus;
}
/// Checks whether or not a premises has been exposed to infection by checking for the presence of an "exp" start time
inline bool Prem_status::beenExposed() const
{
	return start.count("exp")==1;
}
inline void Prem_status::set_control_status(const std::string s, const int level)
{
	statuses[s] = level;
}
inline void Prem_status::set_start(const std::string status, const int t)
{
	start[status] = t;
}
inline void Prem_status::set_end(const std::string status, const int t)
{
	end[status] = t;
}
inline void Prem_status::set_diseaseStatus(std::string& stat)
{
	diseaseStatus = stat;
}
inline County* Farm::get_parent_county() const
{
    return parent_county;
}

class Farm_type
{
public:
    Farm_type(int index, std::string herd, std::vector<std::string> in_species);
    ~Farm_type();
    int get_index() const; //inlined
    std::string get_species() const; //inlined
private:
    int index = 0;
    std::string herd;
    std::string species; //ie dairy, beef

};

inline int Farm_type::get_index() const
{
    return index;
}

inline std::string Farm_type::get_species() const
{
    return species;
}

#endif //FARM_H
