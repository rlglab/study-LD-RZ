#ifndef WEICHICNNTRAINNET_H
#define WEICHICNNTRAINNET_H

#include "Color.h"
#include "BaseCNNNet.h"
#include "CandidateEntry.h"

#ifdef USE_CAFFE
#include <caffe/layers/memory_data_layer.hpp>
#elif USE_CAFFE2
#include <boost/filesystem.hpp>
#include "caffe2/core/flags.h"
#include "caffe2/core/init.h"
#include "caffe2/core/net.h"
#include "caffe2/core/predictor.h"
#include "caffe2/core/common.h"
#include "caffe2/utils/proto_utils.h"
#include "caffe2/core/workspace.h"
#include "caffe2/utils/math.h"
#include "caffe2/core/context_gpu.h"
#include "caffe2/core/tensor.h"
#include "caffe2/core/common_gpu.h"
#include "caffe2/core/blob.h"

#endif

class WeichiCNNTrainNet: public BaseCNNNet
{
private:
	string m_sSolver;
	int m_gpu_size;
	int m_label_size;
	int m_sl_label_size;
	int m_bv_label_size;
	int m_vn_label_size;
	int m_gl_label_size;
	int m_sl_distribution_label_size;
	bool m_bRandomInitWeight;
	vector<int> m_vGpu;
	
	// input
	float* m_label;
	int* m_sl_label;
	float* m_bv_label;
	float* m_vn_label;
	float* m_gl_label;
	float* m_sl_distribution_label;
	float* m_moveNumber;
	SymmetryType* m_symmetric;

#ifdef USE_CAFFE2
	caffe2::NetDef m_trainDef;
	map<string, caffe2::DeviceOption> m_BlobOptionMap;
#endif
#ifdef USE_CAFFE
	MemoryDataLayerSp m_data_input_layer;
	MemoryDataLayerSp m_label_input_layer;
	caffe::SolverParameter m_solverParam;
	boost::shared_ptr< caffe::Solver<float> > m_solver;
	boost::shared_ptr< caffe::P2PSync<float> > m_sync;
	vector< boost::shared_ptr< caffe::P2PSync<float> > > m_vSync;
	vector< boost::shared_ptr< Net<float> > > m_vCaffeNet;
#endif

public:
	WeichiCNNTrainNet(CNNNetParam param, string solover)
		: BaseCNNNet(param), m_sSolver(solover)
		, m_gpu_size(-1), m_label_size(-1), m_sl_label_size(-1), m_bv_label_size(-1), m_vn_label_size(-1), m_gl_label_size(-1)
		, m_sl_distribution_label_size(-1), m_moveNumber(nullptr), m_label(nullptr), m_symmetric(nullptr)
		, m_bRandomInitWeight(false),m_vn_label(nullptr), m_sl_label(nullptr), m_bv_label(nullptr), m_gl_label(nullptr)
		, m_sl_distribution_label(nullptr)
	{
	}

	~WeichiCNNTrainNet()
	{
#ifdef USE_CAFFE
		for (int i = 1; i < m_vSync.size(); i++) { m_vSync[i]->StopInternalThread(); }
#endif
		if (m_label != nullptr) { delete[] m_label; }
		if (m_moveNumber != nullptr) { delete[] m_moveNumber; }
		if (m_symmetric != nullptr) { delete[] m_symmetric; }
	}

	void init( bool bDisplayErrorMessage=false );
	void update(bool bEvaluateFirstBatch);
	void push_back( bool bEvaluateFirstBatch, const float* data, Color turnColor, float* fLabel, float fMoveNumber, SymmetryType type=SYM_NORMAL );
	void set_data( const float* data, Color turnColor, float* fLabel, float fMoveNumber, int batch_pos, SymmetryType type=SYM_NORMAL );
	void saveNetWork(string sSnapshot);
	string getCaffeLossInfo();
	string getNetInfomation(string sPrefix = "");
	string getTrainingInfo();

	inline bool isFull() { return m_current_batch_size==m_batch_size*WeichiConfigure::dcnn_train_gpu_list.length(); }
	inline int getLabelSize() { return m_label_size; }
	inline int getIteration() {
#ifdef USE_CAFFE
		return m_solver->iter();
#elif USE_CAFFE2
		return m_caffe2_workspace.GetBlob("gpu_" + to_string(m_vGpu[0]) + "/iter")->Get<caffe2::TensorCPU>().data<long>()[0];
#endif
		return 0;
	}
	inline bool hasNFSPath() { return (WeichiConfigure::NFS_path != ""); }
	inline bool hasSLOutput() const { return (m_sl_label_size != -1); }
	inline bool hasVNOutput() const { return (m_vn_label_size != -1); }
	inline bool hasBVOutput() const { return (m_bv_label_size != -1); }
	inline bool hasGLOutput() const { return (m_gl_label_size != -1); }
	inline bool hasSLDistribution() const {
#ifdef USE_CAFFE
		return m_caffe_net->has_blob("label_sl_distribution");
#elif USE_CAFFE2
		return (m_sl_distribution_label_size != -1);
#endif
		return false;
	}
	inline int getSLLabelSize() const { return m_sl_label_size; }
	inline int getVNLabelSize() const { return m_vn_label_size; }
	inline int getBVLabelSize() const { return m_bv_label_size; }
	inline int getGLLabelSize() const { return m_gl_label_size; }
	inline SymmetryType getRotateType( int batch ) { return m_symmetric[batch]; }
	inline float* getLabel(int batch) { return m_label + batch*m_label_size; }
	void setBaseLearningRate(float lr);

private:
#ifdef USE_CAFFE2
	void addDistributedArg(const string& path, const string& prefix, caffe2::NetDef& net);
	int checkIteration(caffe2::NetDef& initDef);
#endif

	void inputDataIntoLayers(bool bEvaluateFirstBatch);
	template<typename T>
	void inputDataIntoLayers(string blobName, T* data)
	{
#ifdef USE_CAFFE2
		if (!m_caffe2_workspace.HasBlob(blobName)) {
			CERR() << "Blob: " << blobName << " not found." << endl;
			exit(0);
		}
		auto tensorData = m_caffe2_workspace.GetBlob(blobName)->template GetMutable<caffe2::TensorCUDA>();
		caffe2::CUDAContext contextLabel(m_BlobOptionMap[blobName]);
		contextLabel.SwitchToDevice();
		contextLabel.CopyBytes< caffe2::CPUContext, caffe2::CUDAContext>(tensorData->size() * sizeof(T), static_cast<void*>(data), tensorData->raw_mutable_data());
		contextLabel.FinishDeviceComputation();
#endif
	}
	void setDataAtSpecificBatchIndex(const float* data, int batch_pos, SymmetryType type = SYM_NORMAL);
	void setLabelAtSpecificBatchIndex(const float* label, int batch_pos, SymmetryType type = SYM_NORMAL);

	string getSLTrainingInfo();
	vector<string> getSLTrainingInfo(int gpu_id);
	string getBVTrainingInfo();
	vector<string> getBVTrainingInfo(int gpu_id, double& dLoss);
	string getVNTrainingInfo();
	vector<string> getVNTrainingInfo(int gpu_id, double* dMSE);
	string getGLTrainingInfo();
	vector<string> getGLTrainingInfo(int gpu_id);
};

#endif 
