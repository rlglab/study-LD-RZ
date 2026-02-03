#ifndef CONCEPTS_H
#define CONCEPTS_H

#include "WeichiMonteCarlo.h"
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

typedef WeichiMove Move ;
typedef WeichiMonteCarlo MCTS;

#endif 
