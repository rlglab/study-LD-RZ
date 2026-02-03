#include "RZoneTranspositionTable.h"
#include "WeichiGlobalInfo.h"
#include "WeichiThreadState.h"

void RZoneHierarchicalTT::store(const TTentry& entry)
{
    ++m_nStore;
    m_storeTimer.start();
    store_(entry, getRoot(), 0);
    m_storeTimer.stopAndAddAccumulatedTime();

    return;
}

void RZoneHierarchicalTT::reconstructStore(const TTentry& entry)
{
    ++m_nReconstructStore;
    m_storeTimer.start();
    store_(entry, getRoot(), 0);
    m_storeTimer.stopAndAddAccumulatedTime();

    return;
}

bool RZoneHierarchicalTT::lookup(const WeichiThreadState& state, TTentry& entry, UctNodePtr pNode)
{
    ++m_nLookup;
    m_lookupTimer.start();
    getRoot()->m_nVisit++;
    int nNum = 0;
    TTstatistic stat;
    bool bResult = false;
    bResult = lookup_(state, entry, 0, 0, pNode->getRZoneTTQueryEntryStartID(), nNum, false, stat);
    if (bResult) { stat.m_tree.m_nFound = 1; }
    addSummaryStatistic(stat, pNode->getRZoneTTQueryEntryStartID());
    pNode->setRZoneTTQueryEntryStartID(getTTSize());
    m_avgTraverse.add(nNum);
    m_lookupTimer.stopAndAddAccumulatedTime();
    if (bResult) { ++m_nHit; }

    // [NoTS Region Start]
    m_lookupNoTSTimer.start();
    bool ts_skip_flag = WeichiConfigure::use_rzone_tt_skip_on_id_diff;
    WeichiConfigure::use_rzone_tt_skip_on_id_diff = false;
    TTentry nots_entry;
    TTstatistic no_ts_stat;
    bool bResult2 = lookup_(state, nots_entry, 0, 0, pNode->getRZoneTTQueryEntryStartID(), nNum, false, no_ts_stat);
    if (bResult2) { no_ts_stat.m_tree.m_nFound = 1; }
    addSummaryStatistic2(no_ts_stat, pNode->getRZoneTTQueryEntryStartID());
    WeichiConfigure::use_rzone_tt_skip_on_id_diff = ts_skip_flag;
    m_lookupNoTSTimer.stopAndAddAccumulatedTime();
    // [NoTS Region End]

    return bResult;
}

void RZoneHierarchicalTT::reConstruct()
{
    if (m_vEntry.size() <= PHASE1_RECONSTRUCT_SIZE) {
        if (m_vEntry.size() % (100 * (m_nReconstruct + 1)) != 0) { return; }
    } else { // > 1000
        if (m_vEntry.size() < float(m_nPreviousConstructSize) * RECONSTRUCT_SIZE_RATIO) {
            return;
        } else {
            m_nPreviousConstructSize *= RECONSTRUCT_SIZE_RATIO;
        }
    }

    ++m_nReconstruct;

    WeichiBitBoard bmCrucial;
    Heatmap heatmap = m_heatmap;
    vector<int> vCrucialStones;
    int sum = heatmap.getSum(), acc_sum = 0;
    while (!heatmap.empty()) {
        int pos = heatmap.getHottestPosition();
        acc_sum += heatmap.getCounts(pos);

        vCrucialStones.push_back(pos);
        bmCrucial.SetBitOn(pos);
        heatmap.setCounts(pos, 0);

        if (vCrucialStones.size() == MAX_LAYERS) { break; }
        if (acc_sum * 1.0f / sum >= CRUCIAL_RATIO) { break; }
    }

    CERR() << m_nReconstruct << ": Entry Size: " << m_vEntry.size();
    if (m_vCrucialStones == vCrucialStones) {
        m_vReconstruct.push_back(pair<int, bool>(m_nReconstruct, false));
        CERR() << ", the tree does not change due to the same crucial stone order." << std::endl;
        return;
    }
    CERR() << std::endl;
    m_vReconstruct.push_back(pair<int, bool>(m_nReconstruct, true));

    vector<TTentry> vEntryBackup = m_vEntry;
    initializeTreeStructure();
    m_vCrucialStones = vCrucialStones;
    m_heatmap.setCrucialBitBoard(bmCrucial);
    for (int i = 0; i < vEntryBackup.size(); ++i) {
        if (WeichiConfigure::use_rzone_tt_skip_larger_pattern && vEntryBackup[i].m_bSkip) {
            ++m_nSkipStore;
            m_vEntry.push_back(vEntryBackup[i]); // still push_back, but not recrod in the tree by store
            continue;
        }

        reconstructStore(vEntryBackup[i]);
    }

    return;
}

