#ifndef Control_actions_h
#define Control_actions_h

#include "file_manager.h" // For parameters struct. Also includes shared functions.

extern int verboseLevel;

class Control_actions
{
	private:
		int verbose; ///< Can be set to override global setting for console output
		
	public:
		Control_actions(const Parameters*);
		~Control_actions();

};


#endif
