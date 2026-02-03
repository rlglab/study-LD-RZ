#ifndef __MASTERTHREAD_H__
#define __MASTERTHREAD_H__

#include "boost/thread.hpp"

#include "SlaveThread.h"
#include "ThreadSharedData.h"
#include "PageAllocator.h"

#include "BasePrunePolicy.h"
#include "BaseStopCondition.h"
#include "BaseUctNode.h"
#include "NodePtr.h"
#include "NodePage.h"
#include "Vector.h"
#include "NodeRecycler.h"

#include "TimeSystem.h"

#include <iostream>
/*!
	@brief  MasterThread (Controller of Slave Threads)
	@author T.F. Liao
*/
template<
	class _Move,
	class _ThreadGameState, 
	class _SlaveThread,
	class _UctNode = BaseUctNode<_Move>,
	class _MoveDecisionMaker = BaseDecisionMaker<_Move, _UctNode>,
	class _RootShifter = BaseRootShifter<_Move, _UctNode>,
	class _PrunePolicy = BasePrunePolicy<_UctNode>,
	class _StopCondition = BaseStopCondition<_UctNode, _ThreadGameState>
>
class MasterThread
{
	typedef NodePtr<_UctNode> UctNodePtr ;
protected:
	Vector<_SlaveThread*, MAX_NUM_THREADS+MAX_NUM_WORKERS> m_vSlaveThreads ;
	boost::thread_group m_threads ;

	node_manage::PageAllocator<_UctNode>* m_pageAllocator ;

	UctNodePtr m_pRoot ;

	ThreadSharedData<_UctNode> m_threadData ;

	node_manage::NodePage<_UctNode>* m_page ; ///< first page with index=0, keep for NULL_PTR(0) and root node

	_MoveDecisionMaker m_decisionMaker ;
	_RootShifter m_rootShifter ;

	node_manage::NodeRecycler<_UctNode, _PrunePolicy> m_recycler ;

	_StopCondition m_condition;
	bool m_isRunning;

public:
	/*!
	@brief  constructor with number of slave threads
	@author T.F. Liao
	@param  numThread [in] specified number of slave threads
	*/
	MasterThread ( uint numThread = 1 ) 
	{
		assert ( numThread > 0 ) ;
		{
			using namespace Configure;
			if ( UseTimeSeed ) {
				RandomSeed = static_cast<uint>(time(NULL));
			}
			Random::reset(RandomSeed);
		}

		m_pageAllocator = node_manage::PageAllocator<_UctNode>::get();
		for ( uint i=0 ; i<numThread ; ++ i ) {
			_SlaveThread* slaveThread = new _SlaveThread ( i+1, m_threadData ) ;
			m_vSlaveThreads.push_back(slaveThread) ;
			m_threads.create_thread( boost::bind ( &_SlaveThread::run, slaveThread ) ) ;
		}
		m_isRunning = false ;
		// SlaveThread will initialize itself, so use newTree() instead of newGame()
		newTree();
	}
	/*!
		@brief  destructor
		@author T.F. Liao
	*/
	~MasterThread () 
	{
		terminateThreads() ;
		for ( uint i=0 ; i<m_vSlaveThreads.size() ; ++i ) {
			delete m_vSlaveThreads[i] ;
		}
	}

	/*!
		@brief  start a new game, reset all game state
		@author T.F. Liao
	*/
	void newGame()
	{
		bool isRunningBefore = m_isRunning;
		if ( Configure::Pondering && isRunningBefore ) ponder_stop();
		for ( uint i=0; i<m_vSlaveThreads.size(); ++i ) {
			m_vSlaveThreads[i]->reset();
		}
		newTree();
		if ( Configure::Pondering && isRunningBefore ) ponder_start();
	}

	/*!
		@breif  create new tree (extracted method)
		@author T.F. Liao
	*/
	void newTree ( ) 
	{
		assert(!m_isRunning);

		/// TODO: special handle first page, use minimal size of node
		m_pageAllocator->reset();

		m_page = m_pageAllocator->allocatePage();
		m_page->allocate(1) ; // address 0
		setRoot(m_page->allocate(1));
		m_pRoot->getUctData().add(0,Configure::ExpandThreshold);
	}

	/*!
		@brief  generate a move 
		@author T.F. Liao
		@return move generated
	*/
	_Move genmove ( ) 
	{
		compute();
		_Move m = m_decisionMaker(getState(), m_pRoot);
		summarize(m_pRoot, m);

		return m ;
	}