void RZoneHierarchicalTT::debugDisplay()
{
    COUT() << "Entry size: " << m_vEntry.size() << " = " << getRoot()->m_nEntry << endl;
    COUT() << "Node size: " << m_nNode << endl;
    COUT() << "Query num: " << m_nEntryCompare << endl;
    COUT() << "Skip Query num: " << m_nSkipEntryCompare << endl;
    COUT() << "RepeatRZone num: " << m_nLargeRZonePattern << endl;
    COUT() << "Reconstruct num: " << m_nReconstruct << endl;
    COUT() << "Avg traverse num: " << m_avgTraverse.getMean() << endl;
    COUT() << "Lookup num: " << fixed << m_avgTraverse.getCount() << endl;
    COUT() << "Crucial stone: " << endl;
    for (int i = 0; i < m_vCrucialStones.size(); ++i) {
        int pos = m_vCrucialStones[i];
        COUT() << WeichiMove(pos).toGtpString() << " " << m_heatmap.getCounts(pos) << endl;
    }

    COUT() << m_heatmap.toString() << endl;

    return;
}

ull RZoneHierarchicalTT::getNumBuckets(int nodeID) const
{
    if (nodeID == -1) { return 0; }

    const RZoneTtNode* node = &m_vNodes[nodeID];
    if (node->m_vEntry.size() != 0) { return 1; }

    int totalBuckets = 0;
    for (int i = 0; i < COLOR_SIZE; ++i) {
        totalBuckets += getNumBuckets(node->m_children[i]);
    }

    return totalBuckets;
}

ull RZoneHierarchicalTT::getNumMaxBucketSize() const
{
    int numMaxBucketSize = 0;
    traverseNumMaxBucketSize_(0, numMaxBucketSize);
    return numMaxBucketSize;
}

string RZoneHierarchicalTT::getBucketEntryString() const
{
    int bucketSize = 0;
    WeichiBitBoard bmEmpty, bmBlack, bmWhite;
    return getBucketEntryString_(0, 0, bucketSize, bmEmpty, bmBlack, bmWhite);
}

string RZoneHierarchicalTT::getTableStructureStr(int nodeID, int& nTotalNode) const
{
    if (nodeID == -1) {
        nTotalNode = 0;
        return "0:0()";
    }

    string sResult = "";
    const RZoneTtNode* node = &m_vNodes[nodeID];
    int nNodes[4];
    for (int i = 0; i < COLOR_SIZE; ++i) {
        nNodes[visitOrder[i]] = 0;
        switch (visitOrder[i]) {
            case COLOR_NONE: sResult += "E:"; break;
            case COLOR_BLACK: sResult += "B:"; break;
            case COLOR_WHITE: sResult += "W:"; break;
            case COLOR_BORDER: sResult += "N:"; break;
        }
        sResult += getTableStructureStr(node->m_children[visitOrder[i]], nNodes[visitOrder[i]]);
        nTotalNode += nNodes[visitOrder[i]];
    }

    nTotalNode += 1;
    sResult = ToString(nTotalNode) + ":" + ToString(node->m_nEntry) + "(" + sResult + ")";

    return sResult;
}

