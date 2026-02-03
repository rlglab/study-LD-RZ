#include "WeichiGlobalInfo.h"
#include "UctChildIterator.h"
#include "WeichiDynamicKomi.h"

WeichiGlobalInfo* WeichiGlobalInfo::s_instance = NULL;
string WeichiGlobalInfo::TreeInfo::getNodeString(const UctNodeOrder& nodeOrder, int deep)
{
    ostringstream oss;
    for (int i = 0; i < deep; i++) { oss << " "; }
    oss << "(" << deep << ")"
        << "{" << setw(3) << nodeOrder.m_pNode->getMove().toGtpString()
        << "(" << toChar(nodeOrder.m_pNode->getMove().getColor()) << ")"
        << "," << (nodeOrder.m_pNode->isCNNNode() ? "C" : (nodeOrder.m_pNode->isInCNNQueue() ? "Q" : " "))
        << "," << setw(2) << nodeOrder.m_order << "} "

        // po
        << setprecision(4) << nodeOrder.m_pNode->getUctDataWithoutVirtualLoss().toString()

        // vn
        << ((WeichiConfigure::use_playout) ? (" " + nodeOrder.m_pNode->getVNValueData().toString()) : "")

        << " {prob: " << nodeOrder.m_pNode->getProb()
        << ", vn: " << nodeOrder.m_pNode->getCNNVNValue();

    if (WeichiConfigure::mcts_use_rave) {
        oss << ", rave: " << nodeOrder.m_pNode->getRaveData().getMean()
            << "/" << nodeOrder.m_pNode->getRaveData().getCount() << "}";
    }

    oss << "}";

    return oss.str();
}

vector<UctNodeOrder> WeichiGlobalInfo::TreeInfo::getChildOrder(UctNodePtr pNode)
{
    vector<UctNodeOrder> vChild;

    if (pNode.isValid() && pNode->hasChildren()) {
        int order = 1;
        for (UctChildIterator<WeichiUctNode> it(pNode); it; ++it, ++order) { vChild.push_back(UctNodeOrder(order, it)); }
        sort(vChild.begin(), vChild.end());
    }

    return vChild;
}

void WeichiGlobalInfo::SearchInfo::clearPolicyStat()
{
    for (uint i = 0; i < POLICY_SIZE; i++) {
        m_policyStat[i].get(COLOR_BLACK) = m_policyStat[i].get(COLOR_WHITE) = 0;
    }
}

void WeichiGlobalInfo::cleanSummaryInfo()
{
    m_treeInfo.m_bIsEarlyAbort = false;
    m_treeInfo.m_nDCNNJob = 0;
    m_treeInfo.m_nMaxTreeWidth = 0;
    m_treeInfo.m_nMaxTreeHeight = 0;
    m_treeInfo.m_nMaxChildIndex = 0;
    m_treeInfo.m_nHandMove = 0;
    m_treeInfo.m_nSimulation = 0;
    m_treeInfo.m_nLeafBenson = 0;
    m_treeInfo.m_nTerminal = 0;
    m_treeInfo.m_nTTmatch = 0;
    m_treeInfo.m_nKos = 0;
    m_treeInfo.m_nLongKos = 0;
    m_treeInfo.m_nRZoneFailed = 0;
    m_treeInfo.m_dThinkTime = 0.0;
    m_treeInfo.m_treeWidth.reset();
    m_treeInfo.m_treeHeight.reset();
    m_treeInfo.m_childIndex.setAllAs(0, MAX_NUM_GRIDS);
    m_searchInfo.m_territory.clear();
    m_searchInfo.m_passTerritory.clear();
    m_searchInfo.clearPolicyStat();
    m_moveChanged.clear();
    m_vRootSimInfo.clear();
    m_mEarlylifeInfo.clear();
    m_timer.reset();
    m_RZoneDataTable.reset();
    m_boardTT.clear();
    m_hierTT.reset();
    m_LinearTT.reset();
    m_doCNNForward = 0;

    return;
}

void WeichiGlobalInfo::logInfo(const WeichiMove& move)
{
    m_treeInfo.m_predictOrder = predictOpponentMoveOrder(move);
    m_treeInfo.m_predictSD = predictOppoentMoveInfo(move);
}