	_Move selectMove ( )
	{
		_Move m = m_decisionMaker(getState(), m_pRoot);
		return m ;
	}

	bool checkStopSearch ( const int sim_count, const float sim_time )
	{
		return m_condition( m_pRoot, getState(), sim_count, sim_time, m_threadData.isGeneratingMove );
	}

	StatisticData getRootUctData ()
	{
		return m_pRoot->getUctData();
	}

	/*!
		@brief  play a move 
		@author T.F. Liao
		@param  move [in] move to be played
		@return true if play successful, otherwise false
	*/
	bool play ( _Move move ) 
	{
		//m_rootShifter.verifyAfterPlay(m_pRoot,true);
		if ( !m_vSlaveThreads[0]->play(move) ) 
			return false ;

		for ( uint i=1 ; i<m_vSlaveThreads.size() ; ++i ) {
			bool res = m_vSlaveThreads[i]->play(move);
			assert ( res ) ;
			// if first thread think this transition is valid, all thread should think so
		}
		setRoot(m_rootShifter ( m_pRoot, move )) ;
		
		if ( m_pRoot.isNull() || Configure::ClearTreeAfterPlay ) {
			CERR()<<"clean tree after play\n";
			newTree();
		} else {
			if ( Configure::PruneAfterPlay ) {
				pruneTree(true) ;
			}
		}
		
		m_rootShifter.verifyAfterPlay(m_pRoot,true);
		return true ;
	}

	/*!
		@brief  get current state information for extra features
		@author T.F. Liao
		@return constant reference to ThreadStae of first SlaveThread
	*/
	const _ThreadGameState& getState ( uint index = 0 ) const 
	{
		return m_vSlaveThreads[index]->getState();
	}

	uint getNumThreads() const { return m_vSlaveThreads.size(); }

	/*!
		@brief  set current state via a modifier (will be applied to thread state)
		@author T.F. Liao
		@param  [in] functor to modify all thread state
	*/
	template<class _Modifier>
	bool modifyState ( const _Modifier& modifier ) 
	{
		if ( !m_vSlaveThreads[0]->modifyState(modifier) )
			return false ;
		for( uint i=1 ; i<m_vSlaveThreads.size() ; ++i ) {
			bool result = m_vSlaveThreads[i]->modifyState(modifier) ;
			assert(result) ;
		}
		return true;
	}

	void ponder_start()
	{
		/// if not pondering, start it
		if ( m_isRunning ) return;

		m_threadData.reset();
		m_threadData.stopwatch.run();
		m_isRunning = true ;

		/// start up SlaveThreads
		const_cast<UctNodePtr&>(m_threadData.root) = m_pRoot ;
		for ( uint i = 0; i < m_vSlaveThreads.size(); ++i ) {
			m_vSlaveThreads[i]->startCompute();
		}
	}

	void ponder_stop()
	{
		/// if pondering, stop it
		if ( !m_isRunning ) return;

		/// force SlaveThreads to stop
		m_threadData.aborted = true ; 
		for ( uint i = 0; i < m_vSlaveThreads.size(); ++i ) {
			m_vSlaveThreads[i]->waitComputeFinish();
		}

		m_isRunning = false ;

		m_threadData.stopwatch.stop();
		//// show total pondering time
	}

	void ponder_pause()
	{
		if ( !m_isRunning ) return;

		/// pause SlaveThreads
		m_threadData.aborted = true;
		for ( uint i = 0; i < m_vSlaveThreads.size(); ++i ) {
			m_vSlaveThreads[i]->waitComputeFinish();
		}
	}

	void ponder_resume()
	{
		if ( !m_isRunning ) return;

		/// resume SlaveThreads
		m_threadData.aborted = false;
		for ( uint i = 0; i < m_vSlaveThreads.size(); ++i ) {
			m_vSlaveThreads[i]->startCompute();
		}
	}

	void ponder_checkpool( bool forcePruning = false )
	{
		assert(m_isRunning) ;

		if ( m_threadData.poolFull || forcePruning ) {
			/// wait all SlaveThreads halt
			ponder_pause();

			pruneTree(false);

			/// start up SlaveThreads again
			const_cast<UctNodePtr&>(m_threadData.root) = m_pRoot ;
			ponder_resume();
		} else {
			/// assume that if pool is not full, Slaves should be running
			assert(!m_threadData.aborted) ;
		}
	}

