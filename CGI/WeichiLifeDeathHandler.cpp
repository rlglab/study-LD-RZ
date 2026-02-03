#include "WeichiLifeDeathHandler.h"

void WeichiLifeDeathHandler::initialize()
{
    m_dragonHandler.initialize(this);
    m_openedAreasHandler.initialize(this);
    m_eyeHandler.initialize(this);
    m_quickwinHandler.initialize(this);

    return;
}

void WeichiLifeDeathHandler::reset()
{
    m_bmMutualCriticalPoints.Reset();
    return;
}

void WeichiLifeDeathHandler::findFullBoardLifeDeath()
{
    /*
            Steps
            1. Find connectors
            2. Use conectors to find dragons and snakes
            3. Find fullboard openedareas due to the dragons find above, and add to neighboring dragon when finding one.
            4. Use static rules to judge eyes for closedareas and openedareas of dragon
            5. Judge life by above data structures
    */

    // m_board.showColorBoard();

    reset();
    m_connectorHandler.findFullBoardConnectors(WeichiGlobalInfo::getTreeInfo().m_winColor);
    m_dragonHandler.findFullBoardBlockGroups();
    m_openedAreasHandler.findFullBoardOpenedAreasForDragon(WeichiGlobalInfo::getTreeInfo().m_winColor);
    m_openedAreasHandler.findFullBoardOpenedAreasForSnake(WeichiGlobalInfo::getTreeInfo().m_winColor);
    m_eyeHandler.findFullBoardEyes(WeichiGlobalInfo::getTreeInfo().m_winColor);
    judgeDragonLifeDeath();

    return;
}

string WeichiLifeDeathHandler::getAllDragonRZoneString()
{
    WeichiBitBoard bmRZone = getAllDragonRZoneBitBoard();

    ostringstream oss;
    int pos = 0;
    while ((pos = bmRZone.bitScanForward()) != -1) {
        oss << "[" << WeichiMove(COLOR_NONE, pos).toGtpString() << "]";
    }

    return oss.str();
}

WeichiBitBoard WeichiLifeDeathHandler::getAllDragonRZoneBitBoard()
{
    Vector<WeichiDragon, MAX_NUM_BLOCKS>& vDragons = getDragons();

    WeichiBitBoard bmRZone;
    for (int iDragon = 0; iDragon < vDragons.size(); ++iDragon) {
        WeichiDragon& dragon = vDragons[iDragon];
        bmRZone |= dragon.getRZone();
    }

    return bmRZone;
}

WeichiBitBoard WeichiLifeDeathHandler::getEyeRZone(int pos)
{
    Vector<WeichiDragon, MAX_NUM_BLOCKS>& vDragons = getDragons();

    WeichiBitBoard bmEye;
    for (int iDragon = 0; iDragon < vDragons.size(); ++iDragon) {
        WeichiDragon& dragon = vDragons[iDragon];
        for (int iEye = 0; iEye < dragon.getNumClosedAreas(); ++iEye) {
            WeichiClosedArea* eye = dragon.getClosedAreas()[iEye];
            if (eye->getEyeStatus() != EYE_UNKNOWN && eye->getStoneMap().BitIsOn(pos)) {
                bmEye |= eye->getRZone();
                break;
            }
        }
    }

    return bmEye;
}

WeichiBitBoard WeichiLifeDeathHandler::getAliveVirtualStone()
{
    Vector<WeichiDragon, MAX_NUM_BLOCKS>& vSnakes = getSnakes();
    WeichiBitBoard bmVirtualStone;
    for (int iSnake = 0; iSnake < vSnakes.size(); ++iSnake) {
        WeichiDragon& snake = vSnakes[iSnake];
        if (snake.getOpenedAreas().size() >= 2) {
            bmVirtualStone |= snake.getConnectorMap();
            break;
        }
    }

    return bmVirtualStone;
}

bool WeichiLifeDeathHandler::isBlockEarlyLife(const WeichiBlock* block)
{
    if (!block) { return false; }

    Vector<WeichiDragon, MAX_NUM_BLOCKS>& vDragons = getDragons();
    for (int iDragon = 0; iDragon < vDragons.size(); ++iDragon) {
        WeichiDragon& dragon = vDragons[iDragon];
        if (dragon.getStatus() != WeichiDragon::LIFESTATUS_ALIVE) { continue; }

        if (dragon.getStoneMap().hasIntersection(block->getStoneMap())) { return true; }
    }

    return false;
}

WeichiBitBoard WeichiLifeDeathHandler::getEarlyLifeBlockRZone(const WeichiBlock* block)
{
    if (!block) { return WeichiBitBoard(); }

    WeichiBitBoard bmRZone;
    Vector<WeichiDragon, MAX_NUM_BLOCKS>& vDragons = getDragons();
    for (int iDragon = 0; iDragon < vDragons.size(); ++iDragon) {
        WeichiDragon& dragon = vDragons[iDragon];
        if (dragon.getStatus() != WeichiDragon::LIFESTATUS_ALIVE) { continue; }

        if (dragon.getStoneMap().hasIntersection(block->getStoneMap())) {
            bmRZone = dragon.getRZone();
            break;
        }
    }

    return bmRZone;
}

