#ifndef WEICHI_GTP_UTIL
#define WEICHI_GTP_UTIL

#include "BasicType.h"
#include <vector>
#include "WeichiMove.h"
#include <string>
#include "MoveLocation.h"
#include "DbMove.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace std ;

namespace WeichiGtpUtil
{
	vector<WeichiMove> getHandiCapStones(uint boardSize, uint stoneNum);
	MoveLocation WeichiMoveToLocation( WeichiMove wMove );
	WeichiMove dbMoveToWeichiMove( Color c, DbMove dMove );
}

#endif