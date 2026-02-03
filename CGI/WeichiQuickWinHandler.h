#ifndef H_DIRECTWIN_HANDLER
#define H_DIRECTWIN_HANDLER

#include "BasicType.h"
#include "OpenAddrHashTable.h"
#include "Timer.h"
#include "WeichiBoard.h"
#include "WeichiDragon.h"
#include "WeichiRZoneHandler.h"

class WeichiLifeDeathHandler;

class WeichiQuickWinHandler {
    friend class WeichiLifeDeathGtpEngine;

public:
    WeichiQuickWinHandler(WeichiBoard& board, OpenAddrHashTable& ht) : m_board(board), m_ht(ht)
    {
        resetTimer();
    }

    void resetTimer() { m_timer.reset(); }
    inline void initialize(WeichiLifeDeathHandler* lifedeathHandler) { m_lifedeathHandler = lifedeathHandler; }

    WeichiBitBoard getFullBoardImmediateWin();
    WeichiBitBoard getFullBoardImmediateLoss();
    bool hasConnectorPotentialRZone(WeichiBitBoard& resultRZone);
    StopTimer m_timer;

private:
    WeichiBoard& m_board;
    OpenAddrHashTable& m_ht;
    WeichiLifeDeathHandler* m_lifedeathHandler;

    // For finding immediate win
    WeichiBitBoard getConnectPoints(WeichiDragon& dragon) const;
    WeichiBitBoard getSplitPoints(const WeichiClosedArea* closedarea) const;
    WeichiBitBoard getKillPoints(const WeichiClosedArea* closedarea) const;
    WeichiBitBoard getAtariPoints(Color oppColor) const;

    // For finding smaller benson
    bool hasPotentialRZone(const WeichiDragon& dragon, WeichiBitBoard& bmResult);
    bool hasBensonSequence(const WeichiDragon& dragon, WeichiBitBoard& bmPlayed, WeichiBitBoard& bmResult);
    WeichiBitBoard getPossibleEyeBitmap(const WeichiDragon& dragon) const;
};

#endif