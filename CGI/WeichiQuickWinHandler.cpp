#include "WeichiQuickWinHandler.h"
#include "WeichiLifeDeathHandler.h"

WeichiBitBoard WeichiQuickWinHandler::getFullBoardImmediateWin()
{
    if (!WeichiConfigure::use_immediate_win) { return WeichiBitBoard(); }

    m_timer.start();

    WeichiBitBoard bmImmediateWin;
    m_lifedeathHandler->findFullBoardLifeDeath();

    Vector<WeichiDragon, MAX_NUM_BLOCKS>& vDragons = m_lifedeathHandler->getDragons();
    for (int iDragon = 0; iDragon < vDragons.size(); ++iDragon) {
        WeichiDragon& dragon = vDragons[iDragon];
        Color ownColor = dragon.getColor();
        Color oppColor = AgainstColor(ownColor);
        WeichiBitBoard bmSize1CA;
        WeichiBitBoard bmConnectBlock;
        WeichiBitBoard bmSplit;
        WeichiBitBoard bmKill;
        WeichiBitBoard bmAtari;
        for (int iCA = 0; iCA < dragon.getNumClosedAreas(); ++iCA) {
            const WeichiClosedArea* ca = dragon.getClosedArea(iCA);
            if (ca->getNumStone() == 1) {
                bmSize1CA |= ca->getStoneMap();
            } else if (ca->getNumStone() >= 3) {
                bmSplit |= getSplitPoints(ca);
            }
            bmKill |= getKillPoints(ca);
        }

        bmAtari |= getAtariPoints(oppColor);

        bmConnectBlock |= getConnectPoints(dragon);
        bmImmediateWin |= bmConnectBlock;
        bmImmediateWin -= bmSize1CA;
        bmImmediateWin |= bmSplit;
        bmImmediateWin |= bmKill;
        bmImmediateWin |= bmAtari;
    }

    m_timer.stopAndAddAccumulatedTime();

    return bmImmediateWin;
}

WeichiBitBoard WeichiQuickWinHandler::getFullBoardImmediateLoss()
{
    if (!WeichiConfigure::use_immediate_win) { return WeichiBitBoard(); }

    Color liveColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
    Color oppColor = AgainstColor(liveColor);

    WeichiBitBoard bmCrucial = WeichiGlobalInfo::getEndGameCondition().getCrucialStonesBitBoard();
    if (bmCrucial.empty()) { return WeichiBitBoard(); }

    WeichiBitBoard bmImmediateLoss;
    bool bHas2libBlock = false;
    WeichiBitBoard bmLastLiberty;
    int pos = 0;
    while ((pos = bmCrucial.bitScanForward()) != -1) {
        const WeichiGrid& grid = m_board.getGrid(pos);
        const WeichiBlock* block = grid.getBlock();
        if (!block) { continue; }

        if (block->getLiberty() >= 2) {
            bHas2libBlock = true;
            break;
        }
        // 1-liberty
        bmLastLiberty.SetBitOn(block->getLastLibertyPos());
    }

    if (!bHas2libBlock && bmLastLiberty.bitCount() == 1) {
        bmImmediateLoss = bmLastLiberty;
    }

    return bmImmediateLoss;
}

bool WeichiQuickWinHandler::hasConnectorPotentialRZone(WeichiBitBoard& resultRZone)
{
    Vector<WeichiDragon, MAX_NUM_DRAGONS>& vSnakes = m_lifedeathHandler->getSnakes();
    for (int iSnake = 0; iSnake < vSnakes.size(); ++iSnake) {
        WeichiDragon& snake = vSnakes[iSnake];
        // CERR() << "iSnake: " << iSnake << endl ;
        // CERR() << "SnakeSize: " << vSnakes.size() << endl ;
        if (hasPotentialRZone(snake, resultRZone)) { return true; }
    }

    return false;
}

