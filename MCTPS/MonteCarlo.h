
#ifndef __MONTECARLO_H__
#define __MONTECARLO_H__

#include "BasePlayoutAgent.h"
#include "BaseUctAccessor.h"
#include "BaseUctNode.h"
#include "BasePrunePolicy.h"
#include "BaseDecisionMaker.h"
#include "BaseRootShifter.h"

#include "MasterThread.h"
#include "SlaveThread.h"
#include "ConfigureLoader.h"
#include "Configure.h"
#include "TimeSystem.h"

#include <string>

template<
	class _Move, 
	class _PlayoutResult, 
	class _ThreadGameState, 
	class _UctNode = BaseUctNode<_Move>, 
	class _MoveDecisionMaker = BaseDecisionMaker<_Move, _UctNode>,
	class _RootShifter = BaseRootShifter<_Move, _UctNode>,
	class _UctAccessor = BaseUctAccessor<_Move, _PlayoutResult, _UctNode, _ThreadGameState>,
	class _PlayoutAgent = BasePlayoutAgent<_Move, _PlayoutResult, _ThreadGameState>,
	class _PrunePolicy = BasePrunePolicy<_UctNode>,
	class _StopCondition = BaseStopCondition<_UctNode, _ThreadGameState>,
	class _SlaveThread = SlaveThread<_Move, _PlayoutResult, _ThreadGameState, _UctNode, _UctAccessor, _PlayoutAgent, _StopCondition>,
	class _MasterThread = MasterThread<_Move, _ThreadGameState, _SlaveThread, _UctNode, _MoveDecisionMaker, _RootShifter, _PrunePolicy, _StopCondition>
>
class MonteCarlo
{
protected:
	_MasterThread* m_masterThread ;

	static bool s_initialized ;

public:
	typedef _Move move_type ;
	typedef _ThreadGameState state_type ;
	typedef _UctNode node_type ;
public:

	MonteCarlo()
	{
		if ( !s_initialized ) {
			node_manage::PageAllocator<_UctNode>::initialize();
			s_initialized = true ;
		}
		m_masterThread = new _MasterThread(Configure::NumThread);
		newGame();
	}

	~MonteCarlo()
	{
		delete m_masterThread;
	}

	/*!
		@brief  initialize for new game
		@author T.F. Liao
	*/
	void newGame () 
	{
		m_masterThread->newGame() ;
	}
	
	/*!
		@breif  clear UCT tree
		@author kwchen
	*/
	void newTree ( ) 
	{
		m_masterThread->newTree();
	}

	/*!
		@brief  play a move
		@author T.F. Liao
		@param  move [in] move to be played
		@return true if valid move, false otherwise
	*/
	bool play ( _Move move )
	{
		return m_masterThread->play(move) ;
	}

	/*!
		@brief  generate move according to MCTS
		@author T.F. Liao
		@param  withPlay [in] set for generating a move without play it
		@return _Move chosen
	*/
	_Move genmove ( bool withPlay = true ) 
	{
		_Move m = m_masterThread->genmove();
		if ( withPlay ) {
			bool res = m_masterThread->play(m) ;
			assert ( res ) ;
		}
		return m ;
	}

	_Move selectMove ()
	{
		return m_masterThread->selectMove();
	}

	bool checkStopSearch ( const int sim_count, const float sim_time )
	{
		return m_masterThread->checkStopSearch( sim_count, sim_time );
	}

	StatisticData getRootUctData()
	{
		return m_masterThread->getRootUctData();
	}

	/*!
		@brief  get current state information for extra features
		@author T.F. Liao
		@return constant reference to ThreadStae of first SlaveThread
	*/
	const _ThreadGameState& getState ( uint index = 0 ) const 
	{
		return m_masterThread->getState(index);
	}

	uint getNumThreads() const
	{
		return m_masterThread->getNumThreads();
	}

	/*!
		@brief  set current state via a modifier (will be applied to thread state)
		@author T.F. Liao
		@param  modifier [in] functor to modify all thread state
	*/
	template<class _Modifier>
	bool modifyState ( const _Modifier& modifier ) 
	{
		return m_masterThread->modifyState(modifier) ;
	}

	void ponder_start()
	{
		m_masterThread->ponder_start();
	}

	void ponder_stop()
	{
		m_masterThread->ponder_stop();
	}

	void ponder_pause()
	{
		m_masterThread->ponder_pause();
	}

	void ponder_resume()
	{
		m_masterThread->ponder_resume();
	}

	void ponder_checkpool( bool forcePruning = false )
	{
		m_masterThread->ponder_checkpool( forcePruning );
	}
	template<class _UctTreeSerializer>
	std::string serializeTree(_UctTreeSerializer& serializer)
	{
		return m_masterThread->serializeTree(serializer) ;
	}

	template<class _UctTreeParser>
	std::string parseTree(_UctTreeParser& parser, const std::string& s)
	{
		return m_masterThread->parseTree(parser, s) ;
	}

