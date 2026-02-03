#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include "TeeStream.h"

extern std::ostream& COUT();
extern std::ostream& CERR( std::string sDomain = "" );

class Logger
{
	friend std::ostream& COUT();
	friend std::ostream& CERR( std::string sDomain );

private:
	static std::filebuf fileBuffer;

	static TeeStream teeOut; // output to stdout and file
	static TeeStream teeErr; // output to stderr and file

public:
	static void initialize();

	// for BaseGtpEngine
	static std::ostream& getTeeOut();
};

#endif