string RZoneHierarchicalTT::getTableNodeStructureStr(int nodeID, int& totalNodes) const
{
    if (nodeID == -1) {
        totalNodes = 0;
        return "0()";
    }

    string sResult = "";
    const RZoneTtNode* node = &m_vNodes[nodeID];
    int nNodes[4];
    for (int i = 0; i < COLOR_SIZE; ++i) {
        nNodes[visitOrder[i]] = 0;
        switch (visitOrder[i]) {
            case COLOR_NONE: sResult += "E:"; break;
            case COLOR_BLACK: sResult += "B:"; break;
            case COLOR_WHITE: sResult += "W:"; break;
            case COLOR_BORDER: sResult += "N:"; break;
        }
        sResult += getTableNodeStructureStr(node->m_children[visitOrder[i]], nNodes[visitOrder[i]]);
        totalNodes += nNodes[visitOrder[i]];
    }

    totalNodes += 1;
    sResult = ToString(totalNodes) + "(" + sResult + ")";

    return sResult;
}

string RZoneHierarchicalTT::getTableEntryStructureStr(int nodeID) const
{
    if (nodeID == -1) {
        return "0()";
    }

    string sResult = "";
    const RZoneTtNode* node = &m_vNodes[nodeID];
    int totalEntrys = 0;
    for (int i = 0; i < COLOR_SIZE; ++i) {
        switch (visitOrder[i]) {
            case COLOR_NONE: sResult += "E:"; break;
            case COLOR_BLACK: sResult += "B:"; break;
            case COLOR_WHITE: sResult += "W:"; break;
            case COLOR_BORDER: sResult += "N:"; break;
        }
        sResult += getTableEntryStructureStr(node->m_children[visitOrder[i]]);
        if (node->m_children[visitOrder[i]] != -1) {
            int childID = node->m_children[visitOrder[i]];
            totalEntrys += m_vNodes[childID].m_nEntry;
        }
    }
    sResult = ToString(totalEntrys) + "(" + sResult + ")";

    return sResult;
}

string RZoneHierarchicalTT::getOutputSummaryStatistic() const
{
    ostringstream oss;
    for (vector<TTSumary>::const_iterator it = m_vSummary.begin(); it != m_vSummary.end(); ++it) {
        oss << it->toString();
    }
    return oss.str();
}

string RZoneHierarchicalTT::getOutputSummaryStatistic2() const
{
    ostringstream oss;
    for (vector<TTSumary>::const_iterator it = m_vSummary2.begin(); it != m_vSummary2.end(); ++it) {
        oss << it->toString();
    }
    return oss.str();
}

string RZoneHierarchicalTT::getSumNodesFromRoot() const
{
    ostringstream oss;
    getSumNodes(0, 0, oss);
    return oss.str();
}

string RZoneHierarchicalTT::getReconstructInfo() const
{
    ostringstream oss;
    for (int i = 0; i < m_vReconstruct.size(); ++i) {
        oss << m_vReconstruct[i].first << "\t" << m_vReconstruct[i].second << std::endl;
    }

    return oss.str();
}

void RZoneHierarchicalTT::getSumNodes(int nodeID, int depth, ostringstream& oss) const
{
    const RZoneTtNode* node = &m_vNodes[nodeID];
    oss << node->m_nSumNodes << "\t";

    int blackID = node->m_children[COLOR_BLACK];
    int whiteID = node->m_children[COLOR_WHITE];
    int emptyID = node->m_children[COLOR_NONE];
    int noneID = node->m_children[COLOR_BORDER];

    if (blackID != -1) {
        oss << m_vNodes[blackID].m_nSumNodes << "\t";
    } else {
        oss << "0"
            << "\t";
    }
    if (whiteID != -1) {
        oss << m_vNodes[whiteID].m_nSumNodes << "\t";
    } else {
        oss << "0"
            << "\t";
    }
    if (emptyID != -1) {
        oss << m_vNodes[emptyID].m_nSumNodes << "\t";
    } else {
        oss << "0"
            << "\t";
    }
    if (noneID != -1) {
        oss << m_vNodes[noneID].m_nSumNodes << "\t";
    } else {
        oss << "0"
            << "\t";
    }
    oss << depth << "\t";
    oss << "\n";

    if (blackID != -1) { getSumNodes(blackID, depth + 1, oss); }
    if (whiteID != -1) { getSumNodes(whiteID, depth + 1, oss); }
    if (emptyID != -1) { getSumNodes(emptyID, depth + 1, oss); }
    if (noneID != -1) { getSumNodes(noneID, depth + 1, oss); }

    return;
}