	template<class _UctTreeSerializer>
	std::string serializeTree(_UctTreeSerializer& serializer)
	{
		bool isPonderingBefore = m_isRunning;
		if( isPonderingBefore ) {
			ponder_pause();
		}
		const std::string result = serializer(m_pRoot) ;
		if( isPonderingBefore ) {
			ponder_resume();
		}
		return result;
	}

	template<class _UctTreeParser>
	std::string parseTree(_UctTreeParser& parser, const std::string& s)
	{
		bool isPonderingBefore = m_isRunning;
		if( isPonderingBefore ) {
			ponder_pause();
		}
		const std::string parseStatus = parser(m_pRoot, s) ;
		if( isPonderingBefore ) {
			ponder_resume();
		}
		return parseStatus;
	}


protected:
	/*!
		@brief  terminate all compute threads, must be call in main state
		@author T.F. Liao
	*/
	void terminateThreads () 
	{
		for ( uint i=0 ; i<m_vSlaveThreads.size() ; ++i ) {
			m_vSlaveThreads[i]->terminate() ;
		}
		m_threads.join_all() ;
	}

	/*!
		@brief  routine to awake compute engines to start (extracted method)
		@author T.F. Liao
	*/
	void compute () 
	{
		m_threadData.reset();
		m_threadData.isGeneratingMove = true;
		m_threadData.stopwatch.run();
		if ( Configure::SimCtrl == Configure::SIMCTRL_TIME ) {
			m_threadData.timer.setLimit(Configure::SimulationTimeLimit ) ;
			m_threadData.timer.run();
		}

		while ( true ) { 
			const_cast<UctNodePtr&>(m_threadData.root) = m_pRoot ;
			for ( uint i = 0; i < m_vSlaveThreads.size(); ++i ) {
				m_vSlaveThreads[i]->startCompute();
			}
			for ( uint i = 0; i < m_vSlaveThreads.size(); ++i ) {
				m_vSlaveThreads[i]->waitComputeFinish();
			}

			if ( m_threadData.poolFull ) {
				CERR() << "pool full" << std::endl;
				break;
				pruneTree(false);
			} else if ( m_threadData.aborted ) {
				break; 
			}
		}
		m_threadData.stopwatch.stop();
		m_threadData.isGeneratingMove = false;
	}

	/*!
		@brief  summarize the computing history, invoke after genmove (extracted method)
		@author T.F. Liao
	*/
	void summarize (UctNodePtr root, _Move move)  
	{
		float timeUsed = (float)m_threadData.stopwatch.getTotalSeconds() ;
		int simulateCount = m_threadData.simulateCount ;
		getState().summarize(root, timeUsed, simulateCount, false, move);
	}

	/*!
		@brief  prune UCT (extracted method)
		@author T.F. Liao
		@param  after_play [in] indicate the pruning is invoked after a play or not
	*/
	virtual void pruneTree (bool after_play)
	{
		if ( !after_play ) {
			CERR() << "Prune tree, sim_count=" << m_threadData.simulateCount 
					  << ", time: " << m_threadData.stopwatch.getRunSeconds() << std::endl;
		}
		clock_t start_clock = clock();
		setRoot(m_recycler.reuse(m_pRoot, after_play)) ;
		m_threadData.poolFull = false ;
		m_threadData.aborted = false ;
		CERR() << "use " << (clock()-start_clock)/(double)CLOCKS_PER_SEC << " second(s)." << std::endl;
#if DUMP_TREE_AFTER_REUSE
		dumpTree(TimeSystem::getTimeString("ymdHis"));
#endif 
	}

#if DUMP_TREE_AFTER_REUSE
	void dumpTree ( std::string file, std::string prefix = string() ) 
	{
		std::ofstream fout ( (file+".sgf").c_str() ) ;
		fout << "(;GM[1]SZ[" << BOARD_SIZE << "]";
		fout << prefix ;
		dumpTree_r(fout, m_pRoot);
		fout << ")";
		fout.close();
	}

	void dumpTree_r ( std::ostream& os, UctNodePtr node, bool is_root = true )
	{
		if ( ! is_root ) 
			os << ";" << node->toString() ;

		for ( UctChildIterator<_UctNode> it(node); it ; ++it ) {
			os << "(" ;
			dumpTree_r(os, it, false );
			os << ")\n" ;
		}
	}
#endif
	virtual void setRoot(UctNodePtr pNode)
	{
		m_pRoot = pNode;
		const_cast<UctNodePtr&>(m_threadData.root) = m_pRoot ;
	}

};


#endif  