WeichiBitBoard WeichiQuickWinHandler::getConnectPoints(WeichiDragon& dragon) const
{
    Color ownColor = dragon.getColor();
    WeichiBitBoard bmConn;
    const Vector<WeichiBlock*, MAX_NUM_BLOCKS>& vBlocks = dragon.getBlocks();
    int countLibs[MAX_NUM_GRIDS] = {0};
    for (int iBlock = 0; iBlock < vBlocks.size(); ++iBlock) {
        WeichiBlock* block = vBlocks[iBlock];
        WeichiBitBoard bmLib = block->getLibertyBitBoard(m_board.getBitBoard());
        int libPos = 0;
        while ((libPos = bmLib.bitScanForward()) != -1) {
            countLibs[libPos]++;
        }
    }

    // Compute co-lib, except split points
    for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
        const WeichiGrid& grid = m_board.getGrid(*it);
        if (countLibs[*it] == 0) { continue; }
        if (countLibs[*it] == 1 && !grid.getPattern().getIsSplitCA(ownColor, ownColor)) { continue; }

        bmConn.SetBitOn(*it);
    }

    /*	for( int i=0 ; i<MAX_NUM_GRIDS ; i++) {
                    const WeichiGrid& grid = m_board.getGrid(i) ;
                    if( countLibs[i] == 0 ) { continue ; }
                    if( countLibs[i] == 1 && !grid.getPattern().getIsSplitCA(ownColor, ownColor) ) { continue ; }

                    bmConn.SetBitOn(i);
            }*/

    return bmConn;
}

WeichiBitBoard WeichiQuickWinHandler::getSplitPoints(const WeichiClosedArea* closedarea) const
{
    WeichiBitBoard bmSplit;
    WeichiBitBoard bmStone = closedarea->getStoneMap();
    Color ownColor = closedarea->getColor();
    int pos = 0;
    while ((pos = bmStone.bitScanForward()) != -1) {
        const WeichiGrid& grid = m_board.getGrid(pos);
        if (grid.getColor() != COLOR_NONE) { continue; }

        if (grid.getPattern().getIsSplitCA(ownColor, ownColor)) {
            bmSplit.SetBitOn(pos);
        }
    }

    return bmSplit;
}

WeichiBitBoard WeichiQuickWinHandler::getKillPoints(const WeichiClosedArea* closedarea) const
{
    WeichiBitBoard bmKill;
    WeichiBitBoard bmStone = closedarea->getStoneMap();
    Color ownColor = closedarea->getColor();
    int pos = 0;
    while ((pos = bmStone.bitScanForward()) != -1) {
        const WeichiGrid& grid = m_board.getGrid(pos);
        if (grid.getBlock() == NULL) { continue; }

        const WeichiBlock* oppBlock = grid.getBlock();
        if (oppBlock->getLiberty() == 1) {
            bmKill |= oppBlock->getLibertyBitBoard(m_board.getBitBoard());
        }
        bmStone -= oppBlock->getStoneMap();
    }

    return bmKill;
}

WeichiBitBoard WeichiQuickWinHandler::getAtariPoints(Color oppColor) const
{
    WeichiBitBoard bmAtari;
    Color ownColor = AgainstColor(oppColor);
    WeichiBitBoard opp2lib = m_board.getTwoLibBlocksBitBoard(oppColor);
    int pos = 0;
    while ((pos = opp2lib.bitScanForward()) != -1) {
        const WeichiGrid& grid = m_board.getGrid(pos);
        Vector<uint, 2> v2libs;
        grid.getBlock()->getLibertyPositions(m_board.getBitBoard(), v2libs);
        for (int i = 0; i < v2libs.size(); i++) {
            WeichiMove atariMove(ownColor, v2libs[i]);
            if (m_board.getLibertyAfterPlay(atariMove) >= 2) {
                bmAtari.SetBitOn(v2libs[i]);
            }
        }

        opp2lib -= grid.getBlock()->getStoneMap();
    }

    return bmAtari;
}

