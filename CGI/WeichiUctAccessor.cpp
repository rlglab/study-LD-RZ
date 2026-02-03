#include "WeichiUctAccessor.h"
#include "BoardTranspositionTable.h"
#include "RZoneTranspositionTable.h"
#include "WeichiGlobalInfo.h"
#include "WeichiJLMctsHandler.h"
#include "WeichiLogger.h"
#include "WeichiMoveDecisionMaker.h"
#include "arguments.h"
#include <boost/asio.hpp>
#include <iostream>

using namespace std;

WeichiUctAccessor::WeichiUctAccessor(WeichiThreadState& state, node_manage::NodeAllocator<WeichiUctNode>& na)
    : BaseUctAccessor<WeichiMove, WeichiPlayoutResult, WeichiUctNode, WeichiThreadState>(state, na), m_uctCNNHandler(state, na)
{
    m_timer.reset();
}

void WeichiUctAccessor::updateAll(const WeichiRemoteCNNJob& currentJob)
{
    m_uctCNNHandler.updateAll(currentJob);
}

WeichiUctAccessor::UctNodePtr WeichiUctAccessor::selectNode()
{
    m_timer.start();
    UctNodePtr pNode = m_pRoot;
    if (WeichiConfigure::do_mcts_sequential_search) {
        if (WeichiGlobalInfo::getDoCNNForward() != 0) { return pNode; }
        if (!isQueueHaveSpace()) {
            WeichiGlobalInfo::getDoCNNForward() = 1;
            return pNode;
        }
    } else {
        if (!isQueueHaveSpace()) { return pNode; }
    }

    Vector<WeichiMove, MAX_TREE_DEPTH> vPaths;
    m_state.m_path.push_back(pNode);
    WeichiBoard& board = m_state.m_board;
    float parentCNNVNValue = -1.0f;

    // selection
    int MMDepth = 0;
    while (pNode->hasChildren()) {
        parentCNNVNValue = pNode->getCNNVNValue();
        UctNodePtr pTerminal = getTerminalNode(pNode);
        if (pTerminal == UctNodePtr::NULL_PTR) {
            pNode = selectChild(pNode);
        } else {
            pNode = pTerminal;
        }

        addVirtualLoss(pNode);
        vPaths.push_back(pNode->getMove());
        playAndStoreStatus(pNode);

        // Refactor: Remove the condition check of TerminalWin, it means does not lookup when it is benson
        if (!m_state.isTerminalWin() && findEntryInTT(pNode)) {
            ++WeichiGlobalInfo::getTreeInfo().m_nTTmatch;
            if (pNode->getUctNodeStatus() == UCT_STATUS_WIN) {
                updateStatus(SOLUTION_PROOF, pNode, false);
            } else if (pNode->getUctNodeStatus() == UCT_STATUS_LOSS) {
                updateStatus(SOLUTION_DISPROOF, pNode, false);
            }
            m_timer.stopAndAddAccumulatedTime();
            return pNode;
        }

        if (!pNode->isCNNNode()) { MMDepth++; }
    }

    if (WeichiConfigure::mcts_use_solver) {
        if (m_state.isTerminalWin()) {
            ++WeichiGlobalInfo::getTreeInfo().m_nLeafBenson;
            pNode->setUctNodeStatus(UCT_STATUS_WIN);
            pNode->setSolDepth(board.getMoveList().size());
            updateStatus(SOLUTION_PROOF, pNode);
            m_timer.stopAndAddAccumulatedTime();
            return pNode;
        } else if (m_state.isTerminalLoss()) {
            pNode->setUctNodeStatus(UCT_STATUS_LOSS);
            pNode->setSolDepth(board.getMoveList().size());
            updateStatus(SOLUTION_DISPROOF, pNode);
            m_timer.stopAndAddAccumulatedTime();
            return pNode;
        }
    }

    if (isPotentialWin(pNode, parentCNNVNValue)) {
        WeichiBitBoard bmPotentailRZone;
        if (m_state.m_lifedeathHandler.getQuickWinHandler().hasConnectorPotentialRZone(bmPotentailRZone)) {
            int id = WeichiGlobalInfo::getRZoneDataTable().createRZoneData(bmPotentailRZone, board);
            pNode->setPotentialRZoneID(id);
        }
    }

    // expansion
    if (!m_state.isTerminal()) {
        bool bSuccess = false;
        if (pNode->getExpandToken()) {
            const int MAX_MM_DEPTH = 1;
            bSuccess = (expandChildren(pNode) || (WeichiConfigure::use_playout && MMDepth <= MAX_MM_DEPTH));
            if (bSuccess) {
                bSuccess = addToCNNJobQueue(pNode, CNNJob(vPaths));

                if (WeichiConfigure::use_playout) {
                    // select to next child
                    pNode = selectChild(pNode);
                    addVirtualLoss(pNode);
                    vPaths.push_back(pNode->getMove());
                    playAndStoreStatus(pNode);
                }
            }
            pNode->releaseExpandToken();
        }

        if (!bSuccess) {
            for (uint i = 1; i < m_state.m_path.size(); ++i) { removeVirutalLoss(m_state.m_path[i]); }
            if (WeichiConfigure::do_mcts_sequential_search) { WeichiGlobalInfo::getDoCNNForward() = 1; }
        }
    }

    m_timer.stopAndAddAccumulatedTime();
    return pNode;
}

