#include "Configure.h"
#include "strops.h"
#include <sstream>

namespace Configure
{
	using namespace std;
	/************************************************************************/
	/* Simulation Control                                                   */
	/************************************************************************/
	SimulationControl SimCtrl = SIMCTRL_TIME ;
	bool loadSimulationControl ( SimulationControl& ref, const std::string& key, const std::string& value )
	{
		std::string ucase_val = value ;
		strToUpper(ucase_val) ;
		using namespace Configure;
		if ( ucase_val == "COUNT" ) {
			ref = SIMCTRL_COUNT ;
		} else if ( ucase_val == "TIME" ) {
			ref = SIMCTRL_TIME ;
		} else if ( ucase_val == "MAX_NODECOUNT" ) {
			ref = SIMCTRL_MAXNODE_COUNT ;
		} else {
			return false;
		}
		return true; 
	}

	uint SimulationCountLimit = 10000 ;
	float SimulationTimeLimit = 1.0f ;
	uint SimulationMaxNodeCountLimit = 30000 ;
	bool Pondering;

	/************************************************************************/
	/* Thread Control                                                       */
	/************************************************************************/
	uint NumThread = 1 ;

	/************************************************************************/
	/* Buffer Size                                                          */
	/************************************************************************/
	size_t BitHashTable = 15;

	/************************************************************************/
	/* Node Manage                                                          */
	/************************************************************************/
	size_t MaxPageNum = 64 ;
	bool PruneAfterPlay = false ;
	bool ClearTreeAfterPlay = false ;

	/************************************************************************/
	/* UCT Configuration                                                    */
	/************************************************************************/
	float UCBWeight = 0.1f ;
	float ExpandThreshold = 1.0f; 

	/************************************************************************/
	/* randomization                                                        */
	/************************************************************************/
	bool UseTimeSeed = true ;
	uint RandomSeed = 0 ;

	/************************************************************************/
	/* display                                                              */
	/************************************************************************/
	bool DisplayMessage							= true;
	bool ShowDomain								= false;
	bool ShowTimestamp							= false;
	string LogConsoleFilename					= "console.txt";

	/************************************************************************/
	/* debugging                                                            */
	/************************************************************************/
	bool ShowSimulationCount = false;
	bool LogServerInformation = false;
	bool LogWorkerInformation = false;

	/************************************************************************/
	/* clustering                                                           */
	/************************************************************************/
	string server_host = "localhost" ;
	int server_port = 15501 ;

	/************************************************************************/
	/* period report                                                        */
	/************************************************************************/
	bool period_report = false;
	float period = 0 ;
	bool periodLoader( float& ref, const string& key, const string& value )
	{
		std::istringstream iss(value);
		iss >> ref ;
		if ( !iss || iss.rdbuf()->in_avail() != 0 )  
			return false;
		if ( ref > 0.1 ) 
			period_report = true; 
		return true;
	}

	/************************************************************************/
	/* Show Configure                                                       */
	/************************************************************************/
	void showConfigure ( std::ostream& os ) 
	{
		using namespace std;
		/************************************************************************/
		/* Simulation Control                                                   */
		/************************************************************************/
		os << "Simulation Control: " ;
		switch ( SimCtrl ) {
		case SIMCTRL_COUNT :
			os << "Total Count" << endl 
				<< "Maximum Simulation Count per move: " << SimulationCountLimit << endl ;
			break;
		case SIMCTRL_TIME :
			os << "Total Count" << endl 
				<< "Maximum Simulation Time per move: " << SimulationTimeLimit << endl ;
			break;
		case SIMCTRL_MAXNODE_COUNT :
			os << "Total Max Node Count" << endl
				<< "Maximum Simulation count node" << SIMCTRL_MAXNODE_COUNT << endl ;
			break;
		}

		/************************************************************************/
		/* Thread Control                                                       */
		/************************************************************************/
		os << "Number of Tread: " << NumThread << endl;

		/************************************************************************/
		/* Buffer Size                                                          */
		/************************************************************************/
		os << "Number of Bits for Each ThreadState HashTable: " << BitHashTable << endl;

		/************************************************************************/
		/* Node Manage                                                          */
		/************************************************************************/
		os << "Maximum Number of Nodes in NodePage: " << MaxPageNum << endl;

		/************************************************************************/
		/* UCT Configuration                                                    */
		/************************************************************************/
		os << "UCB Weight: " << UCBWeight << endl;
		os << "UCT Expand Threshold: " << ExpandThreshold << endl;

		/************************************************************************/
		/* randomization                                                        */
		/************************************************************************/
		os << "Use Time Seed: " << UseTimeSeed << endl;
		os << "Random Seed: " << RandomSeed << endl;

		/************************************************************************/
		/* debuging                                                             */
		/************************************************************************/
		os << "show simulation count: " << ShowSimulationCount << endl;

		/************************************************************************/
		/* period report                                                        */
		/************************************************************************/
		os << "period of report: " << period << endl;
	}

	std::string getConfigureString () 
	{
		std::ostringstream oss ;
		showConfigure(oss);
		return oss.str();
	}
} 
