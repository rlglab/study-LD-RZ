#include "WeichiCNNSLNet.h"

void WeichiCNNSLNet::loadNetWork( bool bDisplayErrorMessage/*=false*/ )
{
	BaseCNNNet::loadNetWork(bDisplayErrorMessage);

	int total_data_size = m_batch_size*WeichiConfigure::TotalGrids;
	m_temperature = new float[total_data_size];
	setTemperature(1/WeichiConfigure::DCNNSLSoftMaxTemperature);

	m_slOutput = new float[m_batch_size*MAX_NUM_GRIDS];
	m_bIslegal = new bool[m_batch_size*MAX_NUM_GRIDS];

#ifdef USE_CAFFE
	// load data input layer
	m_softmax_temperature_layer = boost::dynamic_pointer_cast<caffe::MemoryDataLayer<float> >(m_caffe_net->layer_by_name("softmax_temperature_layer"));
	assert(m_data_input_layer);
#endif
}

void WeichiCNNSLNet::setTemperature( float dTemperature )
{
	int total_data_size = m_batch_size*WeichiConfigure::TotalGrids;
	for( int i=0; i<total_data_size; i++ ) { m_temperature[i] = dTemperature; }
}

void WeichiCNNSLNet::push_back(const float* data, Color turnColor, const WeichiBitBoard& bmLegal, SymmetryType type/*=SYM_NORMAL*/ )
{
	if( getRandomNum()==0 ) {
		setBatchData(data,turnColor,bmLegal,type);
	} else {
		for(int i=0; i<getRandomNum(); i++ ) {
			int index = Random::nextInt(SYMMETRY_SIZE-i);
			setBatchData(data,turnColor,bmLegal,m_vSymmetryEntry[index]);
			m_vSymmetryEntry.swap(index, SYMMETRY_SIZE-1-i);
		}
	}
}

void WeichiCNNSLNet::forward()
{
#ifdef USE_CAFFE
	m_data_input_layer->Reset(m_input,m_dummy,m_batch_size);
	m_softmax_temperature_layer->Reset(m_temperature,m_dummy,m_batch_size);
	m_caffe_net->Forward();
	const float* outputBlob = m_caffe_net->blob_by_name(WeichiConfigure::SL_OUTPUT_NAME)->cpu_data();

	float fOutput[MAX_NUM_GRIDS];
	for(int batch=0; batch<m_current_batch_size; batch++) {
		for( uint pos=0; pos<WeichiConfigure::TotalGrids; pos++ ) {
			fOutput[pos] = outputBlob[batch*WeichiConfigure::TotalGrids+pos];
		}
		rotateOneFeature(fOutput,m_reverseSymmetric[batch]);
		float* slOutput = m_slOutput + batch*MAX_NUM_GRIDS;
		for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
			WeichiMove move(*it);
			slOutput[*it] = fOutput[move.getCompactPosition()];
		}
	}

	resetCurrentBatchSize();
#endif
}

WeichiMove WeichiCNNSLNet::getBestCNNMove( uint batch )
{
	int bestPos = PASS_MOVE.getPosition();
	double dMaxScore = 0;
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		if( !isLegal(batch,*it) ) { continue; }

		double dScore = getCNNResult(batch,*it);
		if( dScore>dMaxScore ) {
			bestPos = *it;
			dMaxScore = dScore;
		}
	}

	return WeichiMove(m_color[batch],bestPos);
}

WeichiMove WeichiCNNSLNet::getSoftMaxMove( uint batch, bool bWithIllegal/*=false*/ )
{
	/*
		Note:
		As there are lots of grid avoided by
			1. illegal positions (by GO rule)
			2. CGI knowledge (by CGI rule)
		We sum these probability as PASS move when "bCNNPass" is true; otherwise we will skip these move.
	*/
	double dSum = 0.0f;
	int position = PASS_MOVE.getPosition();
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		if( !bWithIllegal && !isLegal(batch,*it) ) { continue; }

		double dProb = getCNNResult(batch,*it) * 100;
		dSum += dProb;
		double dRand = Random::nextReal(dSum);
		if( dRand<dProb ) { position = *it; }
	}

	return WeichiMove(m_color[batch],position);
}

WeichiMove WeichiCNNSLNet::getEpsilonGreedyMove( uint batch )
{
	return PASS_MOVE;
}

vector<CandidateEntry> WeichiCNNSLNet::getCandidates( int batch, int size )
{
	float max_score=0;
	vector<CandidateEntry> vCandidates;

	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		float fScore = getCNNResult(batch,*it);
		if( !isLegal(batch,*it) ) { continue; }
		if( fScore>max_score ) { max_score = fScore; }
		vCandidates.push_back(CandidateEntry(*it,fScore));
	}
	vCandidates.push_back(CandidateEntry(PASS_MOVE.getPosition(),max_score+1));
	std::sort(vCandidates.begin(),vCandidates.end());
	
	if( size<vCandidates.size() ) { vCandidates.resize(size); }
	return vCandidates;
}

void WeichiCNNSLNet::setBatchData( const float* data, Color turnColor, const WeichiBitBoard& bmLegal, SymmetryType type/*=SYM_NORMAL*/ )
{
	int start_index = m_current_batch_size*MAX_NUM_GRIDS;
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		if( bmLegal.BitIsOn(*it) ) { m_bIslegal[start_index+*it] = true; }
		else { m_bIslegal[start_index+*it] = false; }
	}
	BaseCNNNet::setBatchData(data,turnColor,type);
}

float WeichiCNNSLNet::getCNNResult( uint batch, uint position )
{
	if (getRandomNum() == 0) { return m_slOutput[batch*MAX_NUM_GRIDS+position]; }

	float dCnnScore=0;
	for(int i=0;i<getRandomNum();++i){
		int index = (batch * getRandomNum()) + i;
		dCnnScore += m_slOutput[index*MAX_NUM_GRIDS+position];
	}
	return dCnnScore / getRandomNum();
}