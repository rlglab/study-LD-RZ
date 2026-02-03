#ifndef THREADSHAREDDATA_H
#define THREADSHAREDDATA_H

#include "TimeSystem.h"
#include "types.h"
#include "NodePtr.h"

/*!
	@brief  the shared data between MainThread and SlaveThread
	@author T.F. Liao
*/
template<class _UctNode>
struct ThreadSharedData 
{
	/*!
		@breif  reset the shared data
		@author T.F. Liao
	*/
	void reset () {
		isGeneratingMove = false;
		aborted = false ;
		poolFull = false ;
		timeOver = false ;
		simulateCount = 0u ;
		timer.reset();
		stopwatch.reset() ;
	}
	volatile bool isGeneratingMove;
	volatile bool aborted ;
	volatile bool poolFull ;
	volatile bool timeOver ;
	volatile uint simulateCount ;
	NodePtr<_UctNode> root ;
	Timer timer ;
	StopWatch stopwatch ;
};


#endif 