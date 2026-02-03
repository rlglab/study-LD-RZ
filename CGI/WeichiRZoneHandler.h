#ifndef H_WEICHIRZONE_HANDLER
#define H_WEICHIRZONE_HANDLER

#include "BasicType.h"
#include "WeichiBoard.h"
#include "WeichiGlobalInfo.h"

class WeichiRZoneHandler {

public:
    WeichiRZoneHandler() {}
    static WeichiBitBoard calculateBensonRZone(const WeichiBoard& board, WeichiMove move, bool bIncludeLiberty = false);
    static WeichiBitBoard calculateDeadRZone(const WeichiBoard& board, WeichiMove move);
};

#endif 