WeichiUctAccessor::UctNodePtr WeichiUctAccessor::selectChild(UctNodePtr pNode)
{
    assert(pNode->hasChildren());

    double dBestScore = -DBL_MAX;
    UctNodePtr pBest = UctNodePtr::NULL_PTR;
    // UctNodePtr pPass = UctNodePtr::NULL_PTR;
    int iTotalCount = calChildSimulationCount(pNode);
    double dLogTotalCount = WeichiConfigure::mcts_use_ucb ? log(iTotalCount) : 0.0f;

    Color liveColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
    Color killColor = AgainstColor(liveColor);
    Color turnColor = m_state.m_board.getToPlay();
    WeichiBitBoard bmCrucialMoves;
    if (turnColor == liveColor) {
        bmCrucialMoves = getOwnCrucialMoves(pNode);
    } else if (turnColor == killColor) {
        bmCrucialMoves = getOppCrucialMoves(pNode);
    }

    uint childIndex = 0;
    uint bestChildIndex = 0;
    float fSumOfChildWins = 0.0f;
    float fSumOfChildSims = 0.0f;
    for (UctChildIterator<WeichiUctNode> it(pNode); it; ++it) {
        WeichiUctNode& child = *it;
        StatisticData UctData = child.getUctData();

        if (WeichiConfigure::mcts_use_solver && child.isProved()) { continue; }
        if (WeichiConfigure::mcts_use_solver && child.getPosition() != WeichiMove::PASS_POSITION && !bmCrucialMoves.BitIsOn(child.getPosition())) { continue; }

        if (!isInWideningRange(iTotalCount, childIndex)) { break; }

        // calculate move score & add bias
        // double dMoveScore = getMoveScore(child);
        double dAdjustChildVNValue = pNode->getAdjustChildVNValue();
        double dMoveScore = getMoveScore(child, dAdjustChildVNValue);
        dMoveScore += getUcbBias(dLogTotalCount, UctData);
        dMoveScore += getProgressiveBias(child);
        dMoveScore += getPUctBias(iTotalCount, child);

        if (child.hasChildren()) {
            fSumOfChildWins += child.getUctData().getMean();
            fSumOfChildSims += 1;
        }

        if (dMoveScore > dBestScore) {
            dBestScore = dMoveScore;
            if (bestChildIndex < childIndex + 1) { bestChildIndex = childIndex; }
            pBest = it;
        }

        childIndex++;
    }

    if (WeichiGlobalInfo::getTreeInfo().m_nMaxChildIndex < bestChildIndex) {
        WeichiGlobalInfo::getTreeInfo().m_nMaxChildIndex = bestChildIndex;
    }
    WeichiGlobalInfo::getTreeInfo().m_childIndex[bestChildIndex]++;

    float fAdjustChildVNValue = fSumOfChildWins / (fSumOfChildSims + 1);
    pNode->setAdjustChildVNValue(fAdjustChildVNValue);

    assertToFile(pBest.isValid(), const_cast<WeichiBoard*>(&m_state.m_board));

    return pBest;
}

bool WeichiUctAccessor::expandChildren(UctNodePtr pNode)
{
    if (WeichiConfigure::use_playout && pNode->getUctData().getCount() < Configure::ExpandThreshold) { return false; }

    m_state.startExpansion();

    if (WeichiConfigure::use_playout) {
        // for MM
        Color turnColor = m_state.m_board.getToPlay();
        m_state.m_moveFeatureHandler.calculateFullBoardCandidates(turnColor);
        const Vector<CandidateEntry, MAX_NUM_GRIDS>& vUctCandidateList = m_state.m_moveFeatureHandler.getUCTCandidateList(true);

        ignoreMoves(pNode, vUctCandidateList);

        uint size = (vUctCandidateList.size() > MAX_EXPAND_CHILD_NODE) ? MAX_EXPAND_CHILD_NODE : vUctCandidateList.size();
        assert(size != 0);

        // allocation child node && check allocation is fail
        UctNodePtr firstChild = m_nodeAllocator.allocateNodes(size);
        if (firstChild.isNull()) { return false; }

        UctChildIterator<WeichiUctNode> childIterator(firstChild, size);
        for (uint i = 0; i < size; i++) {
            WeichiMove move(turnColor, vUctCandidateList[i].getPosition());
            assertToFile(childIterator, const_cast<WeichiBoard*>(&m_state.m_board));
            assertToFile(!m_state.m_board.isIllegalMove(move, m_state.m_ht), const_cast<WeichiBoard*>(&m_state.m_board));

            childIterator->reset(move);
            childIterator->setProb(vUctCandidateList[i].getScore());

            ++childIterator;
        }

        pNode->getUpdateToken();
        if (!pNode->hasChildren()) { pNode->setChildren(firstChild, size); }
        pNode->releaseUpdateToken();
    }

    m_state.endExpansion();

    return true;
}

void WeichiUctAccessor::update(WeichiPlayoutResult result)
{
    Color winner = result.getWinner();

    WeichiGlobalInfo::getGlobalLock().lock();
    if (WeichiConfigure::use_playout || (!WeichiConfigure::use_playout && m_state.isTerminal())) {
        float fBonus = ((winner == COLOR_NONE) ? 0.0f : ((winner != m_state.getRootTurn()) ? 1.0f : -1.0f));
        for (uint i = 0; i < m_state.m_path.size(); ++i) {
            UctNodePtr pNode = m_state.m_path[i];

            // Update
            if (i > 0) { removeVirutalLoss(pNode); }
            pNode->getUctData().add(fBonus, 1.0f);
            fBonus = -fBonus;
        }

        if (!WeichiConfigure::use_playout) {
            WeichiGlobalInfo::getTreeInfo().m_nDCNNJob++;
            WeichiGlobalInfo::getTreeInfo().m_nTerminal++;
        }

        if (m_pRoot->hasChildren()) {
            UctNodePtr maxCountChild = UctNodePtr::NULL_PTR;
            double maxCount = -1;
            for (UctChildIterator<WeichiUctNode> it(m_pRoot); it; ++it) {
                if (it->getUctDataWithoutVirtualLoss().getCount() > maxCount) {
                    maxCountChild = it;
                    maxCount = it->getUctDataWithoutVirtualLoss().getCount();
                }
            }
            WeichiGlobalInfo::getRootSimInfo().push_back(tuple<int, double, short>(m_pRoot->getUctDataWithoutVirtualLoss().getCount(), maxCountChild->getUctDataWithoutVirtualLoss().getMean(), maxCountChild->getPosition()));
        }
    }
    WeichiGlobalInfo::getGlobalLock().unlock();

    if (!WeichiConfigure::use_playout) { return; }

    // update value network's value
    bool bFindValue = false;
    float fValue = 0.0f;
    for (int i = m_state.m_path.size() - 1; i >= 0; i--) {
        UctNodePtr pNode = m_state.m_path[i];

        if (!bFindValue) {
            if (!pNode->isCNNNode()) {
                continue;
            } else {
                bFindValue = true;
                fValue = pNode->getCNNVNValue();
            }
        }

        pNode->getVNValueData().add(fValue);
        fValue = 1 - fValue;
    }

    WeichiGlobalInfo::getTreeInfo().m_nHandMove += m_state.m_moves.size();
    WeichiGlobalInfo::getSearchInfo().m_territory.addTerritory(m_state.m_board.getTerritory(), winner);
    if (m_state.m_path.size() > 1 && m_state.m_path[1]->getMove().isPass()) { WeichiGlobalInfo::getSearchInfo().m_passTerritory.addTerritory(m_state.m_board.getTerritory(), winner); }

    if (WeichiConfigure::DoPlayoutCacheLog) { WeichiGlobalInfo::getLogInfo().m_cacheLogger.writeCacheLogFile(m_state.m_board, m_state.m_board.getTerritory()); }
}

