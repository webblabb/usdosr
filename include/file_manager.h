#ifndef file_manager_h
#define file_manager_h

#include <fstream>
#include "shared_functions.h"
// shared_functions includes iostream, sstream, string, vector

extern bool verbose;

class file_manager
{
	private:
		std::vector<std::string> pv; // parameter vector
		
		bool checkMeanVar(std::string&,int,std::string);
		
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