void WeichiGlobalInfo::logPredictData()
{
    m_treeInfo.m_vPredictOrder.setAllAs(TreeInfo::OUT_OF_ORDER, MAX_NUM_GRIDS);
    m_treeInfo.m_vPredictInfoOrder.setAllAs(StatisticData(), MAX_NUM_GRIDS);

    vector<UctNodeOrder> vChild = m_treeInfo.getChildOrder(m_treeInfo.m_pRoot);
    if (vChild.empty()) { return; }

    UctNodePtr pBest = vChild[0].m_pNode;
    vChild = m_treeInfo.getChildOrder(pBest);
    for (uint i = 0; i < vChild.size(); i++) {
        const WeichiMove& move = vChild[i].m_pNode->getMove();
        m_treeInfo.m_vPredictOrder[move.getPosition()] = i + 1;
        m_treeInfo.m_vPredictInfoOrder[move.getPosition()] = vChild[i].m_pNode->getUctData();
    }
}

string WeichiGlobalInfo::getSearchSummaryInfoString(const WeichiBoard& board)
{
    ostringstream oss;
    Vector<uint, MAX_NUM_GRIDS> vSequenceNumber;
    Vector<uint, MAX_NUM_GRIDS> vSLCandidates = getRootSLCandidates();
    vector<string> vBestSequence = getBestSequenceString(vSequenceNumber);
    vector<string> vBoardSequence = getBoardStringWithSequence(board, vSequenceNumber);
    vector<string> vBoardString = getBoardString(board);
    vector<string> vBoardTerritory = getBoardStringWithTerritory(board);
    vector<string> vBoardSLCandidates = getBoardStringWithSLCandidates(board, vSLCandidates);
    vector<string> vSearchTreeSummary = getSearchTreeSummaryString(board);

    uint lines = 0;
    const int SPLIT_LINE_SIZE = 89;

    vector<string> vLogDetail = vSearchTreeSummary;
    vLogDetail.insert(vLogDetail.end(), vBestSequence.begin(), vBestSequence.end());
    // print board string & board sequence
    for (uint i = 0; i < vBoardString.size(); i++) {
        oss << vBoardString[i] << " | " << vBoardSequence[i] << " | ";
        if (lines < vLogDetail.size()) { oss << vLogDetail[lines++]; }
        oss << endl;
    }

    // print split line
    for (uint i = 0; i < SPLIT_LINE_SIZE; i++) { oss << "-"; }
    if (lines < vLogDetail.size()) { oss << " " << vLogDetail[lines++]; }
    oss << endl;

    // print board territory & SL candidates
    for (uint i = 0; i < vBoardTerritory.size(); i++) {
        oss << vBoardTerritory[i] << " | " << vBoardSLCandidates[i] << " | ";
        if (lines < vLogDetail.size()) { oss << vLogDetail[lines++]; }
        oss << endl;
    }

    // Best move info
    oss << "[" << setw(3) << board.getMoveList().size() << "] Best Move: " << m_treeInfo.m_bestMove.toGtpString()
        << "  PO(" << ToString(m_treeInfo.m_bestSD.getMean()) << "/" << ToString(m_treeInfo.m_bestSD.getCount()) << ")";
    if (WeichiConfigure::use_playout) {
        oss << " VN(" << ToString(m_treeInfo.m_bestValueSD.getMean()) << "/" << ToString(m_treeInfo.m_bestValueSD.getCount()) << ")"
            << " AVG(" << ToString((m_treeInfo.m_bestSD.getMean() + m_treeInfo.m_bestValueSD.getMean()) / 2) << ")";
    }
    oss << endl;

    // Territory difference info
    if (WeichiConfigure::use_playout) {
        calculateSureTerritory();
        oss << "[Territory] Black = " << setw(3) << m_treeInfo.m_fTerritory.m_black
            << "; White = " << setw(3) << m_treeInfo.m_fTerritory.m_white
            << "; Total = " << setw(3) << (m_treeInfo.m_fTerritory.m_black + m_treeInfo.m_fTerritory.m_white)
            << "; Diff(B-W-komi) = " << setw(3) << (m_treeInfo.m_fTerritory.m_black - m_treeInfo.m_fTerritory.m_white - WeichiConfigure::komi) << endl;
    }

    string ss;
    ss = "Child index: (max = " + ToString(m_treeInfo.m_nMaxChildIndex) + ")";
    for (int i = 1; i <= m_treeInfo.m_nMaxChildIndex; i++) {
        ss += "(" + ToString(i) + ": " + ToString(m_treeInfo.m_childIndex[i]) + ")";
    }
    oss << ss << endl;

    return oss.str();
}