void WeichiUctAccessor::collectMovePlayed()
{
    Vector<WeichiMove, MAX_GAME_LENGTH>& moves = m_state.m_moves;
    m_blacks.setAllAs(-1, MAX_NUM_GRIDS);
    m_whites.setAllAs(-1, MAX_NUM_GRIDS);

    for (uint i = 0; i < moves.size(); ++i) {
        const WeichiMove& m = moves[i];

        Vector<int, MAX_NUM_GRIDS>& mylist = (m.getColor() == COLOR_BLACK ? m_blacks : m_whites);
        Vector<int, MAX_NUM_GRIDS>& opplist = (m.getColor() == COLOR_BLACK ? m_whites : m_blacks);
        uint pos = m.getPosition();
        if (mylist[pos] == -1 && opplist[pos] == -1)
            mylist[pos] = i;
    }
}

void WeichiUctAccessor::ignoreMoves(UctNodePtr node, const Vector<CandidateEntry, MAX_NUM_GRIDS>& vUctCandidateList)
{
    if (arguments::mode != "jlmcts") { return; }

    if (node == m_pRoot) {
        string sIgnores = WeichiConfigure::Ignore;
        string normalizedIgnored = WeichiJLMctsHandler::normalizePathStringFromJLMCTS(sIgnores);
        for (uint pos = 0; normalizedIgnored.find_first_of("BW", pos) != string::npos; pos += 5) {
            Color c = toColor(normalizedIgnored[pos]);
            WeichiMove m(c, normalizedIgnored.substr(pos + 2, 2));
            for (uint i = 0; i < vUctCandidateList.size(); i++) {
                if (m.getPosition() == vUctCandidateList[i].getPosition()) {
                    const_cast<Vector<CandidateEntry, MAX_NUM_GRIDS>&>(vUctCandidateList).erase_no_order(i);
                }
            }
        }

        // judge it again
        if (vUctCandidateList.size() == 1)
            WeichiGlobalInfo::get()->m_bIsLastCandidate = true;

        if (vUctCandidateList.size() == 0) {
            const_cast<Vector<CandidateEntry, MAX_NUM_GRIDS>&>(vUctCandidateList).push_back(CandidateEntry(PASS_MOVE.getPosition(), 0.00001));
            WeichiGlobalInfo::get()->m_bIsLastCandidate = true;
        }
    }

    return;
}

void WeichiUctAccessor::updateStatus(int type_sol, UctNodePtr node, bool bProveBySearch)
{
    if (node->getUctNodeStatus() == UCT_STATUS_UNKNOWN) { return; }

    Color solColor = (type_sol == SOLUTION_PROOF) ? WeichiGlobalInfo::getTreeInfo().m_winColor : AgainstColor(WeichiGlobalInfo::getTreeInfo().m_winColor);
    Color oppColor = AgainstColor(solColor);
    WeichiUctNodeStatus solutionStatus = (type_sol == SOLUTION_PROOF) ? UCT_STATUS_WIN : UCT_STATUS_LOSS;

    if (WeichiConfigure::use_rzone && bProveBySearch) {
        WeichiBitBoard bmRZone = WeichiGlobalInfo::getEndGameCondition().getEndGameRZone(m_state, solColor);
        updateNodeRZone(type_sol, node, bmRZone);
    }

    Vector<UctNodePtr, MAX_TREE_DEPTH> vNodePath = m_state.m_path;
    int index = vNodePath.size() - 1;

    bool bStoreFirst = true;
    bool bIsUpdateEnd = false;
    while (index > 0) {
        UctNodePtr currentNode = vNodePath[index];
        removeVirutalLoss(currentNode); // do not remove virtual loss in root

        if (bIsUpdateEnd) {
            undoAndResumeStatus();
            index--;
            continue;
        }

        if (currentNode->getColor() == oppColor) {
            if (!bProveBySearch && bStoreFirst) {
                bStoreFirst = false;
            } else {
                storeTT(currentNode);
            }

            if (index - 1 >= 0) { // Has own parent
                UctNodePtr parentOwn = vNodePath[index - 1];
                currentNode->setUctNodeStatus(solutionStatus);
                if (WeichiConfigure::use_rzone) {
                    int lossPos = currentNode->getPosition();
                    bool bLikePassSelfAtari = isLikePassSelfAtari(lossPos, index, vNodePath);
                    int rzoneID = (type_sol == SOLUTION_PROOF) ? currentNode->getProofRZoneID() : currentNode->getDisproofRZoneID();
                    WeichiBitBoard bmChildRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(rzoneID);
                    proveChildrenOutsideRZone(parentOwn, rzoneID, type_sol, currentNode->getPosition(), bLikePassSelfAtari);
                }
                if (isAllChildrenProved(parentOwn)) {
                    if (WeichiConfigure::use_rzone) {
                        WeichiBitBoard bmUnion;
                        for (UctChildIterator<WeichiUctNode> it(parentOwn); it; ++it) {
                            int childRZoneID = (type_sol == SOLUTION_PROOF) ? it->getProofRZoneID() : it->getDisproofRZoneID();
                            WeichiBitBoard bmRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(childRZoneID);
                            bmUnion |= bmRZone;
                        }
                        undoAndResumeStatus();
                        WeichiBitBoard bmUpdatedRZone = bmUnion;
                        bmUpdatedRZone = m_state.m_board.getAndPlayerRZone(bmUpdatedRZone, parentOwn->getColor());
                        updateNodeRZone(type_sol, parentOwn, bmUpdatedRZone);
                    }
                    parentOwn->setUctNodeStatus(solutionStatus);
                    // update solution depth
                    int maxDepth = -1;
                    for (UctChildIterator<WeichiUctNode> it(parentOwn); it; ++it) {
                        if (it->getSolDepth() > maxDepth) { maxDepth = it->getSolDepth(); }
                    }
                    parentOwn->setSolDepth(maxDepth);
                } else {
                    bIsUpdateEnd = true;
                }
            }
        } else if (currentNode->getColor() == solColor) {
            checkSameAncestorRZpattern(currentNode);
            if (!bProveBySearch && bStoreFirst) {
                bStoreFirst = false;
            } else {
                // win by search
                short koPos = m_state.m_board.getKo();
                if (koPos != -1) { currentNode->setWinByKoPosition(koPos); }
                storeTT(currentNode);
            }
            if (index - 1 >= 0) {
                UctNodePtr parentOpponent = vNodePath[index - 1];
                parentOpponent->setUctNodeStatus(solutionStatus);
                parentOpponent->setSolDepth(currentNode->getSolDepth());
                if (WeichiConfigure::use_rzone) { undoAndUpdateRZoneAboveFromSolSide(type_sol, currentNode, parentOpponent); }
            }
        }
        index--;
    }

    return;
}

