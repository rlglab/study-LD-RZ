#ifndef _BASETHREADSTATE_
#define _BASETHREADSTATE_

#include "types.h"
#include "Vector.h"
#include "NodePtr.h"
#include "Logger.h"
#include <iostream>

/*!
	@brief  represents game state, store necessary information for any enhancement
	@author T.F. Liao
*/
template<class _Move, class _PlayoutResult,class _UctNode>
class BaseThreadState
{
	typedef NodePtr<_UctNode> UctNodePtr ;
private:

public:
	BaseThreadState ()
	{
	}

	Vector<UctNodePtr, MAX_TREE_DEPTH> m_path;
#if NATIVE_PTR_PATH
	Vector<_UctNode*, MAX_TREE_DEPTH> m_native_path ;
#endif
	uint m_thread_id ;

	//////////////////////////////////////////////////////////////////////////
	// implement the following methods if new data fields are added         //
	//////////////////////////////////////////////////////////////////////////
	void resetThreadState () {
		m_path.clear();
#if NATIVE_PTR_PATH
		m_native_path.clear() ;
#endif
	}
	void backup ( ) {
		m_path.clear();
#if NATIVE_PTR_PATH
		m_native_path.clear() ;
#endif
	}
	void rollback ( ) {
		m_path.clear();
#if NATIVE_PTR_PATH
		m_native_path.clear() ;
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	// implement the following method if default UctAccessor is used        //
	//////////////////////////////////////////////////////////////////////////
	void generateAllMoves ( Vector<_Move, MAX_NUM_CHILDREN>& moves ) {}

	//////////////////////////////////////////////////////////////////////////
	// implement the following methods if default playout agent is used     //
	//////////////////////////////////////////////////////////////////////////
	_Move generatePlayoutMove ( ) ;
	bool play ( _Move move ) ;
	bool isTerminal () const ;
	_PlayoutResult evaluate() ;

	//////////////////////////////////////////////////////////////////////////
	// implement the following methods to do something between MCTS phases  //
	//////////////////////////////////////////////////////////////////////////
	void setThreadID( int threadID ) { m_thread_id = threadID; }
	void startSimulation (int sim_count) {}
	void startPlayout() {}
	void endPlayout() {}
	void endSimulation() {}

	bool isMCTSThread() { return true; }
	/*!
		@brief  output summarization after genmove or every a period of time
		@author T.F. Liao
		@param  os  [in] output stream to be write 
		@param  root [in] root node of UCT
		@param  seconds [in] time comsumed after genmove
		@param  simulationCount [in] simulations done after genmove
		@param  period [in] boolean indicate this invocation in after genmove (period=false) or period invoke
		@param  move [in] if period is false, move indicate the move selected by MoveDecisionMaker
	*/
	void summarize (UctNodePtr root, float seconds, uint simulateCount, bool period=true, _Move move=_Move()) const 
	{
		if ( !period ) {
			CERR() << "Total " << simulateCount << " playouts"
					  << ", use " << seconds << " second(s)"
					  << ", average: " << simulateCount/seconds << " games/s" << std::endl;
		}
	} 

};


#endif 
