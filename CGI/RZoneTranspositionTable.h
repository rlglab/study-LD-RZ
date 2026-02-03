#ifndef H_RZONE_TRANSPOSITION_TABLE
#define H_RZONE_TRANSPOSITION_TABLE

#include "BasicType.h"
#include "FeatureList.h"
#include "NodePtr.h"
#include "StaticBoard.h"
#include "TTSummary.h"
#include "TTentry.h"
#include "TTstatistic.h"
#include "TTutility.h"
#include "Timer.h"
#include "WeichiBoard.h"
#include "WeichiUctNode.h"
#include <map>

class WeichiThreadState;

class RZoneTtNode {
public:
    RZoneTtNode()
    {
        m_ID = -1;
        m_nEntry = 0;
        m_maxEntryID = -1;
        memset(m_children, -1, sizeof(int) * COLOR_SIZE);
        m_nQuery = 0;
        m_nVisit = 0;
        m_nSumNodes = 1;
        m_vEntry.clear();
    }

public:
    int m_ID;
    int m_nEntry;
    int m_maxEntryID;
    int m_children[COLOR_SIZE];
    ull m_nQuery;
    ull m_nVisit;
    ull m_nSumNodes;
    vector<int> m_vEntry;
};

class Heatmap {
    int m_sum;
    int m_crucialSum;
    int m_counts[MAX_NUM_GRIDS];
    WeichiBitBoard m_bmCrucial;

    const float RECONSTRUCT_THRESHOLD = 0.8f;

public:
    inline void reset()
    {
        m_sum = 0;
        m_crucialSum = 0;
        m_bmCrucial.Reset();
        memset(m_counts, 0, sizeof(int) * MAX_NUM_GRIDS);
    }

    inline void addStone(WeichiBitBoard bmStone, int count = 1)
    {
        uint pos;
        while ((pos = bmStone.bitScanForward()) != -1) {
            m_sum += count;
            m_counts[pos] += count;
            if (m_bmCrucial.BitIsOn(pos)) { ++m_crucialSum; }
        }
    }

    inline string toString() const
    {
        ostringstream oss;
        oss << "Heatmap: " << endl;
        oss << "crucial ratio: " << m_crucialSum << "/" << m_sum << "(" << m_crucialSum * 1.0f / m_sum << ")" << endl;
        for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
            oss << setw(6) << getCounts(*it) << " ";
            if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
        }

        return oss.str();
    }

    inline bool needToReconstruct() { return (m_crucialSum * 1.0f / m_sum) < RECONSTRUCT_THRESHOLD; }
    inline bool empty() { return (m_sum == 0); }
    inline void setCounts(int pos, int counts) { m_counts[pos] = counts; }
    inline void setCrucialBitBoard(WeichiBitBoard bmBoard) { m_bmCrucial = bmBoard; }
    inline int getCounts(int pos) const { return m_counts[pos]; }
    inline int getSum() { return m_sum; }
    inline int getCrucialSum() { return m_crucialSum; }
    inline int getHottestPosition() { return distance(m_counts, std::max_element(m_counts, m_counts + MAX_NUM_GRIDS)); }
    inline WeichiBitBoard getCrucialBitBoard() { return m_bmCrucial; }
};

class RZoneHierarchicalTT {
public:
    typedef NodePtr<WeichiUctNode> UctNodePtr;
    enum class VisitSide {
        LEFT_SIDE,
        RIGHT_SIDE
    };

    class VisitInfo {
    public:
        VisitInfo(int id, VisitSide side) : m_nodeID(id), m_side(side) {}
        int m_nodeID;
        VisitSide m_side;
    };

public:
    RZoneHierarchicalTT()
    {
        PHASE1_RECONSTRUCT_SIZE = WeichiConfigure::rzone_tt_phase1_size;
        RECONSTRUCT_SIZE_RATIO = WeichiConfigure::rzone_tt_expansion_threshold;
        reset();
    }

    void reset()
    {
        m_nLookup = 0;
        m_nHit = 0;
        m_nStore = 0;
        m_nReconstructStore = 0;
        m_nEntryCompare = 0;
        m_nHitEntrtyCompare = 0;
        m_nSkipEntryCompare = 0;
        m_nSkipOnTree = 0;
        m_nLargeRZonePattern = 0;
        m_nSkipLargeRZPattern = 0;
        m_nReconstruct = 0;
        m_nPreviousConstructSize = PHASE1_RECONSTRUCT_SIZE;
        m_avgTraverse.reset();
        m_storeTimer.reset();
        m_lookupTimer.reset();
        m_lookupNoTSTimer.reset();
        initializeTreeStructure();
        m_vSummary.clear();
        m_vSummary2.clear();
        m_vReconstruct.clear();
        m_nSkipStore = 0;
    }

    void initializeTreeStructure()
    {
        m_vCrucialStones.clear();
        m_nNode = 1; // root always use index of 0
        m_vNodes.clear();
        m_vNodes.resize(10000000);
        m_vNodes[0].m_ID = 0; // set root ID
        m_vEntry.clear();
        m_heatmap.reset();
    }

    void store(const TTentry& entry);
    void reconstructStore(const TTentry& entry);
    bool lookup(const WeichiThreadState& state, TTentry& entry, UctNodePtr pNode);    
    void reConstruct();
    void debugDisplay();