void WeichiUctAccessor::playAndStoreStatus(UctNodePtr pNode)
{
    m_state.backupOneMove(pNode);
    // push_back the following
    // 1. m_moves
    // 2. m_keys
    // 3. m_ht

    m_state.play(pNode->getMove());

    return;
}

void WeichiUctAccessor::undoAndResumeStatus()
{
    m_state.rollbackOneMove();

    return;
}

bool WeichiUctAccessor::findEntryInTT(UctNodePtr pNode)
{
    if (!WeichiConfigure::mcts_use_solver) { return false; }

    WeichiBoard& board = m_state.m_board;
    if (WeichiConfigure::use_board_transposition_table) {
        BoardTranspositionTable& boardTT = WeichiGlobalInfo::getBoardTT();
        uint index = boardTT.lookup(m_state.m_board.getTThashkey());
        if (index == -1) {
            return false;
        } else {
            TTentry foundEntry = boardTT.getEntry(index);
            setNodeAttributeByTT(pNode, foundEntry);
            return true;
        }
    } else if (WeichiConfigure::use_rzone_transposition_table) {
        RZoneHierarchicalTT& tree = WeichiGlobalInfo::getHierTT();
        TTentry foundEntry;
        if (tree.lookup(m_state, foundEntry, pNode)) {
            if (WeichiConfigure::check_zone_tt_ghi && hasZoneTTGHIProblem(foundEntry)) { return false; }
            setNodeAttributeByTT(pNode, foundEntry);
            return true;
        }
    } else if (WeichiConfigure::use_rzone_linear_transposition_table) {
        RZoneLinearTT& table = WeichiGlobalInfo::getLinearTT();
        TTentry foundEntry;
        if (table.lookup(m_state, foundEntry, pNode)) {
            setNodeAttributeByTT(pNode, foundEntry);
            return true;
        }
    }

    return false;
}

bool WeichiUctAccessor::hasPotentialGHIproblem(int type_sol, UctNodePtr pSolutionRoot) const
{
    if (pSolutionRoot == UctNodePtr::NULL_PTR) { return false; }

    const vector<UctNodePtr>& vLoop = pSolutionRoot->getLoopCheck();
    for (int i = 0; i < vLoop.size(); ++i) {
        if (hasPotentialLoop(type_sol, vLoop[i])) { return true; }
    }

    return false;
}

bool WeichiUctAccessor::hasPotentialLoop(int type_sol, UctNodePtr pCheckNode) const
{
    if (pCheckNode == UctNodePtr::NULL_PTR) { return false; }

    int rzoneID = (type_sol == SOLUTION_PROOF) ? pCheckNode->getProofRZoneID() : pCheckNode->getDisproofRZoneID();
    WeichiBitBoard bmRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(rzoneID);
    Dual<WeichiBitBoard> bmRZoneStone = WeichiGlobalInfo::getRZoneDataTable().getRZoneStone(rzoneID);
    Color color = pCheckNode->getColor();

    for (int i = 0; i < m_state.m_path.size() - 1; ++i) {
        int historyIndex = m_state.m_board.getBitBoardList().size() - m_state.m_path.size() + i;
        const Dual<WeichiBitBoard> bmHistoryBitBoard = m_state.m_board.getBitBoardList()[historyIndex];

        if (m_state.m_path[i]->getColor() == color &&
            (bmHistoryBitBoard.m_black & bmRZone) == bmRZoneStone.m_black &&
            (bmHistoryBitBoard.m_white & bmRZone) == bmRZoneStone.m_white) {
            return true;
        }
    }

    const vector<UctNodePtr>& vLoop = pCheckNode->getLoopCheck();
    for (int i = 0; i < vLoop.size(); ++i) {
        if (hasPotentialLoop(type_sol, vLoop[i])) { return true; }
    }

    return false;
}

void WeichiUctAccessor::setNodeAttributeByTT(UctNodePtr pNode, TTentry& ttEntry)
{
    WeichiBitBoard ttRZone = ttEntry.m_bmRZone;
    if (ttEntry.m_status == UCT_STATUS_WIN) {
        updateNodeRZone(SOLUTION_PROOF, pNode, ttRZone);
    } else if (ttEntry.m_status == UCT_STATUS_LOSS) {
        updateNodeRZone(SOLUTION_DISPROOF, pNode, ttRZone);
    }
    pNode->setUctNodeStatus(ttEntry.m_status);
    pNode->setTTmatchedNodeID(ttEntry.m_nodeID);
    pNode->setWinByKoPosition(ttEntry.m_koPosition);
    pNode->setSolDepth(m_state.m_board.getMoveList().size() + ttEntry.m_solDiffDepth);

    return;
}

