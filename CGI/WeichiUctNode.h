#ifndef WEICHIUCTNODE_H
#define WEICHIUCTNODE_H

#include "Dual.h"
#include "UctNodeImpl.h"
#include "Vector.h"
#include "WeichiMove.h"
#include "WeichiUctNodeStatus.h"
#include "boost/atomic.hpp"

class WeichiUctNode : public UctNodeImpl<WeichiMove, WeichiUctNode> {
    typedef int DTIndex;
    typedef NodePtr<WeichiUctNode> UctNodePtr;

private:
    boost::atomic<WeichiUctNodeStatus> m_status;
    boost::atomic<short> m_winByKoPosition;
    boost::atomic<short> m_equalLoss;
    boost::atomic<bool> m_expandToken;
    boost::atomic<bool> m_updateToken;
    boost::atomic<bool> m_bIsCNNNode;
    boost::atomic<bool> m_bIsInCNNQueue;
    boost::atomic<bool> m_bCheckImmediateWin;
    boost::atomic<bool> m_bSkipTTByLoop;
    boost::atomic<int> m_solDepth;
    boost::atomic<int> m_virtual_loss;
    boost::atomic<int> m_matchedTtNodeID;
    boost::atomic<int> m_RZoneTTQueryEntryStartID;
    boost::atomic<int> m_proofRZoneID;
    boost::atomic<int> m_disproofRZoneID;
    boost::atomic<int> m_potentialRZoneID;
    boost::atomic<int> m_sskID;
    boost::atomic<float> m_fCNNVNValue;
    boost::atomic<float> m_fAdjustChildVNValue;
    StatisticData::data_type m_dProb;
    StatisticData m_rave;
    StatisticData m_vnValue;
    pair<int, int> m_sameRZpatternIndex;
    vector<UctNodePtr> m_vLoopCheck;

public:
    WeichiUctNode()
    {
        m_status = UCT_STATUS_UNKNOWN;
        m_winByKoPosition = -1;
        m_equalLoss = -1;
        m_expandToken = false;
        m_updateToken = false;
        m_bIsCNNNode = false;
        m_bIsInCNNQueue = false;
        m_bCheckImmediateWin = false;
        m_bSkipTTByLoop = false;
        m_solDepth.store(-1);
        m_virtual_loss = false;
        m_matchedTtNodeID.store(-1);
        m_RZoneTTQueryEntryStartID.store(0);
        m_proofRZoneID.store(-1);
        m_disproofRZoneID.store(-1);
        m_potentialRZoneID.store(-1);
        m_sskID.store(-1);
        m_fCNNVNValue = 0.0f;
        m_fAdjustChildVNValue = 0.0f;
        m_firstChild = UctNodePtr::NULL_PTR;
        m_nChildren = 0;
        m_sameRZpatternIndex.first = m_sameRZpatternIndex.second = -1;
        m_vLoopCheck.clear();
    }

    WeichiUctNode& operator=(WeichiUctNode& rhs)
    {
        UctNodeImpl<WeichiMove, WeichiUctNode>::operator=(rhs);

        m_status = rhs.getUctNodeStatus();
        m_winByKoPosition = rhs.getWinByKoPosition();
        m_equalLoss = rhs.getEqualLoss();
        m_expandToken = false;
        m_updateToken = false;
        m_bIsCNNNode = rhs.isCNNNode();
        m_bIsInCNNQueue = rhs.isInCNNQueue();
        m_bCheckImmediateWin = rhs.getCheckImmedaiteWin();
        m_bSkipTTByLoop = rhs.getSkipTTByLoop();
        m_solDepth = rhs.getSolDepth();
        m_virtual_loss = rhs.getVirtualLossCount();
        m_matchedTtNodeID = rhs.getTTmatchedNodeID();
        m_RZoneTTQueryEntryStartID = rhs.getRZoneTTQueryEntryStartID();
        m_proofRZoneID = rhs.getProofRZoneID();
        m_disproofRZoneID = rhs.getDisproofRZoneID();
        m_potentialRZoneID = rhs.getPotentialRZoneID();
        m_sskID = rhs.getSSKID();
        m_fCNNVNValue = rhs.getCNNVNValue();
        m_fAdjustChildVNValue = rhs.getAdjustChildVNValue();
        m_dProb = rhs.m_dProb;
        m_rave = rhs.m_rave;
        m_vnValue = rhs.m_vnValue;
        m_sameRZpatternIndex = rhs.getSameRZpatternIndex();
        m_vLoopCheck = rhs.m_vLoopCheck;

        return *this;
    }

    void reset(WeichiMove move)
    {
        setMove(move);
        m_uctStat.reset();
        clearChildren();

        m_status = UCT_STATUS_UNKNOWN;
        m_winByKoPosition = -1;
        m_equalLoss = -1;
        m_expandToken = false;
        m_updateToken = false;
        m_bIsCNNNode = false;
        m_bIsInCNNQueue = false;
        m_bCheckImmediateWin = false;
        m_bSkipTTByLoop = false;
        m_solDepth = -1;
        m_virtual_loss = 0;
        m_matchedTtNodeID.store(-1);
        m_RZoneTTQueryEntryStartID.store(0);
        m_proofRZoneID.store(-1);
        m_disproofRZoneID.store(-1);
        m_potentialRZoneID.store(-1);
        m_sskID.store(-1);
        m_fCNNVNValue = 0.0f;
        m_fAdjustChildVNValue = 0.0f;
        m_dProb = 0;
        m_rave.reset();
        m_vnValue.reset();
        m_sameRZpatternIndex.first = m_sameRZpatternIndex.second = -1;
        m_vLoopCheck.clear();
    }

