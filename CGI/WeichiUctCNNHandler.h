#ifndef WEICHIUCTCNNHANDLER_H
#define WEICHIUCTCNNHANDLER_H

#include "DCNNNetCollection.h"
#include "WeichiThreadState.h"
#include "WeichiRemoteCNNJob.h"
#include "DCNNNetResult.h"

class WeichiUctCNNHandler
{
	typedef int DTIndex;
	typedef NodePtr<WeichiUctNode> UctNodePtr;
public:
	// inner class: CNN data for uct to use
	class CNNData {
	public:
		DTIndex m_id;
		bool m_bIsValid;
		HashKey64 m_key;
		Vector<WeichiMove, MAX_TREE_DEPTH> m_vPaths;

		CNNData() { reset(); }
		CNNData( bool vaild, HashKey64 key, DTIndex id, Vector<WeichiMove, MAX_TREE_DEPTH> vPaths )
			: m_bIsValid(vaild), m_key(key), m_id(id), m_vPaths(vPaths) {}

		void reset()
		{
			m_key = 0;
			m_vPaths.clear();
			m_bIsValid = false;
		}
	};

private:
	SpinLock m_slLock;
	UctNodePtr m_pRoot;
	WeichiThreadState& m_state;
	DCNNNetCollection& m_dcnnNetCollection;
	vector<CNNData> m_vCNNData;
	node_manage::NodeAllocator<WeichiUctNode>& m_nodeAllocator;

public:
	WeichiUctCNNHandler( WeichiThreadState& state, node_manage::NodeAllocator<WeichiUctNode>& na )
		: m_state(state), m_dcnnNetCollection(state.m_dcnnNetCollection), m_nodeAllocator(na)
	{
	}

	void doUctCNNJob();
	void putBackAllToQueue();

	// for worker
	std::string runRemoteJob( WeichiCNNNetType type, const vector<CNNJob>& vJobs );

	// for remote update thread
	void updateAll( const WeichiRemoteCNNJob& currentJob );

	inline void setRoot(UctNodePtr pRoot) { m_pRoot = pRoot; }
	inline vector<CNNData>& getCNNData() { return m_vCNNData; }
	inline const vector<CNNData>& getCNNData() const { return m_vCNNData; }

private:
	int collectJobFromQueue();
	int getQueueCandidates(int nSpaceLeft);
	UctNodePtr selectNodeByPaths( Vector<WeichiMove,MAX_TREE_DEPTH> vMoves, bool bWithPlay, bool bWithToken, UctNodePtr& pParent );
	UctNodePtr selectNodeByPaths( Vector<WeichiMove,MAX_TREE_DEPTH> vMoves, bool bWithPlay, bool bWithToken, vector<UctNodePtr>& vNodePaths );
	UctNodePtr selectChildByMove( UctNodePtr pNode, WeichiMove move );

	void forwardAndUpdate();
	void updateAll();
	void putBackToQueue();
	void updateNet(const WeichiRemoteCNNJob& currentJob);
	bool updateNet(int batch, CNNData& cnnData, WeichiCNNNet* cnnNet);
	bool updateNet(CNNData& cnnData, const DCNNNetResult& netResult);

	bool updatePolicy(const DCNNNetResult& netResult, UctNodePtr pNode);
	void copyPolicyNode(UctNodePtr oldNode, UctNodePtr newNode);
	bool updateValue(const DCNNNetResult& netResult, UctNodePtr pNode, vector<UctNodePtr>& vNodePaths);

	void removePathVirtualLoss(vector<UctNodePtr>& vNodePaths);

	void ignoreMoves(const vector<CandidateEntry>& vUctCandidateList);

	inline void getUpdateToken( UctNodePtr pNode ) { pNode->getUpdateToken(); }
	inline void releaseUpdateToken( UctNodePtr pNode ) { if( pNode.isValid() ) { pNode->releaseUpdateToken(); } }
};

#endif