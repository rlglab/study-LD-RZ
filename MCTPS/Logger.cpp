#include "Logger.h"
#include "Configure.h"
#include "TimeSystem.h"

void Logger::initialize()
{
	// if LogStderrFilename is not empty, log stdout and stderr to that file
	if ( Configure::LogConsoleFilename != "" ) {
		std::string sFormat;
		bool bEscape = false;
		bool bNeedEscape = true;
		for ( uint i = 0; i < Configure::LogConsoleFilename.size(); ++i ) {
			if ( bEscape ) { sFormat += Configure::LogConsoleFilename[i]; bEscape = false; continue; }
			if ( Configure::LogConsoleFilename[i] == '\\' ) { bEscape = true; continue; }
			if ( Configure::LogConsoleFilename[i] == ']' ) { bNeedEscape = true; continue; }
			if ( Configure::LogConsoleFilename[i] == '[' ) { bNeedEscape = false; continue; }
			if ( bNeedEscape ) { sFormat += '\\'; }
			sFormat += Configure::LogConsoleFilename[i];
		}
		fileBuffer.open(TimeSystem::getTimeString(sFormat), std::ios_base::out);
	}

	// do not print stderr if Configure::DisplayMessage is false
	if ( !Configure::DisplayMessage ) { teeErr.setStreambuf(nullptr, &Logger::fileBuffer); }
}

std::ostream& Logger::getTeeOut()
{
	return teeOut;
}

// public functions
std::ostream& COUT()
{
	if ( Configure::ShowTimestamp ) { Logger::teeOut << TimeSystem::getTimeString("[H:i:s.f] "); }
	if ( Configure::ShowDomain ) { Logger::teeOut << "[cout] "; }
	return Logger::teeOut;
}

std::ostream& CERR( std::string sDomain /*= ""*/ )
{
	if ( Configure::ShowTimestamp ) { Logger::teeErr << TimeSystem::getTimeString("[H:i:s.f] "); }
	if ( Configure::ShowDomain ) {
		Logger::teeErr << "[cerr] ";
		if ( sDomain.size() > 0 ) { Logger::teeErr << "[" << sDomain << "] "; }
	}
	return Logger::teeErr;
}

// initialization
std::filebuf Logger::fileBuffer;
TeeStream Logger::teeOut(std::cout.rdbuf(), &Logger::fileBuffer);
TeeStream Logger::teeErr(std::cerr.rdbuf(), &Logger::fileBuffer);