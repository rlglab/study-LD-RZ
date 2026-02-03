#ifndef RLPLAYOUTGENERATOR_H
#define RLPLAYOUTGENERATOR_H

#include <vector>
#include <boost/thread/barrier.hpp>
#include "RLShareData.h"
#include "BaseCNNPlayoutGenerator.h"

class RLInputData {
public:
	uint m_gameID;
	uint m_moveNumber;
	float m_input[MAX_CNN_CHANNEL_SIZE*MAX_NUM_GRIDS];
	SymmetryType type;

	RLInputData( uint gameID, uint moveNumber )
		: m_gameID(gameID), m_moveNumber(moveNumber) {}
};

class RLGameData {
public:
	Color m_winColor;
	Vector<bool,MAX_GAME_LENGTH> m_vLegal;
	Vector<WeichiMove,MAX_GAME_LENGTH> m_vMoves;
	Vector<float,MAX_GAME_LENGTH> m_vBaselineValues;
	RLGameData() { reset(); }
	void reset()
	{
		m_winColor = COLOR_NONE;
		m_vLegal.clear();
		m_vMoves.clear();
		m_vBaselineValues.clear();
	}
};

class RLPlayoutGenerator: public BaseCNNPlayoutGenerator
{
public:
	StatisticData m_illegal;

private:
	uint m_id;
	uint m_seed;
	uint m_gpuID;
	Color m_ourColor;
	fstream m_fSgfInfo;
	bool m_bIsInitialize;
	RLShareData& m_shareData;
	boost::barrier m_barStart;
	boost::barrier m_barFinish;
	WeichiCNNSLNet m_ourCNNNet;
	WeichiCNNSLNet m_oppCNNNet;
	WeichiCNNBVVNNet m_bvvnNet;
	Dual<StatisticData> m_winRate;
	vector< pair<int,int> > m_vWinIndex;
	vector< pair<int,int> > m_vLoseIndex;
	vector<RLGameData> m_vRLGameData;
	vector<RLInputData> m_vRLInputData;

public:
	RLPlayoutGenerator( int id, uint gpuID, RLShareData& shareData )
		: BaseCNNPlayoutGenerator(*new WeichiThreadState())
		, m_id(id), m_gpuID(gpuID), m_barStart(2), m_barFinish(2), m_shareData(shareData), m_bIsInitialize(false)
		, m_ourCNNNet(CNNNetParam(WeichiConfigure::dcnn_RL_ourNet))
		, m_oppCNNNet(CNNNetParam(WeichiConfigure::dcnn_RL_oppNet))
		, m_bvvnNet(CNNNetParam(WeichiConfigure::dcnn_BV_VNNet))
	{
	}

	void run();
	void initialize();
	StatisticData runOneTurn( Color ourColor );
	void startRun() { m_barStart.wait(); }
	void finishRun() { m_barFinish.wait(); }
	void setTimeSeed( uint seed ) { m_seed = seed; }
	void summarizeData();
	void summarizeThreadData( RLPlayoutGenerator* generator );
	void setOpenning(vector<WeichiMove>& vMoves, int startMove);
	inline StatisticData getWinRate( Color color ) const { return m_winRate.get(color); }
	inline const vector<RLGameData>& getGameData() const { return m_vRLGameData; }
	inline const vector<RLInputData>& getInputData() const { return m_vRLInputData; }
	inline vector< pair<int,int> >& getWinIndex() { return m_vWinIndex; }
	inline vector< pair<int,int> >& getLoseIndex() { return m_vLoseIndex; }
#ifdef USE_CAFFE
	void reloadOurNetWorkByParam( caffe::NetParameter net_param ) { m_ourCNNNet.reloadNetWorkByParam(net_param); }
#endif
	void saveOurNetwork( string sFile ) { m_ourCNNNet.saveNetWork(sFile); }

private:
	void reset();
	void reloadOppNetwork( string sNetwork ) { m_oppCNNNet.reloadNetWork(sNetwork); }
	virtual void prepareOneMoveData( uint gameID );
	virtual void playOneMove( uint gameID, uint net_index );
	virtual bool checkEndGame( uint gameID );
	virtual void forward();
	void saveGameResultToInputData( uint gameID, WeichiPlayoutResult result );
	virtual WeichiCNNSLNet& getNet( Color turnColor );
	inline uint getGameRealID( GameInfo* gameInfo ) const { return gameInfo->getID()+((m_ourColor==COLOR_BLACK)? 0: m_shareData.m_totalGames/2); }
};

#endif