void WeichiUctAccessor::storeTT(UctNodePtr pNode)
{
    if (!WeichiConfigure::mcts_use_solver) { return; }

    // Not to store ZTT when board is benson (Refactor: to move it out)
    WeichiBoard& board = m_state.m_board;
    if (WeichiGlobalInfo::getEndGameCondition().isEndGame(m_state, WeichiGlobalInfo::getTreeInfo().m_winColor)) { return; }

    if (WeichiConfigure::use_board_transposition_table) {
        TTentry entry = createStoredTTEntry(pNode);
        BoardTranspositionTable& boardTT = WeichiGlobalInfo::getBoardTT();
        boardTT.store(board.getTThashkey(), entry);
    } else if (WeichiConfigure::use_rzone_transposition_table) {
        handleSSK(pNode);
        TTentry entry = createStoredTTEntry(pNode);
        RZoneHierarchicalTT& hierTT = WeichiGlobalInfo::getHierTT();
        hierTT.store(entry);
        hierTT.reConstruct();
    } else if (WeichiConfigure::use_rzone_linear_transposition_table) {
        TTentry entry = createStoredTTEntry(pNode);
        RZoneLinearTT& linearTT = WeichiGlobalInfo::getLinearTT();
        linearTT.store(entry);
    }

    return;
}

TTentry WeichiUctAccessor::createStoredTTEntry(UctNodePtr pNode)
{
    WeichiBitBoard bmRZone;
    if (pNode->getUctNodeStatus() == UCT_STATUS_WIN) {
        bmRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(pNode->getProofRZoneID());
    } else if (pNode->getUctNodeStatus() == UCT_STATUS_LOSS) {
        bmRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(pNode->getDisproofRZoneID());
    }

    WeichiBoard& board = m_state.m_board;

    TTentry entry;
    entry.m_turnColor = board.getToPlay();
    entry.m_pNode = pNode;
    entry.m_status = pNode->getUctNodeStatus();
    entry.m_bmBlackPiece = board.getStoneBitBoard(COLOR_BLACK) & bmRZone;
    entry.m_bmWhitePiece = board.getStoneBitBoard(COLOR_WHITE) & bmRZone;
    entry.m_bmEmptyPiece = bmRZone - entry.m_bmBlackPiece - entry.m_bmWhitePiece;
    entry.m_bmRZone = bmRZone;
    entry.m_nodeID = pNode.getIndex();
    entry.m_koPosition = pNode->getWinByKoPosition();
    entry.m_solDiffDepth = pNode->getSolDepth() - m_state.m_board.getMoveList().size();

    return entry;
}

void WeichiUctAccessor::handleSSK(UctNodePtr pNode)
{
    Color proofColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
    Color disproofColor = AgainstColor(proofColor);

    int type_sol = (pNode->getUctNodeStatus() == UCT_STATUS_WIN) ? SOLUTION_PROOF : SOLUTION_DISPROOF;
    if (type_sol == SOLUTION_PROOF && pNode->getColor() != proofColor) { return; }
    if (type_sol == SOLUTION_DISPROOF && pNode->getColor() != disproofColor) { return; }

    WeichiBoard& board = m_state.m_board;
    WeichiBitBoard bmSSK;
    for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
        const WeichiMove move(board.m_status.m_colorToPlay, *it);
        if (!board.isTTIllegalMove(move, m_state.m_ht)) { continue; }
        ++WeichiGlobalInfo::getTreeInfo().m_nKos;
        if (move.getPosition() != board.m_status.m_ko) { ++WeichiGlobalInfo::getTreeInfo().m_nLongKos; }

        // For updating R-zone (not general, the R-zone might not be capture)
        int rzoneID = (type_sol == SOLUTION_PROOF) ? pNode->getProofRZoneID() : pNode->getDisproofRZoneID();
        WeichiBitBoard bmRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(rzoneID);
        WeichiBitBoard bmInfluence = board.getInfluenceBitBoard(move);
        if (bmRZone.hasIntersection(bmInfluence)) {
            bmRZone = board.getUpdatedRZoneFromOurTurn(move, bmRZone);
            if (board.isCaptureMove(move)) {
                WeichiBitBoard bmCaptureRZone = board.getCaptureRZone(move);
                bmRZone |= bmCaptureRZone;
            }
        }
        updateNodeRZone(type_sol, pNode, bmRZone);
        // end updating R-zone

        bmSSK.SetBitOn(*it);

        int index = m_state.m_keys.size();
        HashKey64 key = board.getHashKeyAfterPlay(move);
        for (int i = 0; i < m_state.m_keys.size(); ++i) {
            if (key != m_state.m_keys[i]) { continue; }
            index = (index < i ? index : i);
            break;
        }

        if (index < m_state.m_keys.size()) {
            // Root -> P_loop
            ++index;
            UctNodePtr pLoopHead = m_state.m_path[index];
            for (int i = 0; i < index; ++i) {
                UctNodePtr p = m_state.m_path[i];
                p->getLoopCheck().push_back(pLoopHead);
            }

            // P_loop -> P_leaf
            for (int i = index; i < m_state.m_path.size() - 1; ++i) {
                // don't need to skip loop head
                if (i != index) { m_state.m_path[i]->setSkipTTByLoop(true); }
                m_state.m_path[i]->getLoopCheck().push_back(m_state.m_path[i + 1]);
            }

            m_state.m_path.back()->setSkipTTByLoop(true);
        }
    }

    if (!bmSSK.empty()) {
        int id = WeichiGlobalInfo::getRZoneDataTable().createRZoneData(bmSSK, board);
        pNode->setSSKID(id);
    }

    return;
}

HashKey64 WeichiUctAccessor::getBoardHashkeyWithSSK()
{
    WeichiBoard& board = m_state.m_board;
    HashKey64 hashkey = board.getHash();
    for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
        const WeichiMove move(board.m_status.m_colorToPlay, *it);
        if (!board.isTTIllegalMove(move, m_state.m_ht)) { continue; }

        hashkey ^= StaticBoard::getHashGenerator().getKoKeys(*it);
    }

    return hashkey;
}