HashKey64 WeichiLifeDeathHandler::getRepresentativeSkeletonHashkey()
{
    Vector<WeichiDragon, MAX_NUM_BLOCKS>& vDragons = getDragons();
    for (int iDragon = 0; iDragon < vDragons.size(); ++iDragon) {
        HashKey64 hashkey = vDragons[iDragon].getSkeletonHashKey();
        if (hashkey != HashKey64(0)) { return hashkey; }
    }

    return HashKey64();
}

WeichiBitBoard WeichiLifeDeathHandler::getRepresentativeSkeletonBitBoard()
{
    Vector<WeichiDragon, MAX_NUM_BLOCKS>& vDragons = getDragons();
    for (int iDragon = 0; iDragon < vDragons.size(); ++iDragon) {
        HashKey64 hashkey = vDragons[iDragon].getSkeletonHashKey();
        if (hashkey != HashKey64(0)) { return vDragons[iDragon].getSkeletonBitBoard(); }
    }

    return WeichiBitBoard();
}

void WeichiLifeDeathHandler::judgeDragonLifeDeath()
{
    Vector<WeichiDragon, MAX_NUM_BLOCKS>& vDragons = getDragons();
    for (int iDragon = 0; iDragon < vDragons.size(); ++iDragon) {
        WeichiDragon& dragon = vDragons[iDragon];
        WeichiBitBoard bmRZone;
        vector<WeichiClosedArea*> vCheckEyes;
        for (int iClosedArea = 0; iClosedArea < dragon.getNumClosedAreas(); ++iClosedArea) {
            WeichiClosedArea* ca = dragon.getClosedAreas()[iClosedArea];
            if (ca->getEyeStatus() == ONE_REGION_ENOUGH_LIVE) {
                dragon.setRZone(ca->getRZone());
                dragon.setNumEyes(2);
                dragon.setStatus(WeichiDragon::LIFESTATUS_ALIVE);
                break;
            } else if (ca->getEyeStatus() != EYE_UNKNOWN) {
                vCheckEyes.push_back(ca);
                bmRZone |= ca->getRZone();
                dragon.increaseNumTrueEyes();
            }
            if (dragon.getNumTrueEye() == 2) {
                // check conflicts
                if (vCheckEyes.size() == 2 && !checkMutualEyesSafety(vCheckEyes, bmRZone)) { break; }

                dragon.setRZone(bmRZone);
                dragon.setStatus(WeichiDragon::LIFESTATUS_ALIVE);
                break;
            }
        }
    }

    return;
}

bool WeichiLifeDeathHandler::checkMutualEyesSafety(vector<WeichiClosedArea*>& vEyes, WeichiBitBoard& bmRZone)
{
    WeichiBitBoard bmSurroundBlockLibs;
    for (int iCA = 0; iCA < vEyes.size(); ++iCA) {
        for (int iBlock = 0; iBlock < vEyes[iCA]->getNumBlock(); ++iBlock) {
            bmSurroundBlockLibs |= m_board.getBlock(vEyes[iCA]->getBlockID(iBlock))->getLibertyBitBoard(m_board.getBitBoard());
        }
    }

    Color ownColor = vEyes[0]->getColor();
    Color oppColor = AgainstColor(ownColor);

    WeichiBitBoard bmIntersection = vEyes[0]->getRZone() & vEyes[1]->getRZone();
    int pos = 0;
    while ((pos = bmIntersection.bitScanForward()) != -1) {
        const WeichiGrid& grid = m_board.getGrid(pos);
        if (grid.getColor() != COLOR_NONE) {
            bmIntersection -= grid.getBlock()->getStoneMap();
            continue;
        }

        WeichiMove attackMove(oppColor, pos);
        if (m_board.getLibertyAfterPlay(attackMove) >= 2 && bmSurroundBlockLibs.BitIsOn(pos)) {
            m_bmMutualCriticalPoints.SetBitOn(pos);
            return false;
        } else if (m_board.getLibertyAfterPlay(attackMove) == 1) {
            if (m_board.isKoEatPlay(attackMove)) {
                m_bmMutualCriticalPoints.SetBitOn(pos);
                return false;
            } else if (m_board.isKoMakePlay(attackMove)) {
                m_bmMutualCriticalPoints.SetBitOn(pos);
                return false;
            } else if (WeichiKnowledgeBase::isMakeKo(m_board, attackMove)) {
                m_bmMutualCriticalPoints.SetBitOn(pos);
                return false;
            }
            m_knowledge.computeKoRZoneByPattern(grid, ownColor, bmRZone);
        }
    }

    return true;
}
