#ifndef __WEICHI_STOP_CONDITION_H__
#define __WEICHI_STOP_CONDITION_H__

#include "BaseStopCondition.h"
#include "WeichiUctNode.h"
#include "WeichiThreadState.h"

class WeichiStopCondition : public BaseStopCondition<class WeichiUctNode, class WeichiThreadState>
{
public:
	bool operator()(UctNodePtr root, const WeichiThreadState& state, int sim_count, float sim_time, bool isGeneratingMove);
};

#endif