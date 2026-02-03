#ifndef WEICHIRLCNNNET_H
#define WEICHIRLCNNNET_H

#include "BaseCNNNet.h"

#ifdef USE_CAFFE
#include <caffe/layers/memory_data_layer.hpp>
#endif

class WeichiCNNRLNet: public BaseCNNNet
{
private:
	string m_sSolver_positive;
	string m_sSolver_negative;

	float* m_label;
	float* m_weight;

#ifdef USE_CAFFE
	MemoryDataLayerSp m_data_input_layer;
	caffe::SolverParameter m_solver_param_positive;
	caffe::SolverParameter m_solver_param_negative;
	boost::shared_ptr< caffe::Solver<float> > m_solver;
	boost::shared_ptr< caffe::P2PSync<float> > m_sync;
	vector< boost::shared_ptr< caffe::P2PSync<float> > > m_vSync;
	MemoryDataLayerSp m_weight_input_layer;
#endif

public:
	WeichiCNNRLNet( string sSolver_positive, string sSolver_negative, CNNNetParam param )
		: BaseCNNNet(param), m_label(nullptr), m_weight(nullptr)
	{
		m_sSolver_positive = WeichiConfigure::dcnn_dir + sSolver_positive;
		m_sSolver_negative = WeichiConfigure::dcnn_dir + sSolver_negative;
		m_input_data_size = m_channel_size * WeichiConfigure::TotalGrids;
	}

	~WeichiCNNRLNet()
	{
		if( m_label!=nullptr ) { delete[] m_label; }
		if( m_weight!=nullptr ) { delete[] m_weight; }
	}

	void init();
	void update( bool bIsPositive );
	void push_back( const float* data, float baseLineValue, Color color, uint filter, SymmetryType type = SYM_NORMAL );

private:
	void inputDataIntoLayers();
};

#endif 
