#ifndef H_WEICHILIFEDEATH_HANDLER
#define H_WEICHILIFEDEATH_HANDLER

#include "BasicType.h"
#include "WeichiBoard.h"
#include "WeichiConnectorHandler.h"
#include "WeichiDragon.h"
#include "WeichiDragonHandler.h"
#include "WeichiEyeHandler.h"
#include "WeichiOpenedAreaHandler.h"
#include "WeichiQuickWinHandler.h"
#include "WeichiRZoneHandler.h"

class WeichiThreadState;

class WeichiLifeDeathHandler {
    friend class WeichiThreadState;
    friend class WeichiOpenedAreaHandler;

public:
    WeichiLifeDeathHandler(WeichiBoard& board, OpenAddrHashTable& ht)
        : m_board(board), m_ht(ht),
          m_knowledge(board, ht),
          m_dragonHandler(board, ht),
          m_connectorHandler(board, ht),
          m_openedAreasHandler(board, ht),
          m_eyeHandler(board, ht),
          m_quickwinHandler(board, ht)
    {
        initialize();
        reset();
    }

    void initialize();
    void reset();
    void findFullBoardLifeDeath();
    string getAllDragonRZoneString();
    WeichiBitBoard getAllDragonRZoneBitBoard();
    WeichiBitBoard getEyeRZone(int pos);
    WeichiBitBoard getAliveVirtualStone();
    bool isBlockEarlyLife(const WeichiBlock* block);
    WeichiBitBoard getEarlyLifeBlockRZone(const WeichiBlock* block);
    HashKey64 getRepresentativeSkeletonHashkey();
    WeichiBitBoard getRepresentativeSkeletonBitBoard();

    inline WeichiBitBoard getFullBoardImmediateWin() { return m_quickwinHandler.getFullBoardImmediateWin(); }
    inline WeichiBitBoard getFullBoardImmediateLoss() { return m_quickwinHandler.getFullBoardImmediateLoss(); }
    inline WeichiLifeDeathKnowledge& getKnowledge() { return m_knowledge; }
    inline Vector<WeichiDragon, MAX_NUM_BLOCKS>& getDragons() { return m_dragonHandler.getDragons(); }
    inline Vector<WeichiDragon, MAX_NUM_BLOCKS>& getSnakes() { return m_dragonHandler.getSnakes(); }
    inline WeichiConnectorHandler& getConnectorHandler() { return m_connectorHandler; }
    inline WeichiOpenedAreaHandler& getOpenedAreasHandler() { return m_openedAreasHandler; }
    inline WeichiQuickWinHandler& getQuickWinHandler() { return m_quickwinHandler; }
    inline string getEarlyLifeInfo() const { return m_earlyLifeInfo; }
    inline WeichiBitBoard getCriticalPoints() const { return m_bmMutualCriticalPoints; }

private:
    void judgeDragonLifeDeath();
    bool checkMutualEyesSafety(vector<WeichiClosedArea*>& vEyes, WeichiBitBoard& bmRZone);

private:
    WeichiBoard& m_board;
    OpenAddrHashTable& m_ht;
    WeichiLifeDeathKnowledge m_knowledge;
    WeichiDragonHandler m_dragonHandler;
    WeichiConnectorHandler m_connectorHandler;
    WeichiOpenedAreaHandler m_openedAreasHandler;
    WeichiEyeHandler m_eyeHandler;
    WeichiQuickWinHandler m_quickwinHandler;
    string m_earlyLifeInfo;
    WeichiBitBoard m_bmMutualCriticalPoints;
};

#endif