	/*!
		@brief  set configure options in ConfigureLoader
		@author T.F. Liao
		@param cl [inout] ConfigureLoader that will add options into
	*/
	static void setConfigureOptions ( ConfigureLoader& cl ) 
	{
		using namespace Configure;
		cl.addOption ( "NUM_THREAD", NumThread, ToString(NumThread), 
			"number of thread", "Framework" ) ; 
		cl.addOption ( "MAX_PAGE", MaxPageNum, ToString(MaxPageNum), 
			"maximum number of node pages", "Framework" ) ; 
		cl.addOption ( "PRUNE_AFTER_PLAY", PruneAfterPlay, ToString(PruneAfterPlay), 
			"prune tree after each play", "Framework" ) ; 
		cl.addOption ( "CLEAR_TREE_AFTER_PLAY", ClearTreeAfterPlay, ToString(ClearTreeAfterPlay), 
			"clear tree after each play", "Framework" ) ; 

		cl.addOption ( "SIM_CONTROL", SimCtrl, "TIME", 
			"specify control method for simulation (COUNT/ TIME/ MAX_NODECOUNT)", "Framework", loadSimulationControl ) ; 
		cl.addOption ( "SIM_TIME_LIMIT", SimulationTimeLimit, ToString(SimulationTimeLimit), 
			"maximum simulation time (in second)", "Framework" ) ; 
		cl.addOption ( "SIM_COUNT_LIMIT", SimulationCountLimit, ToString(SimulationCountLimit), 
			"maximum simulation count", "Framework" ) ; 
		cl.addOption ( "SIM_MAXNODE_COUNT_LIMIT", SimulationMaxNodeCountLimit, ToString(SimulationMaxNodeCountLimit),
			"maximum node simulation count", "Framework" ) ;
		cl.addOption ( "PONDERING", Pondering, ToString(Pondering),
			"think when AI is idle", "Framework" ) ; 

		cl.addOption ( "UCB_WEIGHT", UCBWeight, ToString(UCBWeight),
			"coefficient for UCB term", "Framework") ;
		cl.addOption ( "EXPAND_THRESHOLD", ExpandThreshold, ToString(ExpandThreshold),
			"minimum number of simulation before expand children", "Framework") ;

		cl.addOption ( "USE_TIME_SEED", UseTimeSeed, ToString(UseTimeSeed),
			"use timestamp as random seed ", "Framework") ;
		cl.addOption ( "RANDOM_SEED", RandomSeed, ToString(RandomSeed),
			"manual set random seed", "Framework") ;

		cl.addOption ( "SHOW_SIM_COUNT", ShowSimulationCount, ToString(ShowSimulationCount),
			"show simulation count for each simulation", "Framework") ;

		cl.addOption( "REPORT_PERIOD", period, ToString(period),
			"set period to report search status (0 indicate no period report)", "Framework", periodLoader) ;

		/// Cluster Parallel
		cl.addOption ( "SERVER_HOST", server_host, ToString(server_host), 
			"host of server in cluster parallel (for worker mode)", "Cluster Parallel");
		cl.addOption ( "SERVER_PORT", server_port, ToString(server_port), 
			"port of server in cluster parallel (for both worker and server mode)", "Cluster Parallel");
		cl.addOption ( "LOG_SERVER", LogServerInformation, ToString(LogServerInformation), 
			"log from server side", "Cluster Parallel");
		cl.addOption ( "LOG_WORKER", LogWorkerInformation, ToString(LogWorkerInformation), 
			"log from worker side", "Cluster Parallel");

		// Display
		cl.addOption ( "DISPLAY_MESSAGE", DisplayMessage, ToString(DisplayMessage), "display stderr message", "Display" ) ;
		cl.addOption ( "SHOW_DOMAIN", ShowDomain, ToString(ShowDomain), "show domain name at start of each line", "Display");
		cl.addOption ( "SHOW_TIMESTAMP", ShowTimestamp, ToString(ShowTimestamp), "show timestamp at start of each line", "Display");
		cl.addOption ( "LOG_CONSOLE_FILENAME", LogConsoleFilename, ToString(LogConsoleFilename), "log stdout and stderr to this file, support time format(ex: console.txt, console_[Y_m_d_H_i_s].txt)", "Display");
	}
};

template<
	class _Move, 
	class _PlayoutResult, 
	class _ThreadGameState, 
	class _UctNode, 
	class _MoveDecisionMaker,
	class _RootShifter,
	class _UctAccessor,
	class _PlayoutAgent,
	class _PrunePolicy,
	class _StopCondition,
	class _SlaveThread,
	class _MasterThread
> bool MonteCarlo<_Move, _PlayoutResult, _ThreadGameState, _UctNode, _MoveDecisionMaker, _RootShifter, _UctAccessor, _PlayoutAgent, _PrunePolicy, _StopCondition, _SlaveThread, _MasterThread>::s_initialized = false ;


#endif