HashKey64 WeichiUctAccessor::getPathHashkeyFor7x7()
{
    const Vector<WeichiMove, MAX_GAME_LENGTH>& vMoveList = m_state.m_board.getMoveList();
    HashKey64 pahtHashkey;
    for (int i = 0; i < vMoveList.size(); ++i) {
        WeichiMove move = vMoveList[i];
        pahtHashkey ^= StaticBoard::getHashGenerator().getSequenceGameKeys(i, move.getPosition(), move.getColor());
    }

    return pahtHashkey;
}

HashKey64 WeichiUctAccessor::getStateHashkeyFor7x7(int& rotation)
{
    WeichiBoard& board = m_state.m_board;
    WeichiBitBoard bmBlack = board.getStoneBitBoard(COLOR_BLACK);
    WeichiBitBoard bmWhite = board.getStoneBitBoard(COLOR_WHITE);
    ull gueiBlack = TTentry::to64bithashFor7x7(bmBlack);
    ull gueiWhite = TTentry::to64bithashFor7x7(bmWhite);
    // Zone7x7Bitboard bm77(gueiBlack, gueiWhite);
    // rotation = bm77.normalize();

    HashKey64 hashkey;
    // for (int i = 0; i < 7; i++) {
    //	for (int j = 0; j < 7; j++) {
    //		Zone7x7Bitboard::PieceType pieceType = bm77.get(i, j);
    //		Color c;
    //		int position = 20 * i + j;
    //		switch (pieceType) {
    //		case Zone7x7Bitboard::PieceType::ZONE_EMPTY: c = COLOR_NONE; break;
    //		case Zone7x7Bitboard::PieceType::ZONE_BLACK: c = COLOR_BLACK; break;
    //		case Zone7x7Bitboard::PieceType::ZONE_WHITE: c = COLOR_WHITE; break;
    //		default: cerr << "Error Type" << endl; break;
    //		}
    //		hashkey ^= StaticBoard::getHashGenerator().getZHashKeyOf(c, position);
    //	}
    // }

    // if (board.getToPlay() == COLOR_BLACK) { hashkey ^= StaticBoard::getHashGenerator().getTurnKey(); }

    return hashkey;
}

HashKey64 WeichiUctAccessor::getGhiHashkeyFor7x7()
{
    return rand64();
}

HashKey64 WeichiUctAccessor::getZoneHashkeyFor7x7(UctNodePtr pNode)
{
    WeichiBoard& board = m_state.m_board;
    WeichiBitBoard bmRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(pNode->getProofRZoneID());
    WeichiBitBoard bmBlack = bmRZone & board.getStoneBitBoard(COLOR_BLACK);
    WeichiBitBoard bmWhite = bmRZone & board.getStoneBitBoard(COLOR_WHITE);

    // ull gueiZone = TTentry::to64bithashFor7x7(bmRZone);
    // ull gueiBlack = TTentry::to64bithashFor7x7(bmBlack);
    // ull gueiWhite = TTentry::to64bithashFor7x7(bmWhite);
    // Zone7x7Bitboard bm77(gueiZone, gueiBlack, gueiWhite);
    // bm77.normalize();

    HashKey64 hashkey;
    // for (int i = 0; i < 7; i++) {
    //	for (int j = 0; j < 7; j++) {
    //		Zone7x7Bitboard::PieceType pieceType = bm77.get(i, j);
    //		if (pieceType == Zone7x7Bitboard::PieceType::IRRELEVANT) { continue; }
    //		Color c;
    //		int position = 20 * i + j;
    //		switch (pieceType) {
    //		case Zone7x7Bitboard::PieceType::ZONE_EMPTY: c = COLOR_NONE; break;
    //		case Zone7x7Bitboard::PieceType::ZONE_BLACK: c = COLOR_BLACK; break;
    //		case Zone7x7Bitboard::PieceType::ZONE_WHITE: c = COLOR_WHITE; break;
    //		default: cerr << "Error Type" << endl; break;
    //		}
    //		hashkey ^= StaticBoard::getHashGenerator().getZHashKeyOf(c, position);
    //	}
    // }

    if (board.getToPlay() == COLOR_BLACK) { hashkey ^= StaticBoard::getHashGenerator().getTurnKey(); }

    return hashkey;
}

void WeichiUctAccessor::checkSameAncestorRZpattern(UctNodePtr pNode)
{
    WeichiBoard& board = m_state.m_board;
    Color checkColor = pNode->getColor();
    if (WeichiConfigure::use_rzone && WeichiConfigure::check_potential_ghi_for_rzone) {
        WeichiBitBoard bmRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(pNode->getProofRZoneID());
        WeichiBitBoard bmBlack = bmRZone & board.getStoneBitBoard(COLOR_BLACK);
        WeichiBitBoard bmWhite = bmRZone & board.getStoneBitBoard(COLOR_WHITE);

        int match_index = m_state.m_path.size();
        for (int i = 0; i < m_state.m_path.size() - 1; ++i) {
            int historyIndex = board.getBitBoardList().size() - m_state.m_path.size() + i;
            const Dual<WeichiBitBoard> bmHistoryBitBoard = board.getBitBoardList()[historyIndex];

            if (checkColor == m_state.m_path[i]->getColor() &&
                (bmHistoryBitBoard.m_black & bmRZone) == bmBlack &&
                (bmHistoryBitBoard.m_white & bmRZone) == bmWhite) {
                match_index = i;
                break;
            }
        }

        for (int i = match_index + 1; i < m_state.m_path.size(); ++i) {
            m_state.m_path[i]->setSameRZpatternIndex({m_state.m_path[match_index].getIndex(), m_state.m_path.back().getIndex()});
        }
    }

    return;
}

bool WeichiUctAccessor::hasZoneTTGHIProblem(const TTentry& foundEntry) const
{
    int type_solution = (foundEntry.m_status == UCT_STATUS_WIN) ? SOLUTION_PROOF : SOLUTION_DISPROOF;
    if (foundEntry.m_pNode->getSkipTTByLoop() || hasPotentialGHIproblem(type_solution, foundEntry.m_pNode)) {
        if (!WeichiConfigure::tt_optimize_for_single_ko || foundEntry.m_koPosition != -1) {
            ++WeichiGlobalInfo::getTreeInfo().m_nRZoneFailed;
            return true;
        }
    }

    return false;
}