void RZoneHierarchicalTT::store_(const TTentry& entry, RZoneTtNode* node, int crucialIndex)
{
    if (crucialIndex < m_vCrucialStones.size()) {
        ++node->m_nEntry;
        node->m_maxEntryID = getTTSize();
        store_(entry, getAndCreateChild(node, getNextEdge(entry, crucialIndex)), crucialIndex + 1);
    } else {
        skipLargerPattern(entry);
        ++node->m_nEntry;
        node->m_maxEntryID = getTTSize();
        node->m_vEntry.push_back(getTTSize());
        m_vEntry.push_back(entry);
        m_heatmap.addStone(entry.m_bmRZone);
    }

    // count nodes
    int nSumNodes = 1;
    for (int i = 0; i < COLOR_SIZE; ++i) {
        int nodeID = node->m_children[i];
        if (nodeID != -1) { nSumNodes += m_vNodes[nodeID].m_nSumNodes; }
    }
    node->m_nSumNodes = nSumNodes;

    return;
}

bool RZoneHierarchicalTT::lookup_(const WeichiThreadState& state, TTentry& entry, int id, int crucialIndex, int queryStartID, int& nNum, bool bFoundStop, TTstatistic& stat)
{
    ++nNum;
    if (id == -1) {
        statisticForNoID(stat, crucialIndex);
        return false;
    }

    RZoneTtNode* node = &m_vNodes[id];
    ++node->m_nVisit;
    if (bFoundStop) {
        statisticForFoundStop(stat, crucialIndex);
        return false;
    }

    if (WeichiConfigure::use_rzone_tt_skip_on_id_diff && node->m_maxEntryID < queryStartID) {
        statisticForIdSkip(stat, crucialIndex);
        ++m_nSkipOnTree;
        return false;
    }

    TTstatistic childStat;
    TTstatistic siblingStat;
    bool bFound = false;
    if (crucialIndex < m_vCrucialStones.size()) {
        int crucial_pos = m_vCrucialStones[crucialIndex];
        Color c = state.m_board.getGrid(crucial_pos).getColor();
        int childNodeID = node->m_children[c];
        int siblingNodeID = node->m_children[COLOR_BORDER];
        vector<VisitInfo> vOrder = getSearchOrder(childNodeID, siblingNodeID);
        for (int iOrder = 0; iOrder < vOrder.size(); ++iOrder) {
            const VisitInfo& visit = vOrder[iOrder];
            TTstatistic nextStat;
            int nextNodeID = visit.m_nodeID;
            VisitSide visitSide = visit.m_side;
            bool bFoundInThisWay = lookup_(state, entry, nextNodeID, crucialIndex + 1, queryStartID, nNum, bFound, nextStat);
            if (nextNodeID == -1) {
                setOneNullEdgeBySide(nextStat, visitSide);
            } else if (bFoundInThisWay && iOrder == 0 && vOrder[1].m_nodeID != -1) {
                setOneFoundStopEdgeBySide(nextStat, visitSide);
            } else if (WeichiConfigure::use_rzone_tt_skip_on_id_diff && m_vNodes[nextNodeID].m_maxEntryID < queryStartID) {
                setOneIdSkipEdgeBySide(nextStat, visitSide);
            } else {
                increaseCompareEdgeBySide(nextStat, visitSide);
            }
            if (visit.m_side == VisitSide::LEFT_SIDE) {
                childStat = nextStat;
            } else if (visit.m_side == VisitSide::RIGHT_SIDE) {
                siblingStat = nextStat;
            }
            if (bFoundInThisWay) { bFound = true; }
        }
        TTstatistic mergeStat = TTstatistic::mergeStat(childStat, siblingStat);
        stat.copy(mergeStat);
    } else {
        // leaf
        int nEntryCompare = 0;
        for (int i = node->m_vEntry.size() - 1; i >= 0; --i) {
            int entryID = node->m_vEntry[i];
            if (WeichiConfigure::use_rzone_tt_skip_on_id_diff && entryID < queryStartID) {
                m_nSkipEntryCompare += (i + 1ull);
                break;
            }

            const TTentry& ttEntry = m_vEntry[node->m_vEntry[i]];
            if (WeichiConfigure::use_rzone_tt_skip_larger_pattern && ttEntry.m_bSkip) {
                ++m_nSkipLargeRZPattern;
                continue;
            }

            ++nEntryCompare;
            TTentry queryEntry = TtUtility::createTtEntry(state, ttEntry);
            if (ttEntry.equal(queryEntry)) {
                entry = ttEntry;
                bFound = true;
                break;
            }
        }
        m_nEntryCompare += nEntryCompare;
        stat.m_tree.m_nCompareTotalLength = crucialIndex + 1ull;
        stat.m_tree.m_nComparePath = 1;
        stat.m_tree.m_nMaxComparePathLength = crucialIndex + 1ull;
        stat.m_nEntry = node->m_vEntry.size();
        stat.m_nMaxEntry = node->m_vEntry.size();
        stat.m_nCompare = nEntryCompare;
        stat.m_nMaxCompare = nEntryCompare;
    }

    return bFound;
}

