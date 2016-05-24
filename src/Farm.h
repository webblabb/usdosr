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
inline County* Farm::get_parent_county() const
{
    return parent_county;
}

class Farm_type
{
public:
    Farm_type(std::string herd, std::vector<std::string> in_species);
    ~Farm_type();
    int get_index() const; //inlined
    std::string get_species() const; //inlined
private:
    static unsigned int types_created;
    unsigned int index = 0;
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

/// Derived class inheriting from Farm, containing additional info on replicate-specific statuses
/// and deleted at the end of each replicate. One object of this type is created for each
/// premises per simulation when it has any kind of status change (i.e. reporting, exposed,
/// prophylactic vaccination).
class Prem_status: public Farm
{
	private:
		std::string fileStatus; /// Current file status, one of: notDangerousContact, dangerousContact, reported
		std::string diseaseStatus; /// Current disease status, one of: exp, inf, imm
		std::unordered_map< std::string, std::string > controlStatus; /// Map with key: control status, value: string for that status

		std::unordered_map<std::string, int> start; /// Start times for each status. Assumes unique names among file, disease, and control statuses due to mapping by status name.
		std::unordered_map<std::string, int> end; /// End times for each status. Assumes unique names among file, disease, and control statuses due to mapping by status name.

	public:
		Prem_status(Farm*);
		~Prem_status();

 		std::string get_fileStatus() const; //inlined
 		std::string get_diseaseStatus() const; //inlined
		std::string get_controlStatus(std::string) const; //inlined

 		int get_start(std::string) const; //inlined
 		int get_end(std::string) const; //inlined

 		void set_fileStatus(const std::string); //inlined
 		void set_diseaseStatus(const std::string); //inlined
 		void set_controlStatus(const std::string, std::string); //inlined - set status for control stype

 		void set_start(const std::string, const int); //inlined - set start time for status
 		void set_end(const std::string, const int); //inlined - set end time for status

 		bool beenExposed() const; //inlined
};

inline std::string Prem_status::get_fileStatus() const
{
	return fileStatus;
}
inline std::string Prem_status::get_diseaseStatus() const
{
	return diseaseStatus;
}
inline std::string Prem_status::get_controlStatus(std::string controlType) const
{
	return controlStatus.at(controlType);
}
inline int Prem_status::get_start(std::string s) const
{
	return start.at(s);
}
inline int Prem_status::get_end(std::string s) const
{
	return end.at(s);
}
inline void Prem_status::set_fileStatus(const std::string status)
{
	fileStatus = status;
}
inline void Prem_status::set_diseaseStatus(const std::string stat)
{
	diseaseStatus = stat;
}
inline void Prem_status::set_controlStatus(const std::string controlType, std::string status)
{
	controlStatus[controlType] = status;
}
inline void Prem_status::set_start(const std::string status, const int t)
{
	start[status] = t;
}
inline void Prem_status::set_end(const std::string status, const int t)
{
	end[status] = t;
}
/// Checks whether or not a premises has been exposed to infection by checking for the presence of an "exp" start time
inline bool Prem_status::beenExposed() const
{
	return start.count("exp")==1;
}

#endif //FARM_H
