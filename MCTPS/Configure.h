#ifndef CONFIGURE_H
#define CONFIGURE_H

#include "types.h"
#include <fstream>
#include <string>

namespace Configure
{
	/************************************************************************/
	/* Simulation Control                                                   */
	/************************************************************************/
	enum SimulationControl
	{
		SIMCTRL_TIME,
		SIMCTRL_COUNT,
		SIMCTRL_MAXNODE_COUNT
	};

	bool loadSimulationControl ( SimulationControl& ref, const std::string& key, const std::string& value );

	extern SimulationControl SimCtrl ;
	extern uint SimulationCountLimit ;
	extern float SimulationTimeLimit ;
	extern uint SimulationMaxNodeCountLimit ;
	extern bool Pondering;

	/************************************************************************/
	/* Thread Control                                                       */
	/************************************************************************/
	extern uint NumThread ;

	/************************************************************************/
	/* UCT Configuration                                                    */
	/************************************************************************/
	extern float UCBWeight ;
	extern float ExpandThreshold ;
	/************************************************************************/
	/* Buffer Size                                                          */
	/************************************************************************/
	extern size_t BitHashTable ;

	/************************************************************************/
	/* Node Manage                                                          */
	/************************************************************************/
	extern size_t MaxPageNum ;
	extern bool PruneAfterPlay ;
	extern bool ClearTreeAfterPlay ;
	/************************************************************************/
	/* randomization                                                        */
	/************************************************************************/
	extern bool UseTimeSeed ;
	extern uint RandomSeed ;

	/************************************************************************/
	/* display                                                              */
	/************************************************************************/
	extern bool DisplayMessage ;
	extern bool ShowDomain ;
	extern bool ShowTimestamp ;
	extern std::string LogConsoleFilename ;

	/************************************************************************/
	/* debugging                                                            */
	/************************************************************************/
	extern bool ShowSimulationCount ;
	extern bool LogServerInformation ;
	extern bool LogWorkerInformation ;

	/************************************************************************/
	/* clustering                                                           */
	/************************************************************************/
	extern std::string server_host ;
	extern int server_port ;

	/************************************************************************/
	/* period report                                                        */
	/************************************************************************/
	extern bool period_report ;
	extern float period ; 

	bool periodLoader(float& ref, const std::string& key, const std::string& value);

	/************************************************************************/
	/* Show Configure                                                       */
	/************************************************************************/
	void showConfigure ( std::ostream& os ) ;
	std::string getConfigureString () ;
}

#endif 