RZoneTtNode* RZoneHierarchicalTT::getAndCreateChild(RZoneTtNode* node, Color c)
{
    if (node->m_children[c] == -1) {
        node->m_children[c] = m_nNode;
        m_vNodes[m_nNode].m_ID = m_nNode;
        ++m_nNode;
    }
    if (m_nNode >= m_vNodes.size()) {
        CERR() << "[WARNING]: m_nNode exceeds m_vNodes.size()" << endl;
        exit(-1);
    }
    return &m_vNodes[node->m_children[c]];
}

vector<RZoneHierarchicalTT::VisitInfo> RZoneHierarchicalTT::getSearchOrder(int childNodeID, int siblingNodeID)
{
    vector<VisitInfo> vOrder;
    VisitInfo leftVisit(childNodeID, VisitSide::LEFT_SIDE);
    VisitInfo rightVisit(siblingNodeID, VisitSide::RIGHT_SIDE);
    if (WeichiConfigure::use_rzone_tt_size_heuristic && childNodeID != -1 && siblingNodeID != -1) {
        int nChildEntry = m_vNodes[childNodeID].m_nEntry;
        int nSiblingEntry = m_vNodes[siblingNodeID].m_nEntry;
        if (nChildEntry < nSiblingEntry) {
            vOrder.push_back(rightVisit);
            vOrder.push_back(leftVisit);
        }
    }
    if (vOrder.size() == 0) {
        if (WeichiConfigure::use_rzone_tt_right_first) {
            vOrder.push_back(rightVisit);
            vOrder.push_back(leftVisit);
        } else {
            vOrder.push_back(leftVisit);
            vOrder.push_back(rightVisit);
        }
    }

    return vOrder;
}

Color RZoneHierarchicalTT::getNextEdge(const TTentry& entry, int crucialIndex) const
{
    int crucial_pos = m_vCrucialStones[crucialIndex];
    if (entry.m_bmBlackPiece.BitIsOn(crucial_pos)) {
        return COLOR_BLACK;
    } else if (entry.m_bmWhitePiece.BitIsOn(crucial_pos)) {
        return COLOR_WHITE;
    } else if (entry.m_bmEmptyPiece.BitIsOn(crucial_pos)) {
        return COLOR_NONE;
    }
    return COLOR_BORDER;
}

