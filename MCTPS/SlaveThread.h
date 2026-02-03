#ifndef __SLAVETHREAD_H__
#define __SLAVETHREAD_H__

#include "PageAllocator.h"
#include "ThreadSharedData.h"
#include "BaseUctNode.h"
#include "BaseUctAccessor.h"
#include "BasePlayoutAgent.h"
#include "NodePtr.h"
#include "BaseStopCondition.h"
#include "Logger.h"

#include <boost/thread/barrier.hpp>
#include <boost/thread/condition.hpp>

#include <iostream>

/*!
	@brief  SlaveThread (Compute Engine of MCTS)
	@author T.F. Liao
*/
template<
	class _Move,
	class _PlayoutResult,
	class _ThreadGameState,
	class _UctNode = BaseUctNode<_Move>,
	class _UctAccessor = BaseUctAccessor<_Move, _PlayoutResult, _UctNode, _ThreadGameState>,
	class _PlayoutAgent = BasePlayoutAgent<_Move, _PlayoutResult, _ThreadGameState>,
	class _StopCondition = BaseStopCondition<_UctNode, _ThreadGameState>
>
class SlaveThread
{
	typedef NodePtr<_UctNode> UctNodePtr ;
protected:
	uint m_threadId ;

	node_manage::PageAllocator<_UctNode>* m_pageAllocator ;
	node_manage::NodeAllocator<_UctNode> m_nodeAllocator ;
	
	_ThreadGameState m_state ;
	_UctAccessor m_uctAccessor ;
	_PlayoutAgent m_playoutAgent ;

	_StopCondition m_condition ;

	boost::barrier m_barStartPlay ;
	boost::barrier m_barFinishPlay ;
	boost::mutex m_exclusiveMutex;

	ThreadSharedData<_UctNode>& m_threadData ;

	bool m_bStateChanged ;
	float m_next_report_time ;
	
	volatile bool m_bIsInitialized;
	volatile bool m_bQuit ;

public:
	/*!
		@brief  constructor
		@author T.F. Liao
		@param  threadId    [in]    the id of the thread
		@param  threadData  [in]    shared info between MainThread and SlaveThreads
	*/
	SlaveThread ( uint threadId, ThreadSharedData<_UctNode>& threadData ) 
		: m_threadId ( threadId ), 
		m_pageAllocator ( node_manage::PageAllocator<_UctNode>::get() ),
		m_nodeAllocator(),
		m_state(), 
		m_uctAccessor ( m_state, m_nodeAllocator ), m_playoutAgent ( m_state ),
		m_condition(),
		m_barStartPlay ( 2 ), m_barFinishPlay ( 2 ),
		m_threadData ( threadData ),
		m_bIsInitialized ( false ),
		m_bQuit ( false ), m_bStateChanged ( true ),
		m_next_report_time(0.0f)
	{
	}
	
	/*!
		@brief  thread starting function
		@author T.F. Liao
	*/
	void run () 
	{
		initialize();
		while ( true ) {
			waitComputeStart();
			if ( isQuit() ) break ;
			boost::lock_guard<boost::mutex> lock(m_exclusiveMutex);

			m_uctAccessor.setRoot(m_threadData.root) ;
			m_next_report_time = m_threadData.stopwatch.getRunSeconds() + Configure::period ;

			if ( m_bStateChanged ) {
				/// if state changed, backup-state should be renew
				m_bStateChanged = false ;

				// start doing root filter (add by 5D)
				m_state.m_rootFilter.startFilter();
			}
			doPlayouts();
			
			finishCompute();
		}
	}

	/*!
		@brief  terminate threading
		@author T.F. Liao
	*/
	void terminate() 
	{
		m_bQuit = true ;
		startCompute() ;
	}

	/*!
		@brief  reset data as start of game
		@author T.F. Liao
	*/
	void reset () 
	{
		boost::lock_guard<boost::mutex> lock(m_exclusiveMutex);
		m_bStateChanged = true ;
		m_state.resetThreadState();
		m_nodeAllocator.invalidate();
	}
	/*!
		@brief  play a move on current game state
		@author T.F. Liao
		@return true if play successful, false indicate invalid move (state not changed)
	*/
	bool play ( _Move move ) 
	{
		boost::lock_guard<boost::mutex> lock(m_exclusiveMutex);
		if ( !m_state.play(move, true) ) 
			return false ;
		m_bStateChanged = true ;
		return true ; 
	}

	/*!
		@brief  get const refernece of game state
		@author T.F. Liao
		@return const reference of game state
	*/
	const _ThreadGameState& getState () const 
	{
		return m_state ;
	}

	/*!
		@brief  modify the game state
		@author T.F. Liao
		@param  modifier    [in]    functor that modify state
		the modifier should accept a parameter with reference to type _ThreadGameState
		and modify the _ThreadGameState
	*/
	template<class _Modifier>
	bool modifyState ( const _Modifier& modifier ) 
	{
		boost::lock_guard<boost::mutex> lock(m_exclusiveMutex);
		m_bStateChanged = true ;
		return modifier(m_state) ;
	}

