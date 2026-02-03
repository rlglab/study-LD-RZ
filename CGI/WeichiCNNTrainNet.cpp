#include "WeichiCNNTrainNet.h"
#include "TimeSystem.h"
#include "ColorMessage.h"
#include "StaticBoard.h"

void WeichiCNNTrainNet::init( bool bDisplayErrorMessage/*=false*/ )
{
#ifdef USE_CAFFE
	// read solver parameters & create solver
	caffe::ReadProtoFromTextFileOrDie(m_sSolver, &m_solverParam);
	m_solver.reset(caffe::SolverRegistry<float>::CreateSolver(m_solverParam));

	// create memory date layers
	m_caffe_net = m_solver->net();
	m_data_input_layer = boost::dynamic_pointer_cast<caffe::MemoryDataLayer<float> >(m_caffe_net->layer_by_name("data_input_layer"));
	m_label_input_layer = boost::dynamic_pointer_cast<caffe::MemoryDataLayer<float> >(m_caffe_net->layer_by_name("label_input_layer"));
	assert(m_data_input_layer);
	assert(m_label_input_layer);

	// load param weight
	if( m_param.m_sCaffeModel.find("solverstate")!=string::npos ) { m_solver->Restore(m_param.m_sCaffeModel.c_str()); }
	else if( m_param.m_sCaffeModel.find("caffemodel")!=string::npos ) { m_caffe_net->CopyTrainedLayersFrom(m_param.m_sCaffeModel.c_str()); }
	else {
		m_bRandomInitWeight = true;
		if (bDisplayErrorMessage) { CERR() << "Snapshot not found, random initial weights." << endl; }
	}

	// load batch size & label size
	m_batch_size = m_caffe_net->blob_by_name("data")->num();
	m_channel_size = m_caffe_net->blob_by_name("data")->channels(); 
	m_label_size = m_caffe_net->blob_by_name("label")->count(1);
	m_input_data_size = m_channel_size * WeichiConfigure::TotalGrids;

	// check sl/bv/vn label size
	m_sl_label_size = m_caffe_net->has_blob(WeichiConfigure::SL_LABEL_NAME) ? m_caffe_net->blob_by_name("label_sl")->count(1) : -1;
	m_bv_label_size = m_caffe_net->has_blob(WeichiConfigure::BV_OUTPUT_NAME) ? m_caffe_net->blob_by_name(WeichiConfigure::BV_OUTPUT_NAME)->count(1) : -1;
	m_vn_label_size = m_caffe_net->has_blob(WeichiConfigure::VN_OUTPUT_NAME) ? m_caffe_net->blob_by_name(WeichiConfigure::VN_OUTPUT_NAME)->count(1) : -1;
	m_gl_label_size = m_caffe_net->has_blob(WeichiConfigure::GL_LABEL_NAME) ? m_caffe_net->blob_by_name("label_gl")->count(1) : -1;
	if (bDisplayErrorMessage) { CERR() << "Using batch size = " << m_batch_size << ", label size = " << m_label_size << endl; }

	// malloc cpu array data
	m_gpu_size = WeichiConfigure::dcnn_train_gpu_list.length();
	int real_batch_size = m_batch_size * m_gpu_size;
	int buffer_batch_size = 2 * real_batch_size;

	// base variable
	m_color = new Color[buffer_batch_size];
	m_reverseSymmetric = new SymmetryType[buffer_batch_size];
	m_input = new float[buffer_batch_size*m_input_data_size];
	m_dummy = new float[buffer_batch_size*m_input_data_size];
	
	// member variable
	m_label = new float[buffer_batch_size * m_label_size];
	m_moveNumber = new float[buffer_batch_size];
	m_symmetric = new SymmetryType[buffer_batch_size];
	
	// for caffe multi-gpu
	vector<int> vGpu;
	for( uint i=0; i<m_gpu_size; i++ ) {
		vGpu.push_back(WeichiConfigure::dcnn_train_gpu_list[i]-'0');
	}

	m_solverParam.set_device_id(vGpu[0]);
	caffe::Caffe::SetDevice(vGpu[0]);
	caffe::Caffe::set_mode(Caffe::GPU);
	caffe::Caffe::set_solver_count(vGpu.size());

	caffe::P2PSync<float>* sync = new caffe::P2PSync<float>(m_solver, NULL, m_solver->param());
	boost::shared_ptr< caffe::P2PSync<float> > tmp(sync);
	m_sync = tmp;
	m_vSync.resize(vGpu.size());
	m_sync->Prepare(vGpu, &m_vSync);
	for( int i=1; i<m_vSync.size(); i++ ) { m_vSync[i]->StartInternalThread(); }
	
	// initialize caffe net for all gpus
	m_vCaffeNet.resize(vGpu.size());
	for( uint i=0; i<m_vCaffeNet.size(); i++ ) {
		if( i==0 ) { m_vCaffeNet[i] = m_caffe_net; }
		else { m_vCaffeNet[i] = m_vSync[i]->solver()->net(); }
	}
#elif USE_CAFFE2

	caffe2::NetDef initDef; // only run once, not neccesary to be a member
	
	// for caffe2 multi-gpu
	m_gpu_size = WeichiConfigure::dcnn_train_gpu_list.length();
	m_vGpu.clear();
	for (uint i = 0; i<m_gpu_size; i++) {
		m_vGpu.push_back(WeichiConfigure::dcnn_train_gpu_list[i] - '0');
	}
	string firstGPUID = to_string(m_vGpu[0]);

	//load train net and init net
	CAFFE_ENFORCE(ReadProtoFromFile(m_param.m_sCaffeModel, &initDef));
	CAFFE_ENFORCE(ReadProtoFromFile(m_param.m_sPrototxt, &m_trainDef));
	int pretrainIter = checkIteration(initDef);

	// create FileStoreHandlerCreate operator to record NFS path and job ID for distributed training
	if (hasNFSPath()) { addDistributedArg(WeichiConfigure::NFS_path, WeichiConfigure::run_id, initDef); }

	// load data (setting and weight) 
	m_caffe2_workspace.RunNetOnce(initDef);
	// load network architecture
	m_caffe2_workspace.CreateNet(m_trainDef, false);
	// we must know what GPU (record in deviceOption) that blob is belong to
	for (int i = 0; i < m_trainDef.op_size(); ++i) {
		for (int j = 0; j < m_trainDef.op(i).input_size(); ++j) {
			const string &inputBlobName = m_trainDef.op(i).input(j);
			if (m_BlobOptionMap.find(inputBlobName) == m_BlobOptionMap.end()) {
				m_BlobOptionMap[inputBlobName] = m_trainDef.op(i).device_option();
			}
		}
	}

	//revise the workspace iteration (RunNetOnce function would reset iteration to 0)
	for (uint i = 0; i < m_gpu_size; i++) {
		m_caffe2_workspace.GetBlob("gpu_" + std::to_string(m_vGpu[i]) + "/iter")->GetMutable<caffe2::TensorCPU>()->mutable_data<long>()[0] = pretrainIter;
	}
	
	// load batch size & label size, default we'll run it on GPU(TensorCUDA)
	// after generate data parellel code by python, take the first GPU setting for global setting

	//dims: 0-> batch size, 1-> channel size
	m_batch_size = m_caffe2_workspace.GetBlob("gpu_" + firstGPUID  + "/data")->Get<caffe2::TensorCUDA>().dims()[0]; 
	m_channel_size = m_caffe2_workspace.GetBlob("gpu_" + firstGPUID + "/data")->Get<caffe2::TensorCUDA>().dims()[1];

	m_input_data_size = m_channel_size * WeichiConfigure::TotalGrids;
	
	// check sl/bv/vn label size
	// in caffe2, our label blob only accept 1 dimension vector for softmaxLoss. That means we need to divide batchSize to get the real label size for SL
	if (bDisplayErrorMessage) { CERR() << "Using batch size = " << m_batch_size << ", label size = " << m_label_size << endl; }

	// malloc cpu array data
	int real_batch_size = m_batch_size * m_gpu_size;
	int buffer_batch_size = 2 * real_batch_size;

	m_sl_label_size = -1;
	m_sl_distribution_label_size = -1;
	m_bv_label_size = -1;
	m_vn_label_size = -1;
	m_gl_label_size = -1;
	m_label_size = 0;
	if (m_caffe2_workspace.HasBlob("gpu_" + firstGPUID + "/" + WeichiConfigure::SL_OUTPUT_NAME)) {
		if (!m_caffe2_workspace.HasBlob("gpu_" + firstGPUID + "/" + WeichiConfigure::SL_LABEL_NAME)) {
			CERR() << "Blob name mismatch: cannot find Blob for sl_label size " << endl;
			CERR() << "-> missing Blob: " << string("gpu_" + firstGPUID + "/" + WeichiConfigure::SL_LABEL_NAME) << endl;
			exit(0);
		}
		m_sl_label_size = m_caffe2_workspace.GetBlob("gpu_" + firstGPUID + "/" + WeichiConfigure::SL_LABEL_NAME)->Get<caffe2::TensorCUDA>().dims()[0] / m_batch_size;
		m_sl_label = new int[buffer_batch_size * m_sl_label_size];
		m_label_size += m_sl_label_size;
		//for sl_distribution
		if (m_caffe2_workspace.HasBlob("gpu_" + firstGPUID + "/" + WeichiConfigure::SL_DISTRIBUTION_LABEL_NAME)) {
			m_sl_distribution_label_size = m_caffe2_workspace.GetBlob("gpu_" + firstGPUID + "/" + WeichiConfigure::SL_DISTRIBUTION_LABEL_NAME)->Get<caffe2::TensorCUDA>().dims()[1];
			m_sl_distribution_label = new float[buffer_batch_size * m_sl_distribution_label_size];
			m_label_size += m_sl_distribution_label_size;
		}
	}
	if (m_caffe2_workspace.HasBlob("gpu_" + firstGPUID + "/" + WeichiConfigure::BV_OUTPUT_NAME)) {
		if (!m_caffe2_workspace.HasBlob("gpu_" + firstGPUID + "/" + WeichiConfigure::BV_LABEL_NAME)) {
			CERR() << "Blob name mismatch: cannot find Blob for bv_label size " << endl;
			CERR() << "-> missing Blob: " << string("gpu_" + firstGPUID + "/" + WeichiConfigure::BV_LABEL_NAME) << endl;
			exit(0);
		}
		m_bv_label_size = m_caffe2_workspace.GetBlob("gpu_" + firstGPUID + "/" + WeichiConfigure::BV_LABEL_NAME)->Get<caffe2::TensorCUDA>().dims()[1];
		m_bv_label = new float[buffer_batch_size * m_bv_label_size];
		m_label_size += m_bv_label_size;
	}
	if (m_caffe2_workspace.HasBlob("gpu_" + firstGPUID + "/" + WeichiConfigure::VN_OUTPUT_NAME)) {
		if (!m_caffe2_workspace.HasBlob("gpu_" + firstGPUID + "/" + WeichiConfigure::VN_LABEL_NAME)) {
			CERR() << "Blob name mismatch: cannot find Blob for vn_label size " << endl;
			CERR() << "-> missing Blob: " << string("gpu_" + firstGPUID + "/" + WeichiConfigure::VN_LABEL_NAME) << endl;
			exit(0);
		}
		m_vn_label_size = m_caffe2_workspace.GetBlob("gpu_" + firstGPUID + "/" + WeichiConfigure::VN_LABEL_NAME)->Get<caffe2::TensorCUDA>().dims()[1];
		m_vn_label = new float[buffer_batch_size * m_vn_label_size];
		m_label_size += m_vn_label_size;
	}
	if (m_caffe2_workspace.HasBlob("gpu_" + firstGPUID + "/" + WeichiConfigure::GL_OUTPUT_NAME)) {
		if (!m_caffe2_workspace.HasBlob("gpu_" + firstGPUID + "/" + WeichiConfigure::GL_LABEL_NAME)) {
			CERR() << "Blob name mismatch: cannot find Blob for gl_label size " << endl;
			CERR() << "-> missing Blob: " << string("gpu_" + firstGPUID + "/" + WeichiConfigure::GL_LABEL_NAME) << endl;
			exit(0);
		}
		m_gl_label_size = m_caffe2_workspace.GetBlob("gpu_" + firstGPUID + "/" + WeichiConfigure::GL_LABEL_NAME)->Get<caffe2::TensorCUDA>().dims()[1];
		m_gl_label = new float[buffer_batch_size * m_gl_label_size];
		m_label_size += m_gl_label_size;
	}

	// base variable
	m_color = new Color[buffer_batch_size];
	m_reverseSymmetric = new SymmetryType[buffer_batch_size];
	m_input = new float[buffer_batch_size*m_input_data_size];
	m_dummy = new float[buffer_batch_size*m_input_data_size];

	m_label = new float[buffer_batch_size * m_label_size];
	m_moveNumber = new float[buffer_batch_size];
	m_symmetric = new SymmetryType[buffer_batch_size];
#endif
}