bool WeichiQuickWinHandler::hasPotentialRZone(const WeichiDragon& dragon, WeichiBitBoard& bmResult)
{
    Color winColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
    WeichiBitBoard bmPlayed;

    // CERR() << "[Zero remainging stone]" << endl ;
    // CERR() <<  dragon.getConnectorMap().toString() << endl ;
    // m_board.showColorBoard() ;

    bool bAlive = hasBensonSequence(dragon, bmPlayed, bmResult);
    if (!bAlive) { return false; }

    // �b���bensonSquence���e���U�A�ˬd�i�_�����@�Ǥl�A�ϱobmResult�ܤp
    // �]�O�@�������@��
    WeichiBitBoard bmNewResult;
    WeichiBitBoard bmBensonSequence = bmPlayed;
    WeichiBitBoard bmPossibleReduced = bmBensonSequence;
    bool bIsReduced = false;
    while (!bmPossibleReduced.empty() && !bIsReduced) {
        WeichiBitBoard bmTestStones = bmBensonSequence;
        int reducedPos = bmPossibleReduced.bitScanForward();
        bmTestStones.SetBitOff(reducedPos);

        bmPlayed.Reset();
        m_board.storeMoveBackup();
        bool bAlive = false;
        int pos = 0;
        while ((pos = bmTestStones.bitScanForward()) != -1) {
            WeichiMove move(winColor, pos);
            m_board.play(move);
            bmPlayed.SetBitOn(pos);
            if (m_board.getGrid(pos).getBlock()->getStatus() == LAD_LIFE) {
                bmNewResult = WeichiRZoneHandler::calculateBensonRZone(m_board, move);
                if (bmNewResult.bitCount() < bmResult.bitCount()) {
                    bmResult = bmNewResult;
                    bIsReduced = true;
                }
                break;
            }
        }

        // CERR() << "[Third test reduced]" << endl ;
        // CERR() << "Reduced pos: " << reducedPos << endl ;
        // CERR() << "Alive:" << bAlive << endl ;
        // CERR() << "bmPlayed:" << endl << bmPlayed.toString() << endl ;
        // CERR() << "bmNewResult:" << endl << bmNewResult.toString() << endl ;
        // CERR() << "Final Board:" << endl ;
        // m_board.showColorBoard() ;

        m_board.resumeMoveBackup();
    }

    return bAlive;
}

