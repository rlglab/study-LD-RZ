#ifndef BASIC_TYPE_H
#define BASIC_TYPE_H

/************************************************************************/
/*  framework                                                           */
/************************************************************************/
#include "types.h"

/************************************************************************/
/*  STL                                                                 */
/************************************************************************/
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <string>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <queue>
#include <algorithm>
using namespace std;

/************************************************************************/
/*  standard C library                                                  */
/************************************************************************/
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>

/************************************************************************/
/* boost library                                                        */
/************************************************************************/
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

/************************************************************************/
/* Commom Utils                                                         */
/************************************************************************/
#include "HashKey64.h"
#include "Logger.h"
#include "WeichiBadMoveType.h"
#include "WeichiBlockSearchResult.h"
#include "WeichiConfigure.h"
#include "WeichiEdgeType.h"
#include "WeichiLifeAndDeathStatus.h"
#include "WeichiLadderType.h"
#include "WeichiMoveFeatureType.h"
#include "WeichiPlayoutPolicy.h"
//#include "WeichiRadiusPatternTable.h"
#include "WeichiRadiusPatternAttribute.h"
#include "WeichiSemeaiResult.h"
#include "WeichiCNNNetType.h"
#include "WeichiUctNodeStatus.h"
#include "WeichiSearchGoal.h"

/************************************************************************/
/* Limitations                                                          */
/************************************************************************/

// for data structure limit
const int MAX_BOARD_SIZE = 20;
const int MAX_NUM_GRIDS = MAX_BOARD_SIZE*MAX_BOARD_SIZE;
const int MAX_NUM_BLOCKS = MAX_NUM_GRIDS/2;
const int MAX_NUM_CONNECTORS = MAX_NUM_BLOCKS ;
const int MAX_NUM_DRAGONS = MAX_NUM_GRIDS/2;
const int MAX_NUM_CLOSEDAREA = MAX_NUM_GRIDS/2;
const int MAX_NUM_OPENEDAREA = MAX_NUM_GRIDS/2;
const int MAX_GAME_LENGTH = MAX_NUM_GRIDS*2;
const int MAX_NEW_CAND = 5*5 ;

// for closed area limit
const int MAX_CLOSEDAREA_SIZE = 7;
const int MAX_CLOSEDAREA_SCAN_STOP_TIMES = 13;	// for human calculate, should be re-calculate again if modify MAX_BASIC_CLOSEDAREA_SIZE
const int MAX_UCT_CLOSEDAREA_SIZE = 30;
const int MAX_OPENEDAREA_SIZE = 5;
const int MAX_OPENEDAREA_SCAN_STOP_TIMES = 8;

// for mcts limit
const int MAX_EXPAND_CHILD_NODE = 16;	// Uct expand max child node, one for pass
const int INIT_EXPAND_THRESHOLD = 1;	// Uct expand threshold

/************************************************************************/
/* alias type define                                                    */
/************************************************************************/

#include "BitBoard.h"
typedef BitBoard<MAX_BOARD_SIZE,MAX_BOARD_SIZE> WeichiBitBoard ;
typedef BitBoard<MOVE_FEATURE_SIZE,1> WeichiFixedFeatureBits ;
typedef BitBoard<EDGE_SIZE,1> WeichiEdgeBits ;
typedef BitBoard<POLICY_SIZE,1> WeichiPlayoutFeatureBits ;

/************************************************************************/
/* Re define assert                                                     */
/************************************************************************/
//使用assertToFile會將目前盤面(toString())存到檔案後再assert,第二個參數必須是WeichiBoard
#undef assertToFile
#ifdef NDEBUG
#define assertToFile(_Expression,board) ((void)0)
#else
#define assertToFile(_Expression,board) {if(!(_Expression)){ \
	ofstream fout("WeichiAssert.sgf", ios::out);\
	fout << board->toSgfFileString() << endl;\
	assert(_Expression);}\
	}
#endif

/************************************************************************/
/* constants                                                            */
/************************************************************************/

#define LEFT_IDX			0
#define UPPER_IDX			1
#define RIGHT_IDX			2
#define DOWN_IDX			3

#define START_ADJANCENT		0
#define END_ADJANCENT		3
#define NUM_ADJACENT		4

#define LEFT_UPPER_IDX      4
#define RIGHT_UPPER_IDX     5
#define RIGHT_DOWN_IDX      6
#define LEFT_DOWN_IDX       7

#define LEFT2_IDX			8
#define UPPER2_IDX			9
#define RIGHT2_IDX			10
#define DOWN2_IDX			11
#define NUM_DIRECTIONS		12

/// <cassert> should be included here since we may change the definition of NDEBUG.
#include <cassert>

/************************************************************************/
/* optimization                                                         */
/************************************************************************/
#define USE_BIT_FIELD 0

/************************************************************************/
/* additional define                                                    */
/************************************************************************/

#ifndef NULL
#define NULL    0
#endif

#if USE_BIT_FIELD
#define BIT_N(n)    :n
#else
#define BIT_N(n)
#endif

#endif /* BASIC_TYPE_H */ 