void WeichiCNNTrainNet::setBaseLearningRate(float lr)
{
#ifdef USE_CAFFE2
	for (int i = 0; i < m_trainDef.op_size(); i++) {
		if (m_trainDef.op(i).type().find("LearningRate") != std::string::npos) {
			for (int j = 0; j < m_trainDef.op(i).arg_size(); j++) {
				if (m_trainDef.op(i).arg(j).name().find("base_lr") != std::string::npos) {
					m_trainDef.mutable_op(i)->mutable_arg(j)->set_f(lr);
				}
			}
		}
	}
	m_caffe2_workspace.CreateNet(m_trainDef, true);
#endif
}

#ifdef USE_CAFFE2
void WeichiCNNTrainNet::addDistributedArg(const string& path, const string& prefix, caffe2::NetDef& initDef)
{
	string fileStoreHandle_path = path + "/" + prefix;

	int worker_id, worker_num = 0;

	for (caffe2::OperatorDef op : initDef.op()) {
		if (op.output(0).find("allreduce_0_cw") != std::string::npos) {
			for (caffe2::Argument arg : op.arg()) {
				if (arg.name().find("size") != std::string::npos) {
					worker_num = arg.has_i()? arg.i() : arg.ints(0);
				} else if (arg.name().find("rank") != std::string::npos) {
					worker_id = arg.has_i()? arg.i() : arg.ints(0);
				}
			}
			break;
		}
	}

	if (worker_num == 0) {
		CERR() << "[Warning] training net doesn't have any allreduce operator." << endl;
		return;
	}

	string worker_configure = fileStoreHandle_path + "/worker" + to_string(worker_id) + ".log";

	if (!boost::filesystem::exists(fileStoreHandle_path)) {
		boost::filesystem::create_directory(fileStoreHandle_path);
	}
	
	if (!boost::filesystem::exists(worker_configure)) {
		fstream fin(worker_configure, ios::out);
	} else {
		CERR() << "[Error] NFS path already exists the worker" << to_string(worker_id) << ".log." << endl;
		exit(0);
	}

	caffe2::NetDef net;
	caffe2::OperatorDef *op = net.add_op();

	op->set_type("FileStoreHandlerCreate");
	op->set_name("");
	op->add_output("store_handler");
	op->add_arg()->CopyFrom(caffe2::MakeArgument("path", fileStoreHandle_path));
	op->add_arg()->CopyFrom(caffe2::MakeArgument("prefix", prefix));
	m_caffe2_workspace.RunNetOnce(net); 
}
#endif

