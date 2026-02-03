#ifndef WEICHICNNBVVNNET_H
#define WEICHICNNBVVNNET_H

#include "Color.h"
#include "BaseCNNNet.h"
#include "WeichiDynamicKomi.h"

class WeichiCNNBVVNNet: public BaseCNNNet
{
private:
	int m_vn_label_size;

	// caffe output
	float* m_value;
	float* m_terriroty;

public:
	WeichiCNNBVVNNet( CNNNetParam param )
		: BaseCNNNet(param), m_value(nullptr), m_terriroty(nullptr), m_vn_label_size(-1)
	{
	}

	~WeichiCNNBVVNNet()
	{
		if( m_value!=nullptr ) { delete[] m_value; }
		if( m_terriroty!=nullptr ) { delete[] m_terriroty; }
	}

	void loadNetWork( bool bDisplayErrorMessage=false )
	{
		BaseCNNNet::loadNetWork(bDisplayErrorMessage);
#ifdef USE_CAFFE
		if( m_caffe_net->has_blob(WeichiConfigure::VN_OUTPUT_NAME) ) {
			m_vn_label_size = m_caffe_net->blob_by_name(WeichiConfigure::VN_OUTPUT_NAME)->channels();
			m_value = new float[m_batch_size*m_vn_label_size];
		}
		if( m_caffe_net->has_blob(WeichiConfigure::BV_OUTPUT_NAME) ) {
			m_terriroty = new float[m_batch_size*MAX_NUM_GRIDS];
		}
#endif
	}

	void forward()
	{
#ifdef USE_CAFFE
		m_data_input_layer->Reset(m_input,m_dummy,m_batch_size);
		m_caffe_net->Forward();

		// BV result
		if( hasBVOutput() ) {
			float fOutput[MAX_NUM_GRIDS];
			const float* fBVOutput = m_caffe_net->blob_by_name(WeichiConfigure::BV_OUTPUT_NAME)->cpu_data();
			for(int batch=0; batch<m_current_batch_size; batch++) {
				for( uint pos=0; pos<WeichiConfigure::TotalGrids; pos++ ) {
					float fResult = fBVOutput[batch*WeichiConfigure::TotalGrids+pos];
					if( m_color[batch]==COLOR_WHITE ) { fResult = 1 - fResult; }
					fResult = fResult*2 - 1;
					fOutput[pos] = fResult;
				}
				rotateOneFeature(fOutput,m_reverseSymmetric[batch]);
				float* fTerritory = m_terriroty + batch*MAX_NUM_GRIDS;
				for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
					fTerritory[*it] = fOutput[WeichiMove(*it).getCompactPosition()];
				}
			}
		}
		
		// VN result
		if( hasVNOutput() ) {
			int total_size = m_current_batch_size*m_vn_label_size;
			const float* fVNOutput = m_caffe_net->blob_by_name(WeichiConfigure::VN_OUTPUT_NAME)->cpu_data();
			for( int index=0; index<total_size; index++ ) {
				m_value[index] = (fVNOutput[index]+1)/2;
			}
		}

		resetCurrentBatchSize();
#endif
	}

	inline bool hasVNOutput() const { return (m_value!=nullptr); }
	inline bool hasBVOutput() const { return (m_terriroty!=nullptr); }
	inline int getVNLabelSize() { return m_vn_label_size; }
	inline float* getVNResult( int batch ) { return m_value+batch*m_vn_label_size; }
	inline float getVNResult( int batch, double dKomi )
	{
		int index_diff = static_cast<int>(dKomi - 7.5);
		int index = (getVNLabelSize()-1)/2 + index_diff;
		if( index<0 ) { index = 0; }
		else if( index>=getVNLabelSize() ) { index = getVNLabelSize() - 1; }

		return getVNResult(batch)[index];
	}

	Vector<float,MAX_NUM_GRIDS> getCNNTerritory( uint batch )
	{
		Vector<float,MAX_NUM_GRIDS> vTerritory;
		vTerritory.setAllAs(0.0f,MAX_NUM_GRIDS);
		for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
			vTerritory[*it] = getCNNTerritory(batch,*it);
		}
		return vTerritory;
	}
	inline float getCNNTerritory(uint batch, uint position) {  return m_terriroty[batch*MAX_NUM_GRIDS+position]; }
};

#endif