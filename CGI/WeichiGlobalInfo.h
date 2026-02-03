#ifndef WEICHIRUNINFO_H
#define WEICHIRUNINFO_H

#include "BasicType.h"
#include "NodePtr.h"
#include "WeichiUctNode.h"
#include "WeichiMove.h"
#include "Dual.h"
#include "StatisticData.h"
#include "Territory.h"
#include "WeichiLGRTable.h"
#include "WeichiMoveCache.h"
#include "WeichiCacheLogger.h"
#include "UctNodeOrder.h"
#include "CNNJobQueue.h"
#include "WeichiRemoteCNNJobTable.h"
#include "WeichiRZoneDataTable.h"
#include "BoardTranspositionTable.h"
#include "RZoneTranspositionTable.h"
#include "RZoneLinearTT.h"
#include "WeichiEndGameCondition.h"

class WeichiGlobalInfo
{
	typedef NodePtr<WeichiUctNode> UctNodePtr;

private:
	struct TreeInfo
	{
		static const int OUT_OF_ORDER = -1;
		bool m_bIsEarlyAbort;		
		int m_nMove;
		int m_nDCNNJob;
		int m_nMaxTreeWidth;
		int m_nMaxTreeHeight;
		int m_nMaxChildIndex;
		int m_nHandMove;
		int m_nSimulation;		
		int m_predictOrder;		
		int m_nLeafBenson;
		int m_nTerminal;
		int m_nTTmatch;
		int m_nKos;
		int m_nLongKos;
		int m_nRZoneFailed;
		float m_dThinkTime;
		Dual<float> m_fTerritory;
		Dual<float> m_dRemainTime;
		UctNodePtr m_pRoot;
		UctNodePtr m_pBestNode;
		WeichiMove m_bestMove;
		StatisticData m_bestSD;
		StatisticData m_bestValueSD;
		StatisticData m_lastSD;
		StatisticData m_predictSD;
		StatisticData m_treeWidth;
		StatisticData m_treeHeight;
		Vector<uint,MAX_NUM_GRIDS> m_vPredictOrder;
		Vector<StatisticData,MAX_NUM_GRIDS> m_vPredictInfoOrder;
		Vector<int, MAX_NUM_GRIDS> m_childIndex;
		Color m_winColor;
		string m_problemSgfFilename;
		string m_problemSgfUctTreeFilename;

		TreeInfo()
			: m_predictOrder(OUT_OF_ORDER)
			, m_nMove(0)
		{}

		string getNodeString( const UctNodeOrder& nodeOrder, int deep );
		vector<UctNodeOrder> getChildOrder( UctNodePtr pNode );
	};

	struct SearchInfo
	{
		bool m_outOfOpeningBook;
		bool m_calTerritoryStat;
		Territory m_territory;
		Territory m_passTerritory;
		WeichiMoveCache m_moveCache;
		Dual<uint> m_policyStat[POLICY_SIZE];

		void clearPolicyStat();
	};

	struct LogInfo
	{
		bool m_bLogDirUsed;
		string m_sLogfilePrefix;
		WeichiCacheLogger m_cacheLogger;
	};

	struct RemoteInfo
	{
		WeichiRemoteCNNJobTable m_cnnJobTable;
		boost::atomic<uint> m_jobSessionId;
	};

private:
	LogInfo m_logInfo;
	TreeInfo m_treeInfo;
	SearchInfo m_searchInfo;
	RemoteInfo m_remoteInfo;
	CNNJobQueue m_CNNJobQueue;
	WeichiRZoneDataTable m_RZoneDataTable;	
	BoardTranspositionTable m_boardTT;	
	RZoneHierarchicalTT m_hierTT;						  
	RZoneLinearTT m_LinearTT;
	map<int, vector<pair<int, short>>> m_moveChanged;	
	vector<tuple<int,double,short>> m_vRootSimInfo;
	map<int, string> m_mEarlylifeInfo;
	SpinLock m_lock;
	boost::atomic<int> m_doCNNForward;
	StopTimer m_timer;
	WeichiEndGameCondition m_endGameCondition;
	WeichiBitBoard m_bmZone;

	static WeichiGlobalInfo *s_instance;
public:
	bool m_bIsLastCandidate;

public:
	WeichiGlobalInfo() {}
	~WeichiGlobalInfo() { if( s_instance ) { delete s_instance; } }