#ifdef USE_CAFFE2
int WeichiCNNTrainNet::checkIteration(caffe2::NetDef& initDef)
{
	int pretrainIter = 0;
	for (caffe2::OperatorDef op : initDef.op()) {
		if (op.output(0).find("iter") != std::string::npos) {
			for (caffe2::Argument arg : op.arg()) {
				if (arg.name().find("value") != std::string::npos) {
					pretrainIter = arg.has_i()? arg.i() : arg.ints(0);
				}
			}
			break;
		}
	}
	return pretrainIter;
}
#endif

void WeichiCNNTrainNet::update( bool bEvaluateFirstBatch )
{
	inputDataIntoLayers(bEvaluateFirstBatch);
#ifdef USE_CAFFE
	m_sync->solver()->Step(1);
#elif USE_CAFFE2
	m_caffe2_workspace.RunNet(m_trainDef.name());
	long iter = getIteration();
	if (iter % WeichiConfigure::snapshot == 0) { saveNetWork(WeichiConfigure::snapshot_prefix + std::to_string(iter) + ".pb"); }
	if (iter % WeichiConfigure::display == 0) { CERR() << getTrainingInfo(); }
#endif
}

void WeichiCNNTrainNet::push_back( bool bEvaluateFirstBatch, const float* data, Color turnColor, float* fLabel, float fMoveNumber, SymmetryType type/*=SYM_NORMAL*/ )
{
	int batch_pos = m_current_batch_size + (bEvaluateFirstBatch ? m_batch_size * m_gpu_size : 0);
	set_data(data,turnColor,fLabel,fMoveNumber,batch_pos,type);
	m_current_batch_size++;
}

void WeichiCNNTrainNet::set_data( const float* data, Color turnColor, float* fLabel, float fMoveNumber, int batch_pos, SymmetryType type/* = SYM_NORMAL*/ )
{
	m_color[batch_pos] = turnColor;
	m_moveNumber[batch_pos] = fMoveNumber;

	if( getRandomNum()==0 ) {
		setLabelAtSpecificBatchIndex(fLabel, batch_pos, SYM_NORMAL);
		setDataAtSpecificBatchIndex(data, batch_pos, SYM_NORMAL);
	} else {
		for (int i = 0; i < getRandomNum(); i++){
			int index = Random::nextInt(SYMMETRY_SIZE-i);
			SymmetryType type = m_vSymmetryEntry[index];
			setLabelAtSpecificBatchIndex(fLabel, batch_pos, type);
			setDataAtSpecificBatchIndex(data, batch_pos, type);
			m_vSymmetryEntry.swap(index, SYMMETRY_SIZE - 1 - i);
		}
	}
}

void WeichiCNNTrainNet::saveNetWork(string sSnapshot)
{
	BaseCNNNet::saveNetWork(sSnapshot);
#ifdef USE_CAFFE
	m_solver->Snapshot();
#endif
}

string WeichiCNNTrainNet::getCaffeLossInfo()
{
	ostringstream caffe_loss_info;
#ifdef USE_CAFFE
	const vector<Blob<float>*>& result = m_caffe_net->output_blobs();
	const vector<int>& outputBlobIndex = m_caffe_net->output_blob_indices();
	int score_index = 0;
	for (int j = 0; j < result.size(); ++j) {
		const float* result_vec = result[j]->cpu_data();
		const int index = outputBlobIndex[j];
		const string& output_name = m_caffe_net->blob_names()[index];
		const float loss_weight = m_caffe_net->blob_loss_weights()[index];
		for (int k = 0; k < result[j]->count(); ++k) {
			ostringstream loss_msg_stream;
			if (loss_weight != 0) { loss_msg_stream << " (* " << loss_weight << " = " << loss_weight * result_vec[k] << " loss)"; }
			caffe_loss_info << "Train net output #"  << score_index++ << ": " << output_name << " = " << result_vec[k] << loss_msg_stream.str() << '\n';
		}
	}
#endif
	return caffe_loss_info.str();
}

string WeichiCNNTrainNet::getNetInfomation(string sPrefix/* = ""*/)
{
	ostringstream oss;
	oss << sPrefix << "[train net] " << m_param.m_sPrototxt << endl;
	oss << sPrefix << "[batch size] " << getBatchSize() << endl;
	oss << sPrefix << "[channel size] " << getChannelSize() << endl;
	oss << sPrefix << "[net type] " << getWeichiCNNNetTypeString(getCNNNetType()) << endl;
	oss << sPrefix << "[feature type] " << getWeichiCNNFeatureTypeString(getCNNFeatureType()) << endl;
	oss << sPrefix << "[random rotate number] " << getRandomNum() << endl;
	oss << sPrefix << "[snapshot] " << ((m_bRandomInitWeight) ? "Random Weight" : m_param.m_sCaffeModel) << endl;
#ifdef USE_CAFFE
	oss << sPrefix << "[solver] " << m_sSolver << endl;
	oss << sPrefix << "[learning rate] " << m_solver->param().base_lr() << endl;
#elif USE_CAFFE2
	//oss << sPrefix << "[learning rate] " << caffe2::TensorCPU(m_caffe2_workspace.GetBlob("gpu_" + to_string(m_vGpu[0]) + "/lr")->Get<caffe2::TensorCUDA>()).data<float>()[0] << endl;
#endif
	oss << sPrefix << "[gpu_list] " << WeichiConfigure::dcnn_train_gpu_list << endl;
	oss << sPrefix << "[label size] " << getLabelSize() << endl;
	if (hasSLOutput()) { oss << sPrefix << "\t[SL label size] " << getSLLabelSize() << endl; }
	if (hasVNOutput()) { oss << sPrefix << "\t[VN label size] " << getVNLabelSize() << endl; }
	if (hasBVOutput()) { oss << sPrefix << "\t[BV label size] " << getBVLabelSize() << endl; }
	if (hasGLOutput()) { oss << sPrefix << "\t[GL label size] " << getGLLabelSize() << endl; }
	
	return oss.str();
}

