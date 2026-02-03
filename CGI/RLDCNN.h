#ifndef RLDCNN_H
#define RLDCNN_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "BasicType.h"
#include "RLShareData.h"
#include "StatisticData.h"
#include "WeichiCNNRLNet.h"
#include "RLPlayoutGenerator.h"
#include "SgfLoader.h"
#include "FileDirectoryExplorer.h"

class RLOpponent {
public:
	RLOpponent(uint id, string sName) : m_id(id), m_sOpponentName(sName) {
		m_vMeetIteration.clear();
		m_vOpponentWinRate.clear();
		m_winRateData.reset();
		m_last5TurnWinRateData.reset();
	}
	uint m_id;
	string m_sOpponentName;
	StatisticData m_winRateData;
	StatisticData m_last5TurnWinRateData;
	vector<int> m_vMeetIteration;
	vector<double> m_vOpponentWinRate;
};


class RLOpenning{
public:
	vector<WeichiMove> m_vMoves;
	int m_startMove;
};

class RLDCNN {
private:
	RLShareData m_shareData;
	WeichiCNNRLNet* m_rlCNNNet;
	vector<RLOpponent> m_vOppNetworkPool;
	boost::thread_group m_threads;
	Vector<RLPlayoutGenerator*,MAX_NUM_THREADS> m_vRLPlayoutGenerator;
	FileDirectoryExplorer m_trainDirExplorer;

public:
	RLDCNN()
	{
		string sSolover_positive = WeichiConfigure::rldcnn_solver_positive;
		string sSolover_negative = WeichiConfigure::rldcnn_solver_negative;
		m_rlCNNNet = new WeichiCNNRLNet(sSolover_positive,sSolover_negative,CNNNetParam(WeichiConfigure::dcnn_RL_ourNet));
	}

	void start( uint max_iteration );

private:
	bool initialize();
	void runOneIteration( int iteration, bool bWithUpdate );
	bool readOpponentPoolFile();
	uint randomNewOpponent();
	uint uniformRandomNewOpponent();
	uint softmaxWithWinrateRandomNewOpponent();

	void summarizeThreadData( uint oppID, uint iteration );
	void update( bool bWithShuffle=true );
	void snapshotTrainingResult( int iteration );
	RLOpenning getRLOpenning();
};

#endif