string WeichiGlobalInfo::getPolicyStatString(vector<WeichiPlayoutPolicy> vPrintPolicy)
{
    ostringstream oss;
    uint nBlack, nWhite, nTotal;

    // count total number
    nBlack = nWhite = 0;
    for (uint i = 0; i < POLICY_SIZE; i++) {
        nBlack += m_searchInfo.m_policyStat[i].get(COLOR_BLACK);
        nWhite += m_searchInfo.m_policyStat[i].get(COLOR_WHITE);
    }
    nTotal = nBlack + nWhite;

    // print policy
    uint size = (vPrintPolicy.size() == 0) ? POLICY_SIZE : static_cast<uint>(vPrintPolicy.size());
    for (uint i = 0; i < size; i++) {
        WeichiPlayoutPolicy policy = (size == POLICY_SIZE) ? static_cast<WeichiPlayoutPolicy>(i) : static_cast<WeichiPlayoutPolicy>(vPrintPolicy[i]);
        int numBlack = m_searchInfo.m_policyStat[policy].get(COLOR_BLACK);
        int numWhite = m_searchInfo.m_policyStat[policy].get(COLOR_WHITE);
        oss << getWeichiPlayoutPolicyString(policy) << " -> "
            << "B: " << (double)numBlack * 100 / nBlack << "%, "
            << "W: " << (double)numWhite * 100 / nWhite << "%, "
            << "ALL: " << (double)(numBlack + numWhite) * 100 / nTotal << "%" << endl;
    }

    return oss.str();
}

vector<string> WeichiGlobalInfo::getBoardString(const WeichiBoard& board)
{
    vector<string> vString;

    vString.push_back(board.toBoardCoordinateString(true));
    for (int row = WeichiConfigure::BoardSize; row > 0; row--) { vString.push_back(board.toOneRowBoardString(row, true)); }
    vString.push_back(board.toBoardCoordinateString(true));
    if (WeichiConfigure::ShowBoardUpsideDown) { std::reverse(vString.begin(), vString.end()); }

    return vString;
}

vector<string> WeichiGlobalInfo::getBoardStringWithTerritory(const WeichiBoard& board)
{
    vector<string> vString;

    vString.push_back(board.toBoardCoordinateString(true));
    for (int row = WeichiConfigure::BoardSize; row > 0; row--) { vString.push_back(board.toOneRowTerritoryString(row, true)); }
    vString.push_back(board.toBoardCoordinateString(true));
    if (WeichiConfigure::ShowBoardUpsideDown) { std::reverse(vString.begin(), vString.end()); }

    return vString;
}

vector<string> WeichiGlobalInfo::getBoardStringWithSequence(const WeichiBoard& board, const Vector<uint, MAX_NUM_GRIDS>& vSequenceNumber)
{
    vector<string> vString;

    vString.push_back(board.toBoardCoordinateString(true));
    for (int row = WeichiConfigure::BoardSize; row > 0; row--) {
        vString.push_back(board.toOneRowBoardString(row, vSequenceNumber, m_treeInfo.m_bestMove.getColor(), true, true));
    }
    vString.push_back(board.toBoardCoordinateString(true));
    if (WeichiConfigure::ShowBoardUpsideDown) { std::reverse(vString.begin(), vString.end()); }

    return vString;
}