void RZoneHierarchicalTT::skipLargerPattern(const TTentry& entry)
{
    if (WeichiConfigure::use_rzone_tt_skip_larger_pattern) {
        for (int i = 0; i < m_vEntry.size(); ++i) {
            if (m_vEntry[i].m_bSkip || !entry.isSubsetOf(m_vEntry[i])) { continue; }

            m_vEntry[i].m_bSkip = true;
            ++m_nLargeRZonePattern; // overlapped R-zone
        }
    }
}

void RZoneHierarchicalTT::addSummaryStatistic(TTstatistic& stat, int startID)
{
    const RZoneTtNode* root = &m_vNodes[0];
    int IDdiff = (startID - root->m_maxEntryID);
    int nBlack = (root->m_children[COLOR_BLACK] != -1) ? m_vNodes[root->m_children[COLOR_BLACK]].m_nSumNodes : 0;
    int nWhite = (root->m_children[COLOR_WHITE] != -1) ? m_vNodes[root->m_children[COLOR_WHITE]].m_nSumNodes : 0;
    int nEmpty = (root->m_children[COLOR_NONE] != -1) ? m_vNodes[root->m_children[COLOR_NONE]].m_nSumNodes : 0;
    int nNone = (root->m_children[COLOR_BORDER] != -1) ? m_vNodes[root->m_children[COLOR_BORDER]].m_nSumNodes : 0;

    m_vSummary.push_back(TTSumary(m_nReconstruct, IDdiff, getTTSize(), stat, nBlack, nWhite, nEmpty, nNone));
}

void RZoneHierarchicalTT::addSummaryStatistic2(TTstatistic& stat, int startID)
{
    const RZoneTtNode* root = &m_vNodes[0];
    int IDdiff = (startID - root->m_maxEntryID);
    int nBlack = (root->m_children[COLOR_BLACK] != -1) ? m_vNodes[root->m_children[COLOR_BLACK]].m_nSumNodes : 0;
    int nWhite = (root->m_children[COLOR_WHITE] != -1) ? m_vNodes[root->m_children[COLOR_WHITE]].m_nSumNodes : 0;
    int nEmpty = (root->m_children[COLOR_NONE] != -1) ? m_vNodes[root->m_children[COLOR_NONE]].m_nSumNodes : 0;
    int nNone = (root->m_children[COLOR_BORDER] != -1) ? m_vNodes[root->m_children[COLOR_BORDER]].m_nSumNodes : 0;

    m_vSummary2.push_back(TTSumary(m_nReconstruct, IDdiff, getTTSize(), stat, nBlack, nWhite, nEmpty, nNone));
}

void RZoneHierarchicalTT::statisticForNoID(TTstatistic& statistic, int crucialIndex)
{
    statistic.m_tree.m_nNullTotalLength = crucialIndex + 1;
    statistic.m_tree.m_nNullPath = 1;
    statistic.m_tree.m_nMaxNullPathLength = crucialIndex + 1;
}

void RZoneHierarchicalTT::statisticForFoundStop(TTstatistic& statistic, int crucialIndex)
{
    statistic.m_tree.m_nFoundStopTotalLength = crucialIndex + 1;
    statistic.m_tree.m_nFoundStopPath = 1;
    statistic.m_tree.m_nMaxFoundStopPathLength = crucialIndex + 1;
}

void RZoneHierarchicalTT::statisticForIdSkip(TTstatistic& statistic, int crucialIndex)
{
    statistic.m_tree.m_nIdSkipTotalLength = crucialIndex + 1;
    statistic.m_tree.m_nIdSkipPath = 1;
    statistic.m_tree.m_nMaxIdSkipPathLength = crucialIndex + 1;
}

void RZoneHierarchicalTT::setOneNullEdgeBySide(TTstatistic& stat, VisitSide side)
{
    if (side == VisitSide::LEFT_SIDE) {
        stat.m_tree.m_nNullLeftEdge = 1;
    } else if (side == VisitSide::RIGHT_SIDE) {
        stat.m_tree.m_nNullRightEdge = 1;
    }
}

