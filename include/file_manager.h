#ifndef file_manager_h
#define file_manager_h

#include <fstream>
#include "shared_functions.h"
// shared_functions includes iostream, sstream, string, vector

extern int verboseLevel;

class file_manager
{
	private:
		int verbose;
		std::vector<std::string> pv; // parameter vector
		
		bool checkMeanVar(std::string&, int, std::string);
		bool checkPositive(std::vector<int>&, int);
		bool checkPositive(std::vector<double>&, int);

	public:
		file_manager();
		~file_manager();
		std::vector<std::string> getPV() const; // inlined
		void readConfig(std::string&);
};

inline std::vector<std::string> file_manager::getPV() const
{
	return pv;
}
#endif