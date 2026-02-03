#ifndef H_RZONE_LINEAR_TT
#define H_RZONE_LINEAR_TT

#include "TTentry.h"
#include "TTutility.h"
#include "Timer.h"
#include "WeichiUctNode.h"

class WeichiThreadState;

class RZoneLinearTT {
    typedef NodePtr<WeichiUctNode> UctNodePtr;

public:
    RZoneLinearTT() { reset(); }
    void reset()
    {
        m_nLookup = 0;
        m_nSkipEntryCompare = 0;
        m_nStore = 0;
        m_nSkipLargeRZPattern = 0;
        m_nHit = 0;
        m_nTotalEntryCompare = 0;
        m_nTotalHitCompare = 0;
        m_entry.clear();
        m_storeTimer.reset();
        m_lookupTimer.reset();
    }

    void store(const TTentry& entry)
    {
        ++m_nStore;
        m_storeTimer.start();
        m_entry.push_back(entry);
        m_storeTimer.stopAndAddAccumulatedTime();
        if (WeichiConfigure::use_rzone_tt_skip_larger_pattern) {
            for (int i = 0; i < m_entry.size(); --i) {
                if (m_entry[i].m_bSkip && !entry.isSubsetOf(m_entry[i])) { break; }

                m_entry[i].m_bSkip = true;
            }
        }
    }

    bool lookup(const WeichiThreadState& state, TTentry& entry, UctNodePtr pNode)
    {
        ++m_nLookup;
        m_lookupTimer.start();
        bool bResult = lookup_(state, entry, pNode);
        m_lookupTimer.stopAndAddAccumulatedTime();
        pNode->setRZoneTTQueryEntryStartID(m_entry.size());
        if (bResult) { ++m_nHit; }
        return bResult;
    }

    bool lookup_(const WeichiThreadState& state, TTentry& entry, UctNodePtr pNode)
    {
        int startQueryID = WeichiConfigure::use_rzone_tt_skip_on_id_diff ? pNode->getRZoneTTQueryEntryStartID() : 0;
        m_nSkipEntryCompare += startQueryID;
        ull nCompare = 0;
        bool bHit = false;
        for (int i = m_entry.size() - 1; i >= startQueryID; --i) {
            ++nCompare;
            const TTentry& ttEntry = m_entry[i];
            if (WeichiConfigure::use_rzone_tt_skip_larger_pattern && ttEntry.m_bSkip) {
                ++m_nSkipLargeRZPattern;
                continue;
            }
            TTentry queryEntry = TtUtility::createTtEntry(state, ttEntry);
            if (ttEntry.equal(queryEntry)) {
                m_nTotalHitCompare += nCompare;
                entry = ttEntry;
                bHit = true;
                break;
            }
        }
        m_nTotalEntryCompare += nCompare;

        return bHit;
    }

    string getStatistic() const
    {
        ostringstream oss;
        oss << getSize() << "\t"
            << getNumStore() << "\t"
            << getNumLookup() << "\t"
            << getNumSkipCompare() << "\t"
            << getNumHit() << "\t"
            << getNumTotalEntryCompare() << "\t"
            << getNumTotalHitCompare();
        return oss.str();
    }

public:
    inline ull getSize() const { return m_entry.size(); }
    inline ull getNumStore() const { return m_nStore; }
    inline ull getNumLookup() const { return m_nLookup; }
    inline ull getNumSkipCompare() const { return m_nSkipEntryCompare; }
    inline ull getNumHit() const { return m_nHit; }
    inline ull getNumTotalEntryCompare() const { return m_nTotalEntryCompare; }
    inline ull getNumTotalHitCompare() const { return m_nTotalHitCompare; }
    inline StopTimer& getStoreTimer() { return m_storeTimer; }
    inline StopTimer& getLookupTimer() { return m_lookupTimer; }

private:
    ull m_nLookup;
    ull m_nSkipEntryCompare;
    ull m_nStore;
    ull m_nSkipLargeRZPattern;
    ull m_nHit;
    ull m_nTotalEntryCompare;
    ull m_nTotalHitCompare;
    vector<TTentry> m_entry;
    StopTimer m_storeTimer;
    StopTimer m_lookupTimer;
};

#endif
