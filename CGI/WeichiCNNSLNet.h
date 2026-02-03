#ifndef WEICHICNNSLNET_H
#define WEICHICNNSLNET_H

#include "BaseCNNNet.h"
#include "WeichiBoard.h"
#include "CandidateEntry.h"
#include "NodePtr.h"
#include "WeichiUctNode.h"

class WeichiCNNSLNet: public BaseCNNNet
{
private:
	// input data
	float* m_temperature;

	// output
	float* m_slOutput;

	// Node Info
	bool* m_bIslegal;

#ifdef USE_CAFFE
	MemoryDataLayerSp m_softmax_temperature_layer;
#endif

public:
	WeichiCNNSLNet( CNNNetParam param )
		: BaseCNNNet(param), m_temperature(nullptr), m_slOutput(nullptr), m_bIslegal(nullptr)
	{
	}

	~WeichiCNNSLNet()
	{
		if( m_temperature!=nullptr ) { delete[] m_temperature; }
		if( m_slOutput!=nullptr ) { delete[] m_slOutput; }
		if( m_bIslegal!=nullptr ) { delete[] m_bIslegal; }
	}

	void loadNetWork( bool bDisplayErrorMessage=false );
	void setTemperature( float dTemperature );
	void push_back( const float* data, Color turnColor, const WeichiBitBoard& bmLegal, SymmetryType type=SYM_NORMAL );
	void forward();

	//get result and node info
	WeichiMove getBestCNNMove( uint batch );
	WeichiMove getSoftMaxMove( uint batch, bool bWithIllegal=false );
	WeichiMove getEpsilonGreedyMove( uint batch );
	float getCNNResult( uint batch, uint position );
	bool isLegal( uint batch, uint position ) {
		if( getRandomNum()==0 ) { return m_bIslegal[batch*MAX_NUM_GRIDS+position]; }
		else { return m_bIslegal[batch*getRandomNum()*MAX_NUM_GRIDS+position]; }
	}
	Color getColor( int batch ) {
		if( getRandomNum()==0 ) { return m_color[batch]; }
		else { return m_color[batch*getRandomNum()]; }
	}
	vector<CandidateEntry> getCandidates(int it,int size=MAX_NUM_GRIDS);

	inline float getTemperature() { return m_temperature[0]; }

private:
	void setBatchData( const float* data, Color turnColor, const WeichiBitBoard& bmLegal, SymmetryType type=SYM_NORMAL );
};

#endif