	/*!
		@breif  resume computing of this thread (invoked by another thread)
		@author T.F. Liao
	*/
	void startCompute () 
	{
		m_barStartPlay.wait();
	}
	/*!
		@breif  wait for this thread stops (invoked by another thread)
		@author T.F. Liao
	*/
	void waitComputeFinish () 
	{
		m_barFinishPlay.wait();
	}

	bool isInitialized() volatile { return m_bIsInitialized; }
	bool isQuit() volatile { return m_bQuit; }
	void setQuit() volatile { m_bQuit = true; }

protected:

	/*!
		@brief  initialize internal data
		@author T.F. Liao
		some initialization must be executed by each thread itself (e.g. random seed)
	*/
	void initialize ()
	{
		if ( isInitialized() ) { return; }

		using namespace Configure;
		Random::reset(RandomSeed);
		reset();
		m_bIsInitialized = true;
		m_state.setThreadID(m_threadId);
	}

	/*!
		@brief  run simulations until time/ count limit exceeds or node pool is full
		@author T.F. Liao
	*/
	virtual void doPlayouts () 
	{
		m_state.backup();

		while ( !checkStopSearch() ) {

			if ( m_threadId == 1 && Configure::period_report ) {
				float timeUsed = m_threadData.stopwatch.getRunSeconds() ;
				if ( timeUsed >= m_next_report_time ) {
					UctNodePtr& root = const_cast<UctNodePtr&>(m_threadData.root);
					uint sim_count = m_threadData.simulateCount;
					m_state.summarize(root, timeUsed, sim_count, true);
					m_next_report_time = timeUsed + Configure::period ;
				}
			}

			int simulate_count =  m_threadData.simulateCount ;
			m_threadData.simulateCount++;
			simulate_count++;
			if ( Configure::ShowSimulationCount ) {
				CERR() << m_threadData.simulateCount << "           \r";
			}
			m_state.startSimulation(simulate_count) ;
			
			/// TODO: forecast pool-full event

			/// selection & expansion
			UctNodePtr node = m_uctAccessor.selectNode ( ) ;
			if ( m_nodeAllocator.hasAllocFail() ) {
				m_threadData.poolFull = true ;
				m_state.rollback();
				continue ;
			}
			/// simulation 
			m_state.startPlayout();
			_PlayoutResult result = m_playoutAgent.run ( ) ;
			m_state.endPlayout();
			
			/// back-propagation
			m_uctAccessor.update(result) ;
			
			m_state.endSimulation() ;

			m_state.rollback() ;
		}

		if ( m_threadData.timeOver )
			m_threadData.aborted = true ;
		m_state.rollback();
	}

	/*!
		@brief  check if search should stop, and return control to main thread
		@author T.F. Liao
		@return true if search should stop
	*/
	bool checkStopSearch () 
	{
		if ( m_threadData.aborted )
			return true;

		if ( m_threadId != 1 ) return false;
		/// check pool status
		if ( m_threadData.poolFull || m_pageAllocator->isOutOfPage() ) {
			m_threadData.poolFull = true ;
			m_threadData.aborted = true ;
			return true;
		}
		/// in pondering mode, don't care time/ sim_count limitation
		if ( !Configure::Pondering || m_threadData.isGeneratingMove ) {
			if ( Configure::SimCtrl == Configure::SIMCTRL_COUNT ) {
				if ( m_threadData.simulateCount >= Configure::SimulationCountLimit ) {
					m_threadData.timeOver = true ;
					m_threadData.aborted= true ;
					return true ;
				}
			} else if ( Configure::SimCtrl == Configure::SIMCTRL_TIME ) {
				if ( m_threadData.timeOver || m_threadData.timer.timeUp() )  {
					m_threadData.timeOver = true ;
					m_threadData.aborted= true ;
					return true ;
				}
			} else if( Configure::SimCtrl == Configure::SIMCTRL_MAXNODE_COUNT ) {
				UctChildIterator<_UctNode> it(m_threadData.root);
				uint max_count = 0 ;
				for(  ; it ; ++it ) {
					uint count = (uint)(it->getUctData().getCount()) ; 
					if( count > max_count )
						max_count = count ;
				}
				if( max_count >= Configure::SimulationMaxNodeCountLimit ) {
					m_threadData.timeOver = true ;
					m_threadData.aborted= true ;
					return true ;
				}
			}
		}
		if ( m_condition(m_threadData.root, m_state, m_threadData.simulateCount, m_threadData.stopwatch.getRunSeconds(), m_threadData.isGeneratingMove) ) {
			m_threadData.aborted = true ;
			return true;
		}
		return false;
	}

	/*!
		@brief  finish computing (extracted method to clarify code)
		@author T.F. Liao
	*/
	void finishCompute () 
	{
		m_uctAccessor.beforeChangeTree();
		m_barFinishPlay.wait();
	}
	/*!
		@brief  wait for MainThread to start it (extracted method to clarify code)
		@author T.F. Liao
	*/
	void waitComputeStart () 
	{
		m_barStartPlay.wait();
	}
};

#endif 
