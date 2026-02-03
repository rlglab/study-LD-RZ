#ifndef BASESTOPCONDITION
#define BASESTOPCONDITION

#include "BaseUctNode.h"
#include "NodePtr.h"

template<class _UctNode, class _ThreadGameState>
class BaseStopCondition
{
public:
	typedef NodePtr<_UctNode> UctNodePtr ;
	/*!
		@brief  functor determines if extra condition to stop search is satisfied
		@author T.F. Liao
		@param  root    [in] the root node
		@param  state   [in] the game state
		@param  sim_count [in] the total simulation count for this genmove
		@return true if extra condition to stop search is satisfied
		the default implementation always return false
	*/
	bool operator()(UctNodePtr root, const _ThreadGameState& state, int sim_count, float sim_time, bool isGeneratingMove) { return false; }
};

#endif 