bool WeichiQuickWinHandler::hasBensonSequence(const WeichiDragon& dragon, WeichiBitBoard& bmRetPlayed, WeichiBitBoard& bmResult)
{
    Color winColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
    WeichiBitBoard bmPossibleConnector = dragon.getConnectorMap();

    // First test all stones
    bmRetPlayed.Reset();
    m_board.storeMoveBackup();
    bool bAlive = false;
    int pos = 0;
    while ((pos = bmPossibleConnector.bitScanForward()) != -1) {
        WeichiMove move(winColor, pos);
        m_board.play(move);
        bmRetPlayed.SetBitOn(pos);
        if (m_board.getGrid(pos).getBlock()->getStatus() == LAD_LIFE) {
            bmResult = WeichiRZoneHandler::calculateBensonRZone(m_board, move);
            bAlive = true;
            break;
        }
    }

    // CERR() << "[First test benson]" << endl ;
    // CERR() << "Alive:" << bAlive << endl ;
    // CERR() << "bmPlayed:" << endl << bmPlayed.toString() << endl ;
    // CERR() << "bmResult:" << endl << bmResult.toString() << endl ;
    // CERR() << "Final Board:" << endl ;
    // m_board.showColorBoard() ;

    m_board.resumeMoveBackup();

    WeichiBitBoard bmAllPossibleEye = getPossibleEyeBitmap(dragon);
    WeichiBitBoard bmPossibleEye = bmAllPossibleEye;
    if (!bAlive) {
        while (!bmPossibleEye.empty() && !bAlive) {
            bmPossibleConnector = dragon.getConnectorMap();
            int possibleEyePos = bmPossibleEye.bitScanForward();
            bmPossibleConnector.SetBitOff(possibleEyePos);

            bmRetPlayed.Reset();
            m_board.storeMoveBackup();
            bAlive = false;
            int pos = 0;
            while ((pos = bmPossibleConnector.bitScanForward()) != -1) {
                WeichiMove move(winColor, pos);
                m_board.play(move);
                bmRetPlayed.SetBitOn(pos);
                if (m_board.getGrid(pos).getBlock()->getStatus() == LAD_LIFE) {
                    bmResult = WeichiRZoneHandler::calculateBensonRZone(m_board, move);
                    bAlive = true;
                    break;
                }
            }

            // CERR() << "[Second test removing Possible Eye Pos]" << endl ;
            // CERR() << "Remove Possible Eye Pos: " << possibleEyePos << endl ;
            // CERR() << "Alive:" << bAlive << endl ;
            // CERR() << "bmPlayed:" << endl << bmPlayed.toString() << endl ;
            // CERR() << "bmResult:" << endl << bmResult.toString() << endl ;
            // CERR() << "Final Board:" << endl ;
            // m_board.showColorBoard() ;

            m_board.resumeMoveBackup();
        }

        // ���M�S��alive�A�Nreturn false
        // if( !bAlive ) { return false ; }
    }

    // �@��������Ӳ���
    uint iEyeArr[MAX_NUM_GRIDS];
    bmPossibleEye = bmAllPossibleEye;
    int length = bmPossibleEye.bitScanAll(iEyeArr);

    if (!bAlive && length >= 2) {

        for (int i = 0; i < length; ++i) {
            for (int j = i + 1; j < length; ++j) {
                if (bAlive) { break; }

                bmPossibleConnector = dragon.getConnectorMap();
                uint pos1 = iEyeArr[i];
                uint pos2 = iEyeArr[j];
                bmPossibleConnector.SetBitOff(pos1);
                bmPossibleConnector.SetBitOff(pos2);

                bmRetPlayed.Reset();
                m_board.storeMoveBackup();
                bAlive = false;
                int pos = 0;
                while ((pos = bmPossibleConnector.bitScanForward()) != -1) {
                    WeichiMove move(winColor, pos);
                    m_board.play(move);
                    bmRetPlayed.SetBitOn(pos);
                    if (m_board.getGrid(pos).getBlock()->getStatus() == LAD_LIFE) {
                        bmResult = WeichiRZoneHandler::calculateBensonRZone(m_board, move);
                        bAlive = true;
                        break;
                    }
                }

                m_board.resumeMoveBackup();
            }
            if (bAlive) { break; }
        }
        if (!bAlive) { return false; }
    }

    return bAlive;
}

WeichiBitBoard WeichiQuickWinHandler::getPossibleEyeBitmap(const WeichiDragon& dragon) const
{
    WeichiBitBoard bmResult;

    WeichiConnectorHandler& connHandler = m_lifedeathHandler->getConnectorHandler();
    Vector<WeichiBlock*, MAX_NUM_BLOCKS> vBlocks = dragon.getBlocks();
    for (int iBlock = 0; iBlock < vBlocks.size(); ++iBlock) {
        const WeichiBlock* block = vBlocks[iBlock];
        vector<WeichiConnector*>& vConns = connHandler.getBlockConnectors(block);
        for (int iConn = 0; iConn < vConns.size(); ++iConn) {
            WeichiConnector* connector = vConns[iConn];
            if (connector->getType() == TYPE_TIGERMOUTH) {
                bmResult |= connector->getConnectorBitmap();
            } else if (connector->getType() == TYPE_FALSE_EYE) {
                bmResult |= connector->getConnectorBitmap();
            }
        }
    }

    return bmResult;
}
