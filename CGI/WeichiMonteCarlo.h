#ifndef __WEICHIMONTECARLO_H__
#define __WEICHIMONTECARLO_H__

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "MonteCarlo.h"
#include "WeichiThreadState.h"
#include "WeichiMove.h"
#include "WeichiPlayoutResult.h"
#include "WeichiUctNode.h"
#include "WeichiMoveDecisionMaker.h"
#include "WeichiUctAccessor.h"
#include "WeichiPlayoutAgent.h"
#include "WeichiPrunePolicy.h"
#include "WeichiStopCondition.h"
#include "WeichiRootShifter.h"
#include "WeichiSlaveThread.h"
#include "WeichiMasterThread.h"

class WeichiMonteCarlo
	: public MonteCarlo<WeichiMove, WeichiPlayoutResult, WeichiThreadState, WeichiUctNode, WeichiMoveDecisionMaker, WeichiRootShifter, WeichiUctAccessor, WeichiPlayoutAgent, WeichiPrunePolicy, WeichiStopCondition, WeichiSlaveThread, WeichiMasterThread>
{
public:
	void replyStart( uint sessionId )
	{
		m_masterThread->replyStart(sessionId);
	}

	void setupJobs( uint threadId, std::string jobContent )
	{
		m_masterThread->setupJobs(threadId, jobContent);
	}

	void startCompute ( uint threadId )
	{
		m_masterThread->startCompute(threadId);
	}

	void broadcast( const std::string& msg, bool bBroadcastGPU=true )
	{
		m_masterThread->broadcast(msg, bBroadcastGPU);
	}
};


#endif