    inline void setUctNodeStatus(WeichiUctNodeStatus status) { m_status = status; }
    inline WeichiUctNodeStatus getUctNodeStatus() const { return m_status; }
    inline bool isProved() const { return m_status != UCT_STATUS_UNKNOWN; }
    inline bool isUnknownStatus() const { return m_status == UCT_STATUS_UNKNOWN; }
    inline bool isRZonePruned() const { return m_status == UCT_STATUS_RZONE_PRUNED; }
    inline bool isWinLossBySearch() const { return (m_status == UCT_STATUS_WIN || m_status == UCT_STATUS_LOSS); }
    inline void setWinByKoPosition(short pos) { m_winByKoPosition = pos; }
    inline short getWinByKoPosition() const { return m_winByKoPosition; }
    inline void setEqualLoss(short pos) { m_equalLoss = pos; }
    inline short getEqualLoss() const { return m_equalLoss; }
    inline bool getExpandToken()
    {
        bool from = false;
        return m_expandToken.compare_exchange_strong(from, true);
    }
    inline void releaseExpandToken() { m_expandToken.store(false); }
    inline void getUpdateToken()
    {
        while (m_updateToken.exchange(true)) { ; }
    }
    inline void releaseUpdateToken() { m_updateToken.store(false); }
    inline void setCNNNode() { m_bIsCNNNode = true; }
    inline void removeCNNNode() { m_bIsCNNNode = false; }
    inline bool isCNNNode() const { return m_bIsCNNNode; }
    inline void setInCNNQueue() { m_bIsInCNNQueue = true; }
    inline void removeInCNNQueue() { m_bIsInCNNQueue = false; }
    inline bool isInCNNQueue() const { return m_bIsInCNNQueue; }
    inline void setCheckImmediateWin(bool bWin) { m_bCheckImmediateWin = bWin; }
    inline bool getCheckImmedaiteWin() const { return m_bCheckImmediateWin; }
    inline void setSkipTTByLoop(bool b) { m_bSkipTTByLoop = b; }
    inline bool getSkipTTByLoop() const { return m_bSkipTTByLoop; }
    inline void setSolDepth(int depth) { m_solDepth = depth; }
    inline int getSolDepth() const { return m_solDepth; }
    inline int getVirtualLossCount() const { return m_virtual_loss; }
    inline void addVirtualLoss(int count)
    {
        m_virtual_loss.fetch_add(count);
        m_uctStat.add(-1, count);
    }
    inline void removeVirtualLoss(int count)
    {
        m_virtual_loss.fetch_sub(count);
        m_uctStat.remove(-1, count);
    }
    inline StatisticData getUctDataWithoutVirtualLoss() const
    {
        StatisticData uctWithoutVirtualLoss = getUctData();
        uctWithoutVirtualLoss.remove(-1, m_virtual_loss);
        return uctWithoutVirtualLoss;
    }
    inline void setTTmatchedNodeID(int id) { m_matchedTtNodeID = id; }
    inline int getTTmatchedNodeID() { return m_matchedTtNodeID; }
    inline void setRZoneTTQueryEntryStartID(int id) { m_RZoneTTQueryEntryStartID = id; }
    inline int getRZoneTTQueryEntryStartID() { return m_RZoneTTQueryEntryStartID; }
    inline void setCNNVNValue(float fVNValue) { m_fCNNVNValue = fVNValue; }
    inline void setAdjustChildVNValue(float fAdjustChildVNValue) { m_fAdjustChildVNValue = fAdjustChildVNValue; }
    inline float getCNNVNValue() const { return m_fCNNVNValue; }
    inline float getAdjustChildVNValue() const { return m_fAdjustChildVNValue; }
    inline void setSameRZpatternIndex(pair<int, int> p) { m_sameRZpatternIndex = p; }
    inline pair<int, int> getSameRZpatternIndex() { return m_sameRZpatternIndex; }
    inline vector<UctNodePtr>& getLoopCheck() { return m_vLoopCheck; }
    inline const vector<UctNodePtr>& getLoopCheck() const { return m_vLoopCheck; }

    inline Color getToPlay() const { return AgainstColor(m_move.getColor()); }
    inline StatisticData::data_type getProb() const { return m_dProb; }
    inline void setProb(StatisticData::data_type prob) { m_dProb = prob; }
    inline StatisticData& getRaveData() { return m_rave; }
    inline const StatisticData& getRaveData() const { return m_rave; }
    inline StatisticData& getVNValueData() { return m_vnValue; }
    inline const StatisticData& getVNValueData() const { return m_vnValue; }
    inline bool hasVNValue() const { return m_vnValue.getCount() > 0; }

    inline Color getColor() const { return m_move.getColor(); }
    inline uint getPosition() const { return m_move.getPosition(); }
    inline string toString() const { return m_move.toSgfString(true); }

    inline void setProofRZoneID(int id) { m_proofRZoneID = id; }
    inline int getProofRZoneID() const { return m_proofRZoneID; }
    inline bool hasProofRZone() const { return m_proofRZoneID != -1; }
    inline void setDisproofRZoneID(int id) { m_disproofRZoneID = id; }
    inline int getDisproofRZoneID() const { return m_disproofRZoneID; }
    inline bool hasDisproofRZone() const { return m_disproofRZoneID != -1; }
    inline void setPotentialRZoneID(int id) { m_potentialRZoneID = id; }
    inline int getPotentialRZoneID() const { return m_potentialRZoneID; }
    inline bool hasPotentialRZone() const { return m_potentialRZoneID != -1; }
    inline void setSSKID(int id) { m_sskID = id; }
    inline int getSSKID() const { return m_sskID; }
    inline bool hasSSKID() const { return m_sskID != -1; }

    void clearChildren() // clear children
    {
        UctNodeImpl<WeichiMove, WeichiUctNode>::clearChildren();
    }
};

#endif 