WeichiBitBoard WeichiUctAccessor::undoAndUpdateRZoneAboveFromOpponent(int type_sol, UctNodePtr childAno, UctNodePtr parentSol)
{
    WeichiBitBoard bmUpdatedRZone = (type_sol == SOLUTION_PROOF) ? WeichiGlobalInfo::getRZoneDataTable().getRZone(childAno->getProofRZoneID()) : WeichiGlobalInfo::getRZoneDataTable().getRZone(childAno->getDisproofRZoneID());
    undoAndResumeStatus(); // now in the status of liver node
    bmUpdatedRZone = m_state.m_board.getUpdatedRZone(parentSol->getColor(), bmUpdatedRZone);
    updateNodeRZone(type_sol, parentSol, bmUpdatedRZone);

    return bmUpdatedRZone;
}

WeichiBitBoard WeichiUctAccessor::undoAndUpdateRZoneAboveFromSolSide(int type_sol, UctNodePtr childSol, UctNodePtr parentAno)
{
    WeichiBoard& board = m_state.m_board;
    // record the ko position after making the eat-ko move.
    int koPos = board.getKo();
    if (koPos != -1) { parentAno->setWinByKoPosition(childSol->getPosition()); }

    WeichiBitBoard bmRZone = type_sol == SOLUTION_PROOF ? WeichiGlobalInfo::getRZoneDataTable().getRZone(childSol->getProofRZoneID()) : WeichiGlobalInfo::getRZoneDataTable().getRZone(childSol->getDisproofRZoneID());
    WeichiMove orMove = childSol->getMove();
    bool isCaptureMove = board.isLastMoveCaptureBlock();
    undoAndResumeStatus();

    // Note: We don't always add the move position into R-zone
    // We add the move (which is not in the child R-zone) to new R-zone only if the move capture some opponent stone in the child R-zone
    WeichiBitBoard bmUpdatedRZone = bmRZone;
    WeichiBitBoard bmInfluence;
    WeichiBitBoard bmNbrOwn;

    m_state.m_board.getMoveInfluence(orMove, bmInfluence, bmNbrOwn);
    if (bmUpdatedRZone.hasIntersection(bmInfluence)) {
        bmUpdatedRZone = m_state.m_board.getMoveRZone(bmRZone, bmNbrOwn);
        bmUpdatedRZone |= bmInfluence;
    }
    updateNodeRZone(type_sol, parentAno, bmUpdatedRZone);

    return bmUpdatedRZone;
}

bool WeichiUctAccessor::isAllChildrenProved(UctNodePtr node)
{
    for (UctChildIterator<WeichiUctNode> it(node); it; ++it) {
        if (it->isUnknownStatus()) { return false; }
    }

    return true;
}

bool WeichiUctAccessor::isLikePassSelfAtari(int lossPos, int index, Vector<UctNodePtr, MAX_TREE_DEPTH>& nodesPath)
{
    if (!WeichiConfigure::use_opp1stone_suicide_likepass) { return false; }
    if (lossPos == WeichiMove::PASS_POSITION) { return true; }

    // The loss move can not change the pattern and must be captured like a pass move.
    // It can not be a make ko move.
    WeichiBoard& board = m_state.m_board;
    if (index + 1 >= nodesPath.size()) { return false; }
    if (board.isLastMoveCaptureBlock()) { return false; }
    WeichiBlock* block = board.getGrid(lossPos).getBlock();
    if (block->getNumStone() >= 2) { return false; }
    if (block->getLiberty() >= 2) { return false; }
    WeichiMove nextWinMove = nodesPath[index + 1]->getMove();
    if (nextWinMove.getPosition() != block->getLastLiberty(board.getBitBoard())) { return false; }
    if (board.isKoEatPlay(nextWinMove)) { return false; }

    return true;
}

void WeichiUctAccessor::proveChildrenOutsideRZone(UctNodePtr parentNode, int rzoneID, int type_sol, int lossPosition, bool bLikePass)
{
    if (WeichiConfigure::allow_proved_move_outside_rzone) { return; }

    WeichiBitBoard bmRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(rzoneID);
    // Do not prune when loss position is inside R-Zone
    if (!bLikePass && bmRZone.BitIsOn(lossPosition)) { return; }

    for (UctChildIterator<WeichiUctNode> it(parentNode); it; ++it) {
        if (it->getPosition() == lossPosition) { continue; }
        // Skip proved and inside R-zone
        if (it->isProved() && bmRZone.BitIsOn(it->getMove().getPosition())) { continue; }
        if (bmRZone.BitIsOn(it->getMove().getPosition()) && it->getPosition() != WeichiMove::PASS_POSITION) { continue; }

        // outside the R-zone
        // 1. If not pruned ever, just update.
        // 2. If ever pruned, update the smaller one.

        if (it->isRZonePruned()) {
            int itRZoneID = (type_sol == SOLUTION_PROOF) ? it->getProofRZoneID() : it->getDisproofRZoneID();
            WeichiBitBoard itRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(itRZoneID);
            if (bmRZone.bitCount() < itRZone.bitCount()) {
                it->setEqualLoss(lossPosition);
                if (type_sol == SOLUTION_PROOF) {
                    it->setProofRZoneID(rzoneID);
                } else if (type_sol == SOLUTION_DISPROOF) {
                    it->setDisproofRZoneID(rzoneID);
                }
            }
        } else {
            // not ever pruned, including pass
            it->setEqualLoss(lossPosition);
            if (type_sol == SOLUTION_PROOF) {
                it->setProofRZoneID(rzoneID);
            } else if (type_sol == SOLUTION_DISPROOF) {
                it->setDisproofRZoneID(rzoneID);
            }
            it->setUctNodeStatus(UCT_STATUS_RZONE_PRUNED);
        }
    }

    return;
}