	static WeichiGlobalInfo *get()
	{
		if( s_instance==NULL ) { initialize(); }
		return s_instance;
	}
	static TreeInfo& getTreeInfo() { return get()->m_treeInfo; }
	static SearchInfo& getSearchInfo() { return get()->m_searchInfo; }
	static LogInfo& getLogInfo() { return get()->m_logInfo; }
	static RemoteInfo& getRemoteInfo() { return get()->m_remoteInfo; }
	static CNNJobQueue& getCNNJobQueue() { return get()->m_CNNJobQueue; }
	static WeichiRZoneDataTable& getRZoneDataTable() { return get()->m_RZoneDataTable; }		
	static BoardTranspositionTable& getBoardTT() { return get()->m_boardTT; }
	static RZoneHierarchicalTT& getHierTT() { return get()->m_hierTT; }	
	static RZoneLinearTT& getLinearTT() { return get()->m_LinearTT; }
	static vector<tuple<int,double,short>>& getRootSimInfo() { return get()->m_vRootSimInfo; }
	static map<int, string>& getEarlyLifeInfo() { return get()->m_mEarlylifeInfo; }
	static SpinLock& getGlobalLock() { return get()->m_lock; }
	static boost::atomic<int>& getDoCNNForward() { return get()->m_doCNNForward;  }
	static StopTimer& getGlobalTimer() { return get()->m_timer; }
	static WeichiEndGameCondition& getEndGameCondition() { return get()->m_endGameCondition; }

	static WeichiBitBoard& getZone() { return get()->m_bmZone; }
	static WeichiBitBoard& resetZone() { get()->m_bmZone.Reset(); return getZone(); }
	static WeichiBitBoard& setZone(WeichiBitBoard bmZone) { get()->m_bmZone = bmZone; return getZone(); }
	static WeichiBitBoard& addZonePoint(int pos) { get()->m_bmZone.SetBitOn(pos); return getZone(); }
	static WeichiBitBoard& subZonePoint(int pos) { get()->m_bmZone.SetBitOff(pos);; return getZone(); }
	static WeichiBitBoard& addZone(WeichiBitBoard bm) { get()->m_bmZone|=bm; return getZone(); }
	static WeichiBitBoard& subZone(WeichiBitBoard bm) { get()->m_bmZone-=bm; return getZone(); }

	void cleanSummaryInfo();
	void logInfo( const WeichiMove& move );
	void logPredictData();
	string getSearchSummaryInfoString( const WeichiBoard& board );
	vector<string> getSearchTreeSummaryString( const WeichiBoard& board );
	string getPolicyStatString( vector<WeichiPlayoutPolicy> vPrintPolicy );
	string getBestSequenceString()
	{
		Vector<uint,MAX_NUM_GRIDS> vSequenceNumber;
		vector<string> vBestSequence = getBestSequenceString(vSequenceNumber);
		string sBestSequence = "";
		for( uint i=0; i<vBestSequence.size(); i++ ) {
			sBestSequence += vBestSequence[i] + "\n";
		}

		return sBestSequence;
	}
	void calculateSureTerritory();
	
private:
	static void initialize() { s_instance = new WeichiGlobalInfo(); }

	vector<string> getBoardString( const WeichiBoard& board );
	vector<string> getBoardStringWithTerritory( const WeichiBoard& board );
	vector<string> getBoardStringWithSequence( const WeichiBoard& board, const Vector<uint,MAX_NUM_GRIDS>& vSequenceNumber );
	vector<string> getBoardStringWithSLCandidates( const WeichiBoard& board, const Vector<uint,MAX_NUM_GRIDS>& vSLCandidates );
	vector<string> getBestSequenceString( Vector<uint,MAX_NUM_GRIDS>& vSequenceNumber );
	Vector<uint,MAX_NUM_GRIDS> getRootSLCandidates();
	uint predictOpponentMoveOrder( const WeichiMove& move );
	StatisticData predictOppoentMoveInfo( const WeichiMove& move );
	void traverseBestSequence( vector<string>& vBestSequence, UctNodePtr root, int deep, Vector<uint,MAX_NUM_GRIDS>& vSequenceNumber, bool bIsMainSequence=false );
};

#endif 