string WeichiCNNTrainNet::getTrainingInfo()
{
	ostringstream oss;

#ifdef USE_CAFFE
	oss << "Iteration = " << getIteration() << endl;

	if (hasSLOutput()) { oss << getSLTrainingInfo(); }
	if (hasBVOutput()) { oss << getBVTrainingInfo(); }
	if (hasVNOutput()) { oss << getVNTrainingInfo(); }
	if (hasGLOutput()) { oss << getGLTrainingInfo(); }

	oss << "\n === Caffe summary === \n" \
		<< "[" << TimeSystem::getTimeString() << "] Iteration " << getIteration() << '\n'
		<< "Base Learning rate = " << m_solver->param().base_lr() << '\n';
	oss << getCaffeLossInfo() << endl;
#elif USE_CAFFE2
	oss << "Iteration = " << getIteration() << endl;

	if (hasSLOutput()) { oss << getSLTrainingInfo(); }
	if (hasBVOutput()) { oss << getBVTrainingInfo(); }
	if (hasVNOutput()) { oss << getVNTrainingInfo(); }
	if (hasGLOutput()) { oss << getGLTrainingInfo(); }

	float sum_value_loss = 0.0, sum_sl_softmax_loss = 0.0, sum_top_1_01 = 0.0, sum_top_1_05 = 0.0, sum_top_1_10 = 0.0, sl_scale = 0.0, bv_scale = 0.0, vn_scale = 0.0;
	for (int id : m_vGpu) {
		auto top1_blob = m_caffe2_workspace.GetBlob("gpu_" + to_string(id) + "/top_1_01");
		auto top5_blob = m_caffe2_workspace.GetBlob("gpu_" + to_string(id) + "/top_1_05");
		auto top10_blob = m_caffe2_workspace.GetBlob("gpu_" + to_string(id) + "/top_1_10");

		sum_value_loss += caffe2::TensorCPU(m_caffe2_workspace.GetBlob("gpu_" + to_string(id) + "/avg_value_loss")->Get<caffe2::TensorCUDA>()).data<float>()[0];
		sum_sl_softmax_loss += caffe2::TensorCPU(m_caffe2_workspace.GetBlob("gpu_" + to_string(id) + "/_sl_softmax_loss_1")->Get<caffe2::TensorCUDA>()).data<float>()[0];
		if (top1_blob->IsType<caffe2::TensorCUDA>()) {
			sum_top_1_01 += caffe2::TensorCPU(top1_blob->Get<caffe2::TensorCUDA>()).data<float>()[0];
		} else {
			sum_top_1_01 += top1_blob->Get<caffe2::TensorCPU>().data<float>()[0];
		}

		if (top5_blob->IsType<caffe2::TensorCUDA>()) {
			sum_top_1_05 += caffe2::TensorCPU(top5_blob->Get<caffe2::TensorCUDA>()).data<float>()[0];
		} else {
			sum_top_1_05 += top5_blob->Get<caffe2::TensorCPU>().data<float>()[0];
		}

		if (top10_blob->IsType<caffe2::TensorCUDA>()) {
			sum_top_1_10 += caffe2::TensorCPU(top10_blob->Get<caffe2::TensorCUDA>()).data<float>()[0];
		} else {
			sum_top_1_10 += top10_blob->Get<caffe2::TensorCPU>().data<float>()[0];
		}
	}

	oss << "\n === Caffe2 summary === \n" \
		<< "[" << TimeSystem::getTimeString() << "] Iteration " << getIteration() << '\n'
		<< "Base Learning rate = " << caffe2::TensorCPU(m_caffe2_workspace.GetBlob("gpu_" + to_string(m_vGpu[0]) + "/lr")->Get<caffe2::TensorCUDA>()).data<float>()[0] << '\n'
		<< "Train net output #0: loss_vn = " << (double) (sum_value_loss / m_gpu_size) << '\n'
		<< "Train net output #1: sl_softmax_loss_1 = " << (double) (sum_sl_softmax_loss / m_gpu_size) << '\n'
		<< "Train net output #2: top_1_01 = " << (double) (sum_top_1_01 / m_gpu_size) << '\n'
		<< "Train net output #2: top_1_05 = " << (double) (sum_top_1_05 / m_gpu_size) << '\n'
		<< "Train net output #2: top_1_10 = " << (double) (sum_top_1_10 / m_gpu_size) << '\n'
		<< "GPU size = " << m_gpu_size << '\n';
	oss << getCaffeLossInfo() << endl;
#endif

	return oss.str();
}

// private function
void WeichiCNNTrainNet::setDataAtSpecificBatchIndex(const float* data, int batch_pos, SymmetryType type/* = SYM_NORMAL*/)
{
	m_symmetric[batch_pos] = type;
	m_reverseSymmetric[batch_pos] = ReverseSymmetricType[type];

	int start_index = batch_pos*m_input_data_size;
	memcpy((m_input + start_index), data, sizeof(float)*m_input_data_size);

	rotateFeature((m_input + start_index), type);
}