void RZoneHierarchicalTT::setOneIdSkipEdgeBySide(TTstatistic& stat, VisitSide side)
{
    if (side == VisitSide::LEFT_SIDE) {
        stat.m_tree.m_nIdSkipLeftEdge = 1;
    } else if (side == VisitSide::RIGHT_SIDE) {
        stat.m_tree.m_nIdSkipRightEdge = 1;
    }
}

void RZoneHierarchicalTT::setOneFoundStopEdgeBySide(TTstatistic& stat, VisitSide side)
{
    if (side == VisitSide::LEFT_SIDE) {
        stat.m_tree.m_nFoundStopLeftEdge = 1;
    } else if (side == VisitSide::RIGHT_SIDE) {
        stat.m_tree.m_nFoundStopRightEdge = 1;
    }
}

void RZoneHierarchicalTT::increaseCompareEdgeBySide(TTstatistic& stat, VisitSide side)
{
    if (side == VisitSide::LEFT_SIDE) {
        stat.m_tree.m_nCompareLeftEdge++;
    } else if (side == VisitSide::RIGHT_SIDE) {
        stat.m_tree.m_nCompareRightEdge++;
    }
}

void RZoneHierarchicalTT::traverseNumMaxBucketSize_(int nodeID, int& numMaxBucketSize) const
{
    if (nodeID == -1) { return; }

    const RZoneTtNode* node = &m_vNodes[nodeID];
    if (node->m_vEntry.size() != 0) {
        numMaxBucketSize = node->m_vEntry.size() > numMaxBucketSize ? node->m_vEntry.size() : numMaxBucketSize;
    }

    for (int i = 0; i < COLOR_SIZE; ++i) {
        traverseNumMaxBucketSize_(node->m_children[i], numMaxBucketSize);
    }

    return;
}

string RZoneHierarchicalTT::getBucketEntryString_(int nodeID, int depth, int& iBucket, WeichiBitBoard bmEmpty, WeichiBitBoard bmBlack, WeichiBitBoard bmWhite) const
{
    if (nodeID == -1) { return ""; }

    const RZoneTtNode* node = &m_vNodes[nodeID];
    ostringstream oss;
    if (depth < m_vCrucialStones.size()) {
        int pos = m_vCrucialStones[depth];
        for (int i = 0; i < COLOR_SIZE; ++i) {
            switch (i) {
                case 0: bmEmpty.SetBitOn(pos); break;
                case 1: bmBlack.SetBitOn(pos); break;
                case 2: bmWhite.SetBitOn(pos); break;
            }
            oss << getBucketEntryString_(node->m_children[i], depth + 1, iBucket, bmEmpty, bmBlack, bmWhite);
            switch (i) {
                case 0: bmEmpty.SetBitOff(pos); break;
                case 1: bmBlack.SetBitOff(pos); break;
                case 2: bmWhite.SetBitOff(pos); break;
            }
        }
    } else {
        ++iBucket;
        oss << "--iBucket: " << iBucket << ", size: " << node->m_vEntry.size() << endl;
        for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
            if (*it % MAX_BOARD_SIZE == 0) { oss << "\t"; }

            if (bmBlack.BitIsOn(*it)) {
                oss << "B";
            } else if (bmWhite.BitIsOn(*it)) {
                oss << "W";
            } else if (bmEmpty.BitIsOn(*it)) {
                oss << " ";
            } else {
                oss << ".";
            }

            if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
        }
        int index = 0;
        for (int i = 0; i < node->m_vEntry.size(); ++i) {
            oss << "ITEM-[" << index << "]" << endl;
            int entryID = node->m_vEntry[i];

            const TTentry& ttEntry = m_vEntry[entryID];
            if (ttEntry.m_bSkip) { oss << "--skip--" << endl; }
            oss << ttEntry.toString();
        }
    }

    return oss.str();
}