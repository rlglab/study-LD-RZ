#ifndef WEICHICNNNET_H
#define WEICHICNNNET_H

#include "BaseCNNNet.h"
#include "CandidateEntry.h"
#include "WeichiDynamicKomi.h"

class WeichiCNNNet : public BaseCNNNet
{
private:
	// output
	float* m_slOutput;
	float* m_vnOutput;
	float* m_bvOutput;
	float* m_glOutput;
	float* m_eyeOutput;
	float* m_connectOutput;	

	// net info
	bool* m_bIsLegal;
	int m_label_size;
	int m_sl_label_size;
	int m_vn_label_size;
	int m_bv_label_size;
	int m_gl_label_size;
	int m_eye_label_size;
	int m_connect_label_size;

public:
	WeichiCNNNet(CNNNetParam param)
		: BaseCNNNet(param), m_slOutput(nullptr)
		, m_vnOutput(nullptr), m_bvOutput(nullptr)
		, m_eyeOutput(nullptr), m_connectOutput(nullptr)
		, m_glOutput(nullptr), m_bIsLegal(nullptr), m_label_size(-1)
		, m_sl_label_size(-1), m_vn_label_size(-1), m_bv_label_size(-1), m_eye_label_size(-1), m_connect_label_size(-1), m_gl_label_size(-1)
	{
	
	}

	~WeichiCNNNet()
	{
		if (m_slOutput != nullptr) { delete[] m_slOutput; }
		if (m_vnOutput != nullptr) { delete[] m_vnOutput; }
		if (m_bvOutput != nullptr) { delete[] m_bvOutput; }
		if (m_glOutput != nullptr) { delete[] m_glOutput; }
		if (m_eyeOutput != nullptr) { delete[] m_eyeOutput; }
		if (m_connectOutput != nullptr) { delete[] m_connectOutput; }
		if (m_bIsLegal != nullptr) { delete[] m_bIsLegal; }
	}

	inline bool hasSLOutput() const { return (m_slOutput != nullptr); }
	inline bool hasVNOutput() const { return (m_vnOutput != nullptr); }
	inline bool hasBVOutput() const { return (m_bvOutput != nullptr); }
	inline bool hasGLOutput() const { return (m_glOutput != nullptr); }
	inline bool hasEyeOutput() const { return (m_eyeOutput!=nullptr); }
	inline bool hasConnectOutput() const { return (m_connectOutput!=nullptr) ; } 

	inline int getLabelSize() const { return m_label_size; }
	inline int getSLLabelSize() const { return m_sl_label_size; }
	inline int getVNLabelSize() const { return m_vn_label_size; }
	inline int getBVLabelSize() const { return m_bv_label_size; }
	inline int getGLLabelSize() const { return m_gl_label_size; }
	inline int getConnectLabelSize() const { return m_connect_label_size; }
	inline int getEyeLabelSize() const { return m_eye_label_size; }

	void loadNetWork(bool bDisplayErrorMessage = false);
	void push_back(const float* data, Color turnColor, const WeichiBitBoard& bmLegal, SymmetryType type = SYM_NORMAL);
	void set_data(int batch_pos, const float* data, Color turnColor, const WeichiBitBoard& bmLegal, SymmetryType type = SYM_NORMAL);
	void forward(bool bForwardFirstBatch = true);

	// get result and node info
	WeichiMove getBestCNNMove(uint batch);
	WeichiMove getSoftMaxMove(uint batch, bool bWithIllegal = false);
	vector<CandidateEntry> getSLCandidates(uint batch, int size = MAX_NUM_GRIDS);
	float getSLResult(uint batch, uint position);
	float getVNResult(uint batch, double dKomi);
	vector<float> getVNResult(uint batch);
	float getGLResult(uint batch, uint length);
	vector<float> getGLResult(uint batch);
	Vector<float, MAX_NUM_GRIDS> getBVResult(uint batch);
	float getBVResult(uint batch, uint position);
	Vector<float, MAX_NUM_GRIDS> getEyeResult(uint batch);
	float getEyeResult(uint batch, uint position);
	Vector<float, MAX_NUM_GRIDS> getConnectResult(uint batch);
	float getConnectResult(uint batch, uint position);

	bool isLegal(uint batch, uint position);
	Color getColor(uint batch);
	string getNetInfomation(string sPrefix = "");

	static int getVNKomiIndex(double dKomi, int vn_label_size);

private:
	void setBatchData(int batch_pos, const float* data, Color turnColor, const WeichiBitBoard& bmLegal, SymmetryType type = SYM_NORMAL);
};

#endif