void WeichiCNNTrainNet::setLabelAtSpecificBatchIndex(const float* label, int batch_pos, SymmetryType type/* = SYM_NORMAL*/)
{
#ifdef USE_CAFFE
	int iIndex = 0;
	int shift = batch_pos * m_label_size;

	if (hasSLOutput()) {
		m_label[shift + iIndex] = static_cast<float>(WeichiMove::rotateCompactPosition(static_cast<uint>(label[iIndex]), type));
		iIndex++;

		if (m_caffe_net->has_blob("label_sl_distribution")) {
			int base_index = iIndex;
			for (uint pos = 0; pos < WeichiConfigure::TotalGrids + 1; pos++) {
				m_label[shift + base_index + WeichiMove::rotateCompactPosition(pos, type)] = label[base_index + pos];
			}
			iIndex += WeichiConfigure::TotalGrids + 1;
		}
	}

	if (hasVNOutput()) {
		int vn_end_index = iIndex + getVNLabelSize();
		for (; iIndex < vn_end_index; iIndex++) {
			m_label[shift + iIndex] = label[iIndex];
		}
	}

	if (hasBVOutput()) {
		int base_index = iIndex;
		for (uint pos = 0; pos < WeichiConfigure::TotalGrids; pos++) {
			m_label[shift + base_index + WeichiMove::rotateCompactPosition(pos, type)] = label[base_index + pos];
		}
		iIndex += WeichiConfigure::TotalGrids;
	}

	if (hasGLOutput()) {
		m_label[shift + iIndex] = label[iIndex];
		iIndex++;
	}

	if (iIndex != m_label_size) {
		CERR() << "[Error] Label size not match. " << iIndex << " != " << m_label_size << endl;
		exit(0);
	}
#elif USE_CAFFE2
	int iIndex = 0;
	int shift; 

	if (hasSLOutput()) {
		shift = batch_pos * m_sl_label_size;
		m_sl_label[shift] = static_cast<float>(WeichiMove::rotateCompactPosition(static_cast<uint>(label[iIndex]), type));
		iIndex++;
	}

	if (hasSLDistribution()) {
		shift = batch_pos * m_sl_distribution_label_size;
		int base_index = iIndex;
		for (uint pos = 0; pos < WeichiConfigure::TotalGrids + 1; pos++) {
			m_sl_distribution_label[shift + WeichiMove::rotateCompactPosition(pos, type)] = label[base_index + pos];
		}
		iIndex += WeichiConfigure::TotalGrids + 1;
	}

	if (hasBVOutput()) {
		//TODO
		//shift = batch_pos * m_bv_label_size;
		//int base_index = iIndex;
		//for (int i = 0; i < m_bv_label_size; i++) {
		//	m_bv_label[shift + i] = label[base_index + i];
		//}
		//iIndex += m_bv_label_size;
	}

	if (hasVNOutput()) {
		shift = batch_pos * m_vn_label_size;
		int base_index = iIndex;
		for (int i = 0; i < m_vn_label_size; i++) {
			m_vn_label[shift + i] = label[base_index + i];
		}
		iIndex += m_vn_label_size;
	}

	if (hasGLOutput()) {
		// TODO: add GL output here
	}

	if (iIndex != m_label_size) {
		CERR() << "[Error] Label size not match." << endl;
		exit(0);
	}
#endif
}

void WeichiCNNTrainNet::inputDataIntoLayers(bool bEvaluateFirstBatch)
{
#ifdef USE_CAFFE
	uint batch_size = m_batch_size * m_gpu_size;
	float *input = m_input + (bEvaluateFirstBatch ? 0 : batch_size*m_input_data_size);
	float *label = m_label + (bEvaluateFirstBatch ? 0 : batch_size*m_label_size);
	float *move_number = m_moveNumber + (bEvaluateFirstBatch ? 0 : batch_size);

	// we take move number as dummy data, in order to check data consistency
	m_data_input_layer->Reset(input, move_number, batch_size);
	m_label_input_layer->Reset(label, move_number, batch_size);
#elif USE_CAFFE2

	uint batch_size = m_batch_size * m_gpu_size;

	for (int i = 0; i < m_gpu_size; ++i) {
		
		float *input = m_input + (bEvaluateFirstBatch ? 0 : batch_size * m_input_data_size) + m_batch_size * i * m_input_data_size;
		string dataBlobName = "gpu_" + to_string(m_vGpu[i]) + string("/data");
		inputDataIntoLayers<float>(dataBlobName, input);

		if (hasSLOutput()) {
			int *slLabel = m_sl_label + (bEvaluateFirstBatch ? 0 : batch_size * m_sl_label_size) + m_batch_size * i * m_sl_label_size;
			string slInputName = "gpu_" + to_string(m_vGpu[i]) + "/" + WeichiConfigure::SL_LABEL_NAME;
			inputDataIntoLayers<int>(slInputName, slLabel);
		}
		if (hasBVOutput()) {
			float *bvLabel = m_bv_label + (bEvaluateFirstBatch ? 0 : batch_size * m_bv_label_size) + m_batch_size * i * m_bv_label_size;
			string bvInputName = "gpu_" + to_string(m_vGpu[i]) + "/" + WeichiConfigure::BV_LABEL_NAME;
			inputDataIntoLayers<float>(bvInputName, bvLabel);
		}
		if (hasVNOutput()) {
			float *vnLabel = m_vn_label + (bEvaluateFirstBatch ? 0 : batch_size * m_vn_label_size) + m_batch_size * i * m_vn_label_size;
			string vnInputName = "gpu_" + to_string(m_vGpu[i]) + "/" + WeichiConfigure::VN_LABEL_NAME;
			inputDataIntoLayers<float>(vnInputName, vnLabel);
		}
		if (hasSLDistribution()) {
			float *slDistributionLabel = m_sl_distribution_label 
				+ (bEvaluateFirstBatch ? 0 : batch_size * m_sl_distribution_label_size) + m_batch_size * i * m_sl_distribution_label_size;
			string slDistributionInputName = "gpu_" + to_string(m_vGpu[i]) + "/" + WeichiConfigure::SL_DISTRIBUTION_LABEL_NAME;
			inputDataIntoLayers<float>(slDistributionInputName, slDistributionLabel);
		}
		if (hasGLOutput()) {
			// TODO
		}
	}
	
#endif
}