vector<string> WeichiGlobalInfo::getBoardStringWithSLCandidates(const WeichiBoard& board, const Vector<uint, MAX_NUM_GRIDS>& vSLCandidates)
{
    vector<string> vString;

    vString.push_back(board.toBoardCoordinateString(true));
    for (int row = WeichiConfigure::BoardSize; row > 0; row--) {
        vString.push_back(board.toOneRowBoardString(row, vSLCandidates, m_treeInfo.m_bestMove.getColor(), false, true));
    }
    vString.push_back(board.toBoardCoordinateString(true));
    if (WeichiConfigure::ShowBoardUpsideDown) { std::reverse(vString.begin(), vString.end()); }

    return vString;
}

vector<string> WeichiGlobalInfo::getSearchTreeSummaryString(const WeichiBoard& board)
{
    string ss;
    vector<string> vString;

    int order = m_treeInfo.m_predictOrder;
    if (order == TreeInfo::OUT_OF_ORDER) {
        ss = "out of predict opponent move";
    } else {
        StatisticData predictSD = m_treeInfo.m_predictSD;
        ss = "Predict opponent move order is " + ToString(order) + " (" + ToString(predictSD.getMean()) + "/" + ToString(predictSD.getCount()) + ")";
    }
    vString.push_back(ss);

    ss = "Think time: " + ToString(m_treeInfo.m_dThinkTime) + " seconds (" + (m_treeInfo.m_bIsEarlyAbort ? "early abort" : "no early abort") + ")";
    vString.push_back(ss);

    ss = "Total " + ToString(m_treeInfo.m_nSimulation) + " playouts (" + ToString(m_treeInfo.m_nSimulation / m_treeInfo.m_dThinkTime) + " games/s)";
    vString.push_back(ss);

    ss = "DCNN:";
    vString.push_back(ss);

    ss = "  CNN Queue Size: " + ToString(m_CNNJobQueue.getQueueSize()) + "; CNN Job done: " + ToString(m_treeInfo.m_nDCNNJob) + "(" + ToString(m_treeInfo.m_nDCNNJob / m_treeInfo.m_dThinkTime) + " node/s)";
    vString.push_back(ss);

    ss = "Avg tree height: " + ToString(m_treeInfo.m_treeHeight.getMean()) + " nodes (max = " + ToString(m_treeInfo.m_nMaxTreeHeight) + ")";
    ss += ", tree width: " + ToString(m_treeInfo.m_treeWidth.getMean()) + " nodes (max = " + ToString(m_treeInfo.m_nMaxTreeWidth) + ")";
    ss += ", game length: " + ToString(m_treeInfo.m_nHandMove / (double)m_treeInfo.m_nSimulation) + " hands";
    vString.push_back(ss);

    if (WeichiConfigure::use_dynamic_komi) {
        ss = "Dynamic komi change: " + ToString(WeichiDynamicKomi::Old_Internal_komi) + " -> " + ToString(WeichiDynamicKomi::Internal_komi);
        vString.push_back(ss);
    } else {
        ss = "Current komi: " + ToString(WeichiDynamicKomi::Internal_komi);
        vString.push_back(ss);
    }

    ss = "Dynamic expansion threshold: " + ToString(Configure::ExpandThreshold);
    vString.push_back(ss);

    ss = "Cache:";
    vString.push_back(ss);

    ss = "  (Before clean) Trigger use " + m_searchInfo.m_moveCache.getTriggerUseRateStringBeforeClean();
    vString.push_back(ss);

    ss = "  (After clean) Trigger use " + m_searchInfo.m_moveCache.getTriggerUseRateString();
    vString.push_back(ss);

    ss = "  ";
    if (WeichiConfigure::ShowPlayoutPolicyStatistic) {
        vector<WeichiPlayoutPolicy> vPolicy;
        vPolicy.push_back(POLICY_CACHE);
        vPolicy.push_back(POLICY_CACHE_CREATE);
        ss += WeichiGlobalInfo::get()->getPolicyStatString(vPolicy);
    }
    vString.push_back(ss);

    ss = "RZoneDataTable Usage: ";
    ss += ToString(100.0f * (float(getRZoneDataTable().getSize()) / float(getRZoneDataTable().getMaximum()))) + "%";
    ss += " (" + ToString(getRZoneDataTable().getSize()) + "/" + ToString(getRZoneDataTable().getMaximum()) + ")";
    vString.push_back(ss);

    return vString;
}