    inline StopTimer& getStoreTimer() { return m_storeTimer; }
    inline StopTimer& getLookupTimer() { return m_lookupTimer; }
    inline StopTimer& getLookupNoTSTimer() { return m_lookupNoTSTimer; }
    inline vector<int>& getCrucialStones() { return m_vCrucialStones; }
    inline int getTTSize() const { return m_vEntry.size(); }
    inline ull getNumTotalEntryCompare() const { return m_nEntryCompare; }
    inline ull getNumHitEntryCompare() const { return m_nHitEntrtyCompare; }
    inline ull getNumSkipEntryCompare() const { return m_nSkipEntryCompare; }
    inline ull getNumTotalStore() const { return m_nStore; }
    inline ull getNumReconstructStore() const { return m_nReconstructStore; }
    inline ull getNumTotalLookup() const { return m_nLookup; }
    inline ull getNumSkipOnTree() const { return m_nSkipOnTree; }
    inline ull getNumHit() const { return m_nHit; }
    inline ull getNumLargeRZonePattern() const { return m_nLargeRZonePattern; }
    inline ull getNumSkipCompareLargeRZPattern() const { return m_nSkipLargeRZPattern; }
    inline ull getNumRescontruct() const { return m_nReconstruct; }
    inline StatisticData getAvgTraverse() const { return m_avgTraverse; }
    inline string getHeapMapStr() const { return m_heatmap.toString(); }

    ull getNumBuckets(int nodeID = 0) const;
    ull getNumMaxBucketSize() const;
    string getBucketEntryString() const;
    string getTableStructureStr(int nodeID, int& nTotalNode) const;
    string getTableNodeStructureStr(int nodeID, int& totalNodes) const;
    string getTableEntryStructureStr(int nodeID) const;
    string getOutputSummaryStatistic() const;
    string getOutputSummaryStatistic2() const;
    string getSumNodesFromRoot() const;
    string getReconstructInfo() const;

private:
    void store_(const TTentry& entry, RZoneTtNode* node, int crucialIndex);
    bool lookup_(const WeichiThreadState& state, TTentry& entry, int id, int crucialIndex, int queryStartID, int& nNum, bool bFoundStop, TTstatistic& stat);    
    RZoneTtNode* getAndCreateChild(RZoneTtNode* node, Color c);
    vector<VisitInfo> getSearchOrder(int childNodeID, int siblingNodeID);
    Color getNextEdge(const TTentry& entry, int crucialIndex) const;
    void skipLargerPattern(const TTentry& entry);
    inline int getTimeStamp() const { return getTTSize() + m_nSkipStore; }

    // For statistic
    void addSummaryStatistic(TTstatistic& stat, int startID);
    void addSummaryStatistic2(TTstatistic& stat, int startID);
    void statisticForNoID(TTstatistic& stat, int crucialIndex);
    void statisticForFoundStop(TTstatistic& stat, int crucialIndex);
    void statisticForIdSkip(TTstatistic& stat, int crucialIndex);
    void setOneNullEdgeBySide(TTstatistic& stat, VisitSide side);
    void setOneIdSkipEdgeBySide(TTstatistic& stat, VisitSide side);
    void setOneFoundStopEdgeBySide(TTstatistic& stat, VisitSide side);
    void increaseCompareEdgeBySide(TTstatistic& stat, VisitSide side);

    void traverseNumMaxBucketSize_(int nodeID, int& numMaxBucketSize) const;
    string getBucketEntryString_(int nodeID, int depth, int& iBucket, WeichiBitBoard bmEmpty, WeichiBitBoard bmBlack, WeichiBitBoard bmWhite) const;
    void getSumNodes(int nodeID, int depth, ostringstream& oss) const;

    inline RZoneTtNode* getRoot() { return &m_vNodes[0]; }

private:
    const int MAX_LAYERS = 20;
    const float CRUCIAL_RATIO = 0.8f;
    int PHASE1_RECONSTRUCT_SIZE;
    float RECONSTRUCT_SIZE_RATIO;
    int m_nNode;
    int m_nSkipStore;
    Heatmap m_heatmap;
    vector<int> m_vCrucialStones;
    vector<RZoneTtNode> m_vNodes;
    vector<TTentry> m_vEntry;
    StopTimer m_storeTimer;
    StopTimer m_lookupTimer;
    StopTimer m_lookupNoTSTimer;
    const int visitOrder[4] = {COLOR_BLACK, COLOR_WHITE, COLOR_NONE, COLOR_BORDER};

    // statistic data (global)
    ull m_nLookup;
    ull m_nHit;
    ull m_nStore;
    ull m_nReconstructStore;
    ull m_nEntryCompare;
    ull m_nHitEntrtyCompare;
    ull m_nSkipEntryCompare;
    ull m_nSkipOnTree;
    ull m_nLargeRZonePattern;
    ull m_nSkipLargeRZPattern;
    ull m_nReconstruct;
    ull m_nPreviousConstructSize;
    StatisticData m_avgTraverse;

    // more statistic
    vector<TTSumary> m_vSummary;
    vector<TTSumary> m_vSummary2;
    vector<pair<int, bool>> m_vReconstruct;
};

#endif