string WeichiCNNTrainNet::getSLTrainingInfo()
{
	vector< vector<string> > vAllInfo;
	for (uint i = 0; i < m_gpu_size; i++) { vAllInfo.push_back(getSLTrainingInfo(i)); }

	ostringstream ossTitle;
	ossTitle << setw(6) << "label" << setw(5) << "rank" << setw(8) << "moveNo";
	string sSplitSpace = "    ";

	// Title
	ostringstream oss;
	for (uint i = 0; i < m_gpu_size; i++) { oss << ossTitle.str() << sSplitSpace; }
	oss << endl;

	// Info
	for (uint line = 0; line < m_batch_size; line++) {
		for (uint gpu = 0; gpu < m_gpu_size; gpu++) { oss << vAllInfo[gpu][line] << sSplitSpace; }
		oss << endl;
	}

	// Title
	for (uint i = 0; i < m_gpu_size; i++) { oss << ossTitle.str() << sSplitSpace; }
	oss << endl;

#ifdef USE_CAFFE
	// Summary loss
	oss << "sl_softmax_loss_1 = ";
	for (uint i = 0; i < m_vCaffeNet.size(); i++) { oss << left << setw(9) << m_vCaffeNet[i]->blob_by_name("sl_softmax_loss_1")->cpu_data()[0]; }
	oss << endl << "top_1_01 = ";
	for (uint i = 0; i < m_vCaffeNet.size(); i++) { oss << left << setw(9) << m_vCaffeNet[i]->blob_by_name("top_1_01")->cpu_data()[0]; }
	oss << endl << "top_1_05 = ";
	for (uint i = 0; i < m_vCaffeNet.size(); i++) { oss << left << setw(9) << m_vCaffeNet[i]->blob_by_name("top_1_05")->cpu_data()[0]; }
	oss << endl << "top_1_10 = ";
	for (uint i = 0; i < m_vCaffeNet.size(); i++) { oss << left << setw(9) << m_vCaffeNet[i]->blob_by_name("top_1_10")->cpu_data()[0]; }
	oss << endl;
#elif USE_CAFFE2
	oss << "sl_softmax_loss_1 = ";
	for (uint i = 0; i < m_gpu_size; i++) {
		oss << left << setw(9) << caffe2::TensorCPU(m_caffe2_workspace.GetBlob("gpu_" + to_string(m_vGpu[i]) + "/sl_softmax_loss_1")->Get<caffe2::TensorCUDA>()).data<float>()[0];
	}
	if (m_caffe2_workspace.HasBlob("gpu_" + to_string(m_vGpu[0]) + "/top_1_01")) {
		oss << endl << "top_1_01 = ";
		for (uint i = 0; i < m_gpu_size; i++) {
			if (m_caffe2_workspace.GetBlob("gpu_" + to_string(m_vGpu[i]) + "/top_1_01")->IsType<caffe2::TensorCUDA>()) {
				oss << left << setw(9) << caffe2::TensorCPU(m_caffe2_workspace.GetBlob("gpu_" + to_string(m_vGpu[i]) + "/top_1_01")->Get<caffe2::TensorCUDA>()).data<float>()[0];
			} else {
				oss << left << setw(9) << m_caffe2_workspace.GetBlob("gpu_" + to_string(m_vGpu[i]) + "/top_1_01")->Get<caffe2::TensorCPU>().data<float>()[0];
			}
		}
	}
	if (m_caffe2_workspace.HasBlob("gpu_" + to_string(m_vGpu[0]) + "/top_1_05")) {
		oss << endl << "top_1_05 = ";
		for (uint i = 0; i < m_gpu_size; i++) {
			if (m_caffe2_workspace.GetBlob("gpu_" + to_string(m_vGpu[i]) + "/top_1_05")->IsType<caffe2::TensorCUDA>()) {
				oss << left << setw(9) << caffe2::TensorCPU(m_caffe2_workspace.GetBlob("gpu_" + to_string(m_vGpu[i]) + "/top_1_05")->Get<caffe2::TensorCUDA>()).data<float>()[0];
			} else {
				oss << left << setw(9) << m_caffe2_workspace.GetBlob("gpu_" + to_string(m_vGpu[i]) + "/top_1_05")->Get<caffe2::TensorCPU>().data<float>()[0];
			}
		}
	}
	if (m_caffe2_workspace.HasBlob("gpu_" + to_string(m_vGpu[0]) + "/top_1_10")) {
		oss << endl << "top_1_10 = ";
		for (uint i = 0; i < m_gpu_size; i++) {
			if (m_caffe2_workspace.GetBlob("gpu_" + to_string(m_vGpu[i]) + "/top_1_10")->IsType<caffe2::TensorCUDA>()) {
				oss << left << setw(9) << caffe2::TensorCPU(m_caffe2_workspace.GetBlob("gpu_" + to_string(m_vGpu[i]) + "/top_1_10")->Get<caffe2::TensorCUDA>()).data<float>()[0];
			} else {
				oss << left << setw(9) << m_caffe2_workspace.GetBlob("gpu_" + to_string(m_vGpu[i]) + "/top_1_10")->Get<caffe2::TensorCPU>().data<float>()[0];
			}
		}
	}
	oss << endl;
#endif
	return oss.str();
}

vector<string> WeichiCNNTrainNet::getSLTrainingInfo(int gpu_id)
{
	vector<string> vInfo;
#ifdef USE_CAFFE
	const float *fSLOutput = m_vCaffeNet[gpu_id]->blob_by_name("flatten_sl_move")->cpu_data();
	const float *fLabel = m_vCaffeNet[gpu_id]->blob_by_name("label_sl")->cpu_data();
	const float *fDataMove = m_vCaffeNet[gpu_id]->blob_by_name("dummy_data")->cpu_data();
	const float *fLabelMove = m_vCaffeNet[gpu_id]->blob_by_name("dummy_label")->cpu_data();

	for (int batch = 0; batch < m_batch_size; batch++) {
		const uint label_pos = static_cast<uint>(fLabel[batch]);
		const float *fExponential = fSLOutput + batch * (WeichiConfigure::TotalGrids + 1);
		const float fLabel_exponential = fExponential[label_pos];
		
		uint rank = 1;
		for (int i = 0; i < (WeichiConfigure::TotalGrids + 1); i++) {
			if (i == label_pos) { continue; }
			if (fExponential[i] >= fLabel_exponential) { rank++; }
		}

		if (fDataMove[batch] != fLabelMove[batch]) {
			CERR() << "[Error] DataMove and LabelMove not match." << endl;
			exit(0);
		}

		ostringstream oss;
		oss << setw(6) << label_pos << setw(5) << rank << setw(8) << ToString(fDataMove[batch]);
		vInfo.push_back(oss.str());
	}
#elif USE_CAFFE2
	caffe2::TensorCPU fSLTensor = caffe2::TensorCPU(m_caffe2_workspace.GetBlob("gpu_" + to_string(gpu_id) + "/flatten_sl_move")->Get<caffe2::TensorCUDA>());
	caffe2::TensorCPU fLabelTensor = caffe2::TensorCPU(m_caffe2_workspace.GetBlob("gpu_" + to_string(gpu_id) + "/label_sl")->Get<caffe2::TensorCUDA>());
	const float *fSLOutput = fSLTensor.data<float>();
	const int *fLabel = fLabelTensor.data<int>();

	for (int batch = 0; batch < m_batch_size; batch++) {
		const uint label_pos = static_cast<uint>(fLabel[batch]);
		const float *fExponential = fSLOutput + batch * (WeichiConfigure::TotalGrids + 1);
		const float fLabel_exponential = fExponential[label_pos];

		uint rank = 1;
		for (int i = 0; i < (WeichiConfigure::TotalGrids + 1); i++) {
			if (i == label_pos) { continue; }
			if (fExponential[i] >= fLabel_exponential) { rank++; }
		}

		ostringstream oss;
		oss << setw(6) << label_pos << setw(5) << rank << setw(8) << label_pos;
		vInfo.push_back(oss.str());
	}		
#endif
	return vInfo;
}