void WeichiUctAccessor::updateNodeRZone(int type_sol, UctNodePtr node, WeichiBitBoard bmZone)
{
    if (!WeichiConfigure::use_rzone) { return; }

    WeichiBoard& board = m_state.m_board;

    if (type_sol == SOLUTION_PROOF) {
        if (node->getProofRZoneID() == -1) {
            int id = WeichiGlobalInfo::getRZoneDataTable().createRZoneData(bmZone, board);
            node->setProofRZoneID(id);
        } else {
            WeichiGlobalInfo::getRZoneDataTable().appendRZoneData(node->getProofRZoneID(), bmZone, board);
        }
    } else if (type_sol == SOLUTION_DISPROOF) {
        if (node->getDisproofRZoneID() == -1) {
            int id = WeichiGlobalInfo::getRZoneDataTable().createRZoneData(bmZone, board);
            node->setDisproofRZoneID(id);
        } else {
            WeichiGlobalInfo::getRZoneDataTable().appendRZoneData(node->getDisproofRZoneID(), bmZone, board);
        }
    }

    return;
}

WeichiUctAccessor::UctNodePtr WeichiUctAccessor::getTerminalNode(UctNodePtr pNode)
{
    if (!WeichiConfigure::use_immediate_win) { return UctNodePtr::NULL_PTR; }

    // For proof
    Color liveColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
    Color oppColor = AgainstColor(liveColor);
    if (pNode->getColor() == oppColor && pNode->getCNNVNValue() <= (-1.0f) * WeichiConfigure::win_vn_threshold) {
        WeichiBitBoard bmCandidate = m_state.m_lifedeathHandler.getFullBoardImmediateWin();
        for (UctChildIterator<WeichiUctNode> it(pNode); it; ++it) {
            if (it->isCNNNode() || it->getCheckImmedaiteWin()) { continue; }
            if (!bmCandidate.BitIsOn(it->getPosition())) { continue; }

            it->setCheckImmediateWin(true);
            playAndStoreStatus(it);
            bool bTerminalWin = m_state.isTerminalWin();
            undoAndResumeStatus();

            if (bTerminalWin) { return it; }
        }
    }

    // For disproof
    if (!WeichiConfigure::mcts_use_disproof || pNode->getColor() != liveColor) { return UctNodePtr::NULL_PTR; }

    WeichiBitBoard bmLastLiberty = m_state.m_lifedeathHandler.getFullBoardImmediateLoss();
    if (bmLastLiberty.bitCount() == 1) {
        int lastLib = bmLastLiberty.bitScanForward();
        for (UctChildIterator<WeichiUctNode> it(pNode); it; ++it) {
            if (it->getPosition() == lastLib) { return it; }
        }
    }

    return UctNodePtr::NULL_PTR;
}

WeichiBitBoard WeichiUctAccessor::getOwnCrucialMoves(UctNodePtr parentNode)
{
    WeichiBitBoard bmCandidate;
    for (UctChildIterator<WeichiUctNode> it(parentNode); it; ++it) {
        if (it->isProved()) { continue; }

        bmCandidate.SetBitOn(it->getPosition());
    }

    WeichiBitBoard bmOwnCrucialMoves = bmCandidate;
    // If has potential rzone, just use potential rzone.
    if (WeichiConfigure::use_potential_rzone && parentNode->hasPotentialRZone()) {
        WeichiBitBoard bmPotentialRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(parentNode->getPotentialRZoneID());
        bmOwnCrucialMoves &= bmPotentialRZone;
        // To Rock: If empty, check KO again and return bitboard.
        if (!bmOwnCrucialMoves.empty()) {
            // avoid over-focus
            // 1. There's no good child in the potential rzone
            // 2. ParentNode has enough visit count
            if (!isPotentialRZoneHasGoodChild(parentNode) && parentNode->getUctData().getCount() >= WeichiConfigure::potential_rzone_count_limit) {
                return bmCandidate;
            } else {
                return bmOwnCrucialMoves;
            }
        } else {
            return bmCandidate;
        }
    }

    return bmCandidate;
}

WeichiBitBoard WeichiUctAccessor::getOppCrucialMoves(UctNodePtr parentNode)
{
    WeichiBitBoard bmCandidate;
    for (UctChildIterator<WeichiUctNode> it(parentNode); it; ++it) {
        if (it->isProved()) { continue; }

        bmCandidate.SetBitOn(it->getPosition());
    }

    // If has potential rzone, just use potential rzone.
    WeichiBitBoard bmOppCrucialMoves = bmCandidate;
    if (WeichiConfigure::use_potential_rzone && parentNode->hasPotentialRZone()) {
        WeichiBitBoard bmPotentialRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(parentNode->getPotentialRZoneID());

        bmOppCrucialMoves &= bmPotentialRZone;
        if (!bmOppCrucialMoves.empty()) {
            return bmOppCrucialMoves;
        } else {
            return bmCandidate;
        }
    }

    return bmCandidate;
}

bool WeichiUctAccessor::isPotentialRZoneHasGoodChild(UctNodePtr parentNode)
{
    if (!WeichiConfigure::use_potential_rzone) { return false; }

    double dBestWinrate = 0.0f;
    for (UctChildIterator<WeichiUctNode> it(parentNode); it; ++it) {
        WeichiBitBoard bmPotential = WeichiGlobalInfo::getRZoneDataTable().getRZone(parentNode->getPotentialRZoneID());
        if (bmPotential.BitIsOn(it->getPosition()) && it->getUctData().getMean() >= dBestWinrate) {
            dBestWinrate = it->getUctData().getMean();
        }
    }

    if (dBestWinrate >= WeichiConfigure::potential_rzone_goodchild_threshold) { return true; }

    return false;
}

bool WeichiUctAccessor::isPotentialWin(UctNodePtr node, double parentCNNVNValue)
{
    if (!WeichiConfigure::use_potential_rzone) { return false; }

    Color proofColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
    Color oppColor = AgainstColor(proofColor);

    if ((node->getColor() == proofColor && parentCNNVNValue <= (-1.0f) * WeichiConfigure::win_vn_threshold) ||
        (node->getColor() == oppColor && parentCNNVNValue >= WeichiConfigure::win_vn_threshold)) {
        return true;
    }

    return false;
}