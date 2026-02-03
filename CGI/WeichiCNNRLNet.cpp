#include "WeichiCNNRLNet.h"
#include "StaticBoard.h"

void WeichiCNNRLNet::init()
{
#ifdef USE_CAFFE
	// read solver parameters
	caffe::ReadProtoFromTextFileOrDie(m_sSolver_positive, &m_solver_param_positive);
	caffe::ReadProtoFromTextFileOrDie(m_sSolver_negative, &m_solver_param_negative);

	// create solver
	m_solver.reset(caffe::SolverRegistry<float>::CreateSolver(m_solver_param_positive));

	m_caffe_net = m_solver->net();
	m_data_input_layer = boost::dynamic_pointer_cast<caffe::MemoryDataLayer<float> >(m_caffe_net->layer_by_name("data_input_layer"));
	assert(m_data_input_layer);
	m_weight_input_layer = boost::dynamic_pointer_cast<caffe::MemoryDataLayer<float> >(m_caffe_net->layer_by_name("weight_input_layer"));
	assert(m_weight_input_layer);
	// load param weight
	m_caffe_net->CopyTrainedLayersFrom(m_param.m_sCaffeModel.c_str());

	// load batch size
	m_batch_size = m_caffe_net->blob_by_name("data")->num();
	m_channel_size = m_caffe_net->blob_by_name("data")->channels();
	int height = m_caffe_net->blob_by_name("data")->	height();
	int width = m_caffe_net->blob_by_name("data")->	width();
	m_input_data_size = m_channel_size * height * width;

	// malloc cpu array data
	int real_batch_size = m_batch_size*WeichiConfigure::dcnn_train_gpu_list.length();
	m_label = new float[real_batch_size];
	m_weight = new float[real_batch_size];
	m_color = new Color[real_batch_size];
	m_reverseSymmetric = new SymmetryType[real_batch_size];
	m_input = new float[real_batch_size*m_input_data_size];
	m_dummy = new float[real_batch_size*m_input_data_size];


	// for caffe multi-gpu
	vector<int> vGpu;
	for( uint i=0; i<WeichiConfigure::dcnn_train_gpu_list.length(); i++ ) {
		vGpu.push_back(WeichiConfigure::dcnn_train_gpu_list[i]-'0');
	}

	m_solver_param_positive.set_device_id(vGpu[0]);
	m_solver_param_negative.set_device_id(vGpu[0]);
	caffe::Caffe::SetDevice(vGpu[0]);
	caffe::Caffe::set_mode(Caffe::GPU);
	caffe::Caffe::set_solver_count(vGpu.size());

	caffe::P2PSync<float>* sync = new caffe::P2PSync<float>(m_solver, NULL, m_solver->param());
	boost::shared_ptr< caffe::P2PSync<float> > tmp(sync);
	m_sync = tmp;
	m_vSync.resize(vGpu.size());
	m_sync->Prepare(vGpu, &m_vSync);
#endif
}

void WeichiCNNRLNet::update( bool bIsPositive )
{
#ifdef USE_CAFFE
	inputDataIntoLayers();

	// change solver
	caffe::SolverParameter& param = const_cast<caffe::SolverParameter&>(m_sync->solver()->param());
	if( bIsPositive ) { param = m_solver_param_positive; }
	else { param = m_solver_param_negative; }

	for( int i=1; i<m_vSync.size(); i++ ) { m_vSync[i]->StartInternalThread(); }
	m_sync->solver()->Step(1);
	for( int i=1; i<m_vSync.size(); i++ ) { m_vSync[i]->StopInternalThread(); }
#endif
	resetCurrentBatchSize();
}

void WeichiCNNRLNet::push_back( const float* data, float baseLineValue, Color color, uint filter, SymmetryType type /*= SYM_NORMAL*/ )
{
	m_weight[m_current_batch_size] = baseLineValue;
	filter = rotatePosition(filter, type);
	m_label[m_current_batch_size] = static_cast<float>(filter);
	BaseCNNNet::setBatchData(data,color);

	/*
		random number is already decided in RL PlayoutGenerator.
		Also, it is not reasonable to rotate here since the reward should be given according to the symmetry type of
		which RL PlayoutGenerator was used then. If we do rotating here, the training can easily break down. 

	*/
}

void WeichiCNNRLNet::inputDataIntoLayers()
{
#ifdef USE_CAFFE
	m_data_input_layer->Reset(m_input, m_label, m_batch_size*WeichiConfigure::dcnn_train_gpu_list.length());
	m_weight_input_layer->Reset(m_weight, m_weight, m_batch_size*WeichiConfigure::dcnn_train_gpu_list.length());
#endif
}