string WeichiCNNTrainNet::getBVTrainingInfo()
{
	double dLoss;
	vector< vector<string> > vAllInfo;
	for (uint i = 0; i < m_gpu_size; i++) { vAllInfo.push_back(getBVTrainingInfo(i, dLoss)); }

	ostringstream ossTitle;
	ossTitle << setw(9) << "error" << setw(8) << "moveNo";
	string sSplitSpace = "    ";

	// Title
	ostringstream oss;
	for (uint i = 0; i < m_gpu_size; i++) { oss << ossTitle.str() << sSplitSpace; }
	oss << endl;

	// Info
	for (uint line = 0; line < m_batch_size; line++) {
		for (uint gpu = 0; gpu < m_gpu_size; gpu++) {
			oss << vAllInfo[gpu][line] << sSplitSpace;
		}
		oss << endl;
	}

	// Title
	for (uint i = 0; i < m_gpu_size; i++) { oss << ossTitle.str() << sSplitSpace; }
	oss << endl;

#ifdef USE_CAFFE
	// Summary loss
	uint batch_size = m_batch_size*WeichiConfigure::dcnn_train_gpu_list.length();
	oss << "CGI BV loss = " << dLoss / batch_size << endl;
#elif USE_CAFFE2
	uint batch_size = m_batch_size*WeichiConfigure::dcnn_train_gpu_list.length();
	oss << "CGI BV loss = " << dLoss / batch_size << endl;
#endif

	return oss.str();
}

vector<string> WeichiCNNTrainNet::getBVTrainingInfo(int gpu_id, double& dLoss)
{
	vector<string> vInfo;
#ifdef USE_CAFFE
	const float* fOutput = m_vCaffeNet[gpu_id]->blob_by_name(WeichiConfigure::BV_OUTPUT_NAME)->cpu_data();
	const float *fLabel = m_vCaffeNet[gpu_id]->blob_by_name("label_bv")->cpu_data();
	const float *fDataMove = m_vCaffeNet[gpu_id]->blob_by_name("dummy_data")->cpu_data();
	const float *fLabelMove = m_vCaffeNet[gpu_id]->blob_by_name("dummy_label")->cpu_data();

	for (int batch = 0; batch < m_batch_size; batch++) {
		double dError = 0.0f;
		for (int i = 0; i < m_bv_label_size; i++) { dError += pow((fOutput[batch*m_bv_label_size + i] - fLabel[batch*m_bv_label_size + i]), 2) / 2; }
		dLoss += dError;

		if (fDataMove[batch] != fLabelMove[batch]) {
			CERR() << "[Error] DataMove and LabelMove not match." << endl;
			exit(0);
		}

		ostringstream oss;
		oss << setw(9) << fixed << setprecision(5) << dError << setw(8) << ToString(fDataMove[batch]);
		vInfo.push_back(oss.str());
	}
#elif USE_CAFFE2
	caffe2::TensorCPU fBVTensor = caffe2::TensorCPU(m_caffe2_workspace.GetBlob("gpu_" + to_string(gpu_id) + "/" + WeichiConfigure::BV_OUTPUT_NAME)->Get<caffe2::TensorCUDA>());
	caffe2::TensorCPU fLabelTensor = caffe2::TensorCPU(m_caffe2_workspace.GetBlob("gpu_" + to_string(gpu_id) + "/" + WeichiConfigure::BV_LABEL_NAME)->Get<caffe2::TensorCUDA>());

	const float* fBVOutput = fBVTensor.data<float>();
	const float *fLabel = fLabelTensor.data<float>();

	for (int batch = 0; batch < m_batch_size; batch++) {
		double dError = 0.0f;
		for (int i = 0; i < m_bv_label_size; i++) { dError += pow((fBVOutput[batch*m_bv_label_size + i] - fLabel[batch*m_bv_label_size + i]), 2) / 2; }
		dLoss += dError;

		ostringstream oss;
		//TODO
		oss << setw(9) << fixed << setprecision(5) << dError << setw(8);//<< ToString(fDataMove[batch]);
		vInfo.push_back(oss.str());
	}
#endif
	return vInfo;
}

string WeichiCNNTrainNet::getVNTrainingInfo()
{
	double dMSE[3] = { 0,0,0 };
	vector< vector<string> > vAllInfo;
	for (uint i = 0; i < m_gpu_size; i++) { vAllInfo.push_back(getVNTrainingInfo(i, dMSE)); }

	ostringstream ossTitle;
	ossTitle << setw(9) << "tanh" << setw(9) << "error" << setw(9) << "label" << setw(8) << "moveNo";
	string sSplitSpace = "    ";

	// Title
	ostringstream oss;
	oss << setw(6) << "komi";
	for (uint i = 0; i < m_gpu_size; i++) { oss << ossTitle.str() << sSplitSpace; }
	oss << endl;

	// Info
	for (uint line = 0; line < vAllInfo[0].size(); line++) {
		for (uint gpu = 0; gpu < m_gpu_size; gpu++) {
			oss << vAllInfo[gpu][line] << sSplitSpace;
		}
		oss << endl;
	}

	// Title
	oss << setw(6) << "komi";
	for (uint i = 0; i < m_gpu_size; i++) { oss << ossTitle.str() << sSplitSpace; }
	oss << endl;

#ifdef USE_CAFFE
	// Summary loss
	int iDisplayIndex[3] = { 0,m_vn_label_size / 2,m_vn_label_size - 1 };
	float start_komi = 0;
	if (!WeichiConfigure::dcnn_use_ftl) {
		start_komi = WeichiConfigure::komi - (m_vn_label_size / 2)*WeichiConfigure::caffe_komi_vn_step;
	}

	uint batch_size = m_batch_size*WeichiConfigure::dcnn_train_gpu_list.length();
	for (int i = 0; i < 3; i++) {
		oss << "CGI VN loss " << (start_komi + iDisplayIndex[i]) << " = " << dMSE[i] / batch_size << endl;
		if (m_vn_label_size == 1) { break; }
	}
#elif USE_CAFFE2
	int iDisplayIndex[3] = { 0,m_vn_label_size / 2,m_vn_label_size - 1 };
	float start_komi = WeichiConfigure::komi - (m_vn_label_size / 2)*WeichiConfigure::caffe_komi_vn_step;
	uint batch_size = m_batch_size*WeichiConfigure::dcnn_train_gpu_list.length();
	for (int i = 0; i < 3; i++) {
		oss << "CGI VN loss " << (start_komi + iDisplayIndex[i]) << " = " << dMSE[i] / batch_size << endl;
		if (m_vn_label_size == 1) { break; }
	}
#endif

	return oss.str();
}

