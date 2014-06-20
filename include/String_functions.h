/*
A collection of methods having to do with handling string objects.

Currently contains

	-split(s,delimiter), splits s at the indexes where delimiter is found and
	 returns a vector of substrings.

	-str_cast(s, v), casts a number represented as a string and stores it in v,
	 v can be any type that can store numericals such as int, double etc.
*/

#ifndef STR_F_H
#define STR_F_H

#include <vector>
#include <string>
#include <sstream>

std::vector<std::string>& split(const std::string&, char, std::vector<std::string>&);
std::vector<std::string> split(const std::string&, char);

template<typename T> void str_cast(const std::string &s, T &ref)
{
	std::stringstream convert;
	convert << s;
	if(!(convert >> ref))
		ref = 0;
}

#endif //STR_F_H