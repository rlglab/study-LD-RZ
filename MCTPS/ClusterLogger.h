#ifndef CLUSTERLOGGER_H
#define CLUSTERLOGGER_H

#include <iostream>
#include <fstream>
#include <string>

#include "Logger.h"
#include "TimeSystem.h"

class ClusterLogger
{
public:
	enum MsgType {
		NET_IN,
		NET_OUT,
		PLUS,
		MINUS,
		COMMENT
	};

private:
	std::ofstream m_logFile;

public:
	ClusterLogger(){}

	~ClusterLogger()
	{
		closeFile();
	}

	void openFile( std::string filename )
	{
		if ( m_logFile.is_open() ) { closeFile(); }

		m_logFile.open(filename.c_str());
		if ( !m_logFile ) {
			CERR() << "open file " << filename + " fail" << std::endl;
		}
	}

	void closeFile()
	{
		m_logFile.close();
	}

	void logLine ( MsgType type, std::string msg )
	{
		if ( m_logFile ) {
			m_logFile << msgTypeToString(type) << " [" << TimeSystem::getTimeString("H:i:s.f") << "] " << msg << std::endl;
		}
	}

	std::string msgTypeToString ( MsgType type )
	{
		switch (type)
		{
		case NET_IN:   return ">>";
		case NET_OUT:  return "<<";
		case PLUS:     return "++";
		case MINUS:    return "--";
		case COMMENT:  return "==";
		default:       return "??";
		}
	}
};



#endif  