vector<string> WeichiCNNTrainNet::getVNTrainingInfo(int gpu_id, double* dMSE)
{
	vector<string> vInfo;
#ifdef USE_CAFFE
	int iDisplayIndex[3] = { 0,m_vn_label_size / 2,m_vn_label_size - 1 };
	float start_komi = 0;
	if (!WeichiConfigure::dcnn_use_ftl) {
		start_komi = WeichiConfigure::komi - (m_vn_label_size / 2)*WeichiConfigure::caffe_komi_vn_step;
	}

	const float *fOutput = m_vCaffeNet[gpu_id]->blob_by_name(WeichiConfigure::VN_OUTPUT_NAME)->cpu_data();
	const float *fLabel = m_vCaffeNet[gpu_id]->blob_by_name("label_vn")->cpu_data();
	const float *fDataMove = m_vCaffeNet[gpu_id]->blob_by_name("dummy_data")->cpu_data();
	const float *fLabelMove = m_vCaffeNet[gpu_id]->blob_by_name("dummy_label")->cpu_data();

	for (int batch = 0; batch < m_batch_size; batch++) {
		for (int i = 0; i < 3; i++) {
			float tanh = fOutput[batch*m_vn_label_size + iDisplayIndex[i]];
			float label = fLabel[batch*m_vn_label_size + iDisplayIndex[i]];
			double dError = pow((tanh - label), 2) / 2;
			dMSE[i] += dError;

			if (fDataMove[batch] != fLabelMove[batch]) {
				CERR() << "[Error] DataMove and LabelMove not match." << endl;
				exit(0);
			}

			ostringstream oss;
			if (gpu_id == 0) { oss << setw(6) << (start_komi + iDisplayIndex[i]); }
			oss << setw(9) << fixed << setprecision(5) << tanh << setw(9) << dError << setw(9) << label;
			oss.unsetf(ios::fixed);
			oss << setw(8) << ToString(fDataMove[batch]);

			if (batch % 2 == 0) { vInfo.push_back(getColorMessage(oss.str(), ANSITYPE_BOLD, ANSICOLOR_WHITE, ANSICOLOR_BLACK)); }
			else { vInfo.push_back(oss.str()); }

			if (m_vn_label_size == 1) { break; }
		}
	}
#elif USE_CAFFE2
	int iDisplayIndex[3] = {0, m_vn_label_size / 2, m_vn_label_size - 1};
	float start_komi = WeichiConfigure::komi - (m_vn_label_size / 2) * WeichiConfigure::caffe_komi_vn_step;

	caffe2::TensorCPU fVNTensor = caffe2::TensorCPU(m_caffe2_workspace.GetBlob("gpu_" + to_string(gpu_id) + "/" + WeichiConfigure::VN_OUTPUT_NAME)->Get<caffe2::TensorCUDA>());
	caffe2::TensorCPU fLabelTensor = caffe2::TensorCPU(m_caffe2_workspace.GetBlob("gpu_" + to_string(gpu_id) + "/label_vn")->Get<caffe2::TensorCUDA>());
	const float *fVNOutput = fVNTensor.data<float>();
	const float *fLabel = fLabelTensor.data<float>();

	for (int batch = 0; batch < m_batch_size; batch++) {
		for (int i = 0; i < 3; i++) {
			float tanh = fVNOutput[batch * m_vn_label_size + iDisplayIndex[i]];
			float label = fLabel[batch * m_vn_label_size + iDisplayIndex[i]];
			double dError = pow((tanh - label), 2) / 2;
			dMSE[i] += dError;

			ostringstream oss;
			if (gpu_id == 0) { oss << setw(6) << (start_komi + iDisplayIndex[i]); }
			oss << setw(9) << fixed << setprecision(5) << tanh << setw(9) << dError << setw(9) << label;
			oss.unsetf(ios::fixed);
			oss << setw(8) << " ";

			if (batch % 2 == 0) { vInfo.push_back(getColorMessage(oss.str(), ANSITYPE_BOLD, ANSICOLOR_WHITE, ANSICOLOR_BLACK)); }
			else { vInfo.push_back(oss.str()); }

			if (m_vn_label_size == 1) { break; }
		}
	}
#endif
	return vInfo;
}

string WeichiCNNTrainNet::getGLTrainingInfo()
{
	vector< vector<string> > vAllInfo;
	for (uint i = 0; i < m_gpu_size; i++) { vAllInfo.push_back(getGLTrainingInfo(i)); }

	ostringstream ossTitle;
	ossTitle << setw(6) << "label" << setw(5) << "rank" << setw(8) << "moveNo";
	string sSplitSpace = "    ";

	// Title
	ostringstream oss;
	for (uint i = 0; i < m_gpu_size; i++) { oss << ossTitle.str() << sSplitSpace; }
	oss << endl;

	// Info
	for (uint line = 0; line < m_batch_size; line++) {
		for (uint gpu = 0; gpu < m_gpu_size; gpu++) { oss << vAllInfo[gpu][line] << sSplitSpace; }
		oss << endl;
	}

	// Title
	for (uint i = 0; i < m_gpu_size; i++) { oss << ossTitle.str() << sSplitSpace; }
	oss << endl;

#ifdef USE_CAFFE
	// Summary loss
	oss << "gl_softmax_loss_1 = ";
	for (uint i = 0; i < m_vCaffeNet.size(); i++) { oss << left << setw(9) << m_vCaffeNet[i]->blob_by_name("gl_softmax_loss_1")->cpu_data()[0]; }
	oss << endl << "gl_top_1_01 = ";
	for (uint i = 0; i < m_vCaffeNet.size(); i++) { oss << left << setw(9) << m_vCaffeNet[i]->blob_by_name("top_gl_1_01")->cpu_data()[0]; }
	oss << endl << "gl_top_1_05 = ";
	for (uint i = 0; i < m_vCaffeNet.size(); i++) { oss << left << setw(9) << m_vCaffeNet[i]->blob_by_name("top_gl_1_05")->cpu_data()[0]; }
	oss << endl << "gl_top_1_10 = ";
	for (uint i = 0; i < m_vCaffeNet.size(); i++) { oss << left << setw(9) << m_vCaffeNet[i]->blob_by_name("top_gl_1_10")->cpu_data()[0]; }
	oss << endl;
#elif USE_CAFFE2
	// TODO
#endif
	return oss.str();
}

vector<string> WeichiCNNTrainNet::getGLTrainingInfo(int gpu_id)
{
	vector<string> vInfo;
#ifdef USE_CAFFE
	const float *fGLOutput = m_vCaffeNet[gpu_id]->blob_by_name("flatten_gl")->cpu_data();
	const float *fLabel = m_vCaffeNet[gpu_id]->blob_by_name("label_gl")->cpu_data();
	const float *fDataMove = m_vCaffeNet[gpu_id]->blob_by_name("dummy_data")->cpu_data();
	const float *fLabelMove = m_vCaffeNet[gpu_id]->blob_by_name("dummy_label")->cpu_data();

	for (int batch = 0; batch < m_batch_size; batch++) {
		const uint label_pos = static_cast<uint>(fLabel[batch]);
		const float *fExponential = fGLOutput + batch * m_vn_label_size;
		const float fLabel_exponential = fExponential[label_pos];

		uint rank = 1;
		for (int i = 0; i < m_vn_label_size; i++) {
			if (i == label_pos) { continue; }
			if (fExponential[i] >= fLabel_exponential) { rank++; }
		}

		if (fDataMove[batch] != fLabelMove[batch]) {
			CERR() << "[Error] DataMove and LabelMove not match." << endl;
			exit(0);
		}

		ostringstream oss;
		oss << setw(6) << label_pos << setw(5) << rank << setw(8) << ToString(fDataMove[batch]);
		vInfo.push_back(oss.str());
	}
#elif USE_CAFFE2
	// TODO
#endif
	return vInfo;
} 