vector<string> WeichiGlobalInfo::getBestSequenceString(Vector<uint, MAX_NUM_GRIDS>& vSequenceNumber)
{
    int deep = 1;
    vector<string> vBestSequence;

    vBestSequence.push_back("===== Best Sequence ======");
    vSequenceNumber.setAllAs(-1, MAX_NUM_GRIDS);
    traverseBestSequence(vBestSequence, m_treeInfo.m_pRoot, deep, vSequenceNumber, true);

    return vBestSequence;
}

void WeichiGlobalInfo::calculateSureTerritory()
{
    m_treeInfo.m_fTerritory.m_black = m_treeInfo.m_fTerritory.m_white = 0;
    const Territory& territory = WeichiGlobalInfo::getSearchInfo().m_territory;
    for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
        const double dTerritory = territory.getTerritory(*it);
        if (dTerritory > 0) {
            m_treeInfo.m_fTerritory.m_black += dTerritory;
        } else if (dTerritory < 0) {
            m_treeInfo.m_fTerritory.m_white -= dTerritory;
        }
    }
}

Vector<uint, MAX_NUM_GRIDS> WeichiGlobalInfo::getRootSLCandidates()
{
    Vector<uint, MAX_NUM_GRIDS> vSLCandidates;
    vector<UctNodeOrder> vChild = m_treeInfo.getChildOrder(m_treeInfo.m_pRoot);

    vSLCandidates.setAllAs(-1, MAX_NUM_GRIDS);
    for (uint i = 0; i < vChild.size(); i++) {
        uint position = vChild[i].m_pNode->getMove().getPosition();
        vSLCandidates[position] = vChild[i].m_order;
    }

    return vSLCandidates;
}

uint WeichiGlobalInfo::predictOpponentMoveOrder(const WeichiMove& move)
{
    if (move.isPass() || m_treeInfo.m_vPredictOrder.size() == 0) { return TreeInfo::OUT_OF_ORDER; }
    return m_treeInfo.m_vPredictOrder[move.getPosition()];
}

StatisticData WeichiGlobalInfo::predictOppoentMoveInfo(const WeichiMove& move)
{
    if (move.isPass() || m_treeInfo.m_vPredictOrder.size() == 0) { return StatisticData(); }
    return m_treeInfo.m_vPredictInfoOrder[move.getPosition()];
}

void WeichiGlobalInfo::traverseBestSequence(vector<string>& vBestSequence, UctNodePtr root, int deep, Vector<uint, MAX_NUM_GRIDS>& vSequenceNumber, bool bIsMainSequence /*=false*/)
{
    if (root.isNull() || !root->hasChildren()) { return; }
    if (!bIsMainSequence) { return; }
    if (deep > WeichiConfigure::ShowBoardBestSequenceLimit) { return; }

    vector<UctNodeOrder> vChild = m_treeInfo.getChildOrder(root);
    if (vChild[0].m_pNode->getUctData().getCount() == 0) { return; }

    vBestSequence.push_back(m_treeInfo.getNodeString(vChild[0], deep));
    uint position = vChild[0].m_pNode->getMove().getPosition();
    if (vSequenceNumber[position] == -1 && deep + 1 <= WeichiConfigure::ShowBoardBestSequenceLimit) { vSequenceNumber[position] = deep; }
    traverseBestSequence(vBestSequence, vChild[0].m_pNode, deep + 1, vSequenceNumber, true);

    if (deep >= 3) { return; }
    for (uint i = 1; i < vChild.size(); i++) {
        // always show first 5 child when deep = 1, and show first 3 child when deep = 2
        if (vChild[i].m_pNode->getUctData().getCount() < root->getUctData().getCount() * 0.1 && (i >= 7 - deep * 2)) { return; }
        vBestSequence.push_back(m_treeInfo.getNodeString(vChild[i], deep));
        traverseBestSequence(vBestSequence, vChild[i].m_pNode, deep + 1, vSequenceNumber, false);
    }
}
