#include "WeichiCNNNet.h"
#include "StaticBoard.h"
#include <float.h>

void WeichiCNNNet::loadNetWork(bool bDisplayErrorMessage/*=false*/)
{
	BaseCNNNet::loadNetWork(bDisplayErrorMessage);

	delete[] m_color;
	delete[] m_reverseSymmetric;
	delete[] m_input;
	delete[] m_dummy;
	m_color = new Color[2 * m_batch_size];
	m_reverseSymmetric = new SymmetryType[2 * m_batch_size];
	m_input = new float[2 * m_batch_size * m_input_data_size];
	m_dummy = new float[2 * m_batch_size * m_input_data_size];

	std::fill(m_color, m_color + 2 * m_batch_size, COLOR_NONE);
	std::fill(m_reverseSymmetric, m_reverseSymmetric + 2 * m_batch_size, SYM_NORMAL);
	std::fill(m_input, m_input + 2 * m_batch_size * m_input_data_size, 0.0f);
	std::fill(m_dummy, m_dummy + 2 * m_batch_size * m_input_data_size, 0.0f);

#ifdef USE_CAFFE
	m_label_size = 0;
	if (m_caffe_net->has_blob(WeichiConfigure::SL_OUTPUT_NAME)) {
		m_sl_label_size = m_caffe_net->blob_by_name(WeichiConfigure::SL_OUTPUT_NAME)->channels();
		m_label_size += m_sl_label_size;
		m_slOutput = new float[2 * m_batch_size * (WeichiConfigure::TotalGrids + 1)];
		m_bIsLegal = new bool[2 * m_batch_size * (WeichiConfigure::TotalGrids + 1)];
	}
	if (m_caffe_net->has_blob(WeichiConfigure::BV_OUTPUT_NAME)) {
		m_bv_label_size = m_caffe_net->blob_by_name(WeichiConfigure::BV_OUTPUT_NAME)->channels();
		m_label_size += m_bv_label_size;
		m_bvOutput = new float[2 * m_batch_size * MAX_NUM_GRIDS];
	}
	if (m_caffe_net->has_blob(WeichiConfigure::VN_OUTPUT_NAME)) {
		m_vn_label_size = m_caffe_net->blob_by_name(WeichiConfigure::VN_OUTPUT_NAME)->channels();
		m_label_size += m_vn_label_size;
		m_vnOutput = new float[2 * m_batch_size * m_vn_label_size];
	}
	if (m_caffe_net->has_blob(WeichiConfigure::GL_OUTPUT_NAME)) {
		m_gl_label_size = m_caffe_net->blob_by_name(WeichiConfigure::GL_OUTPUT_NAME)->channels();
		m_label_size += m_gl_label_size;
		m_gl_label_size = m_vn_label_size;
		m_glOutput = new float[2 * m_batch_size * m_gl_label_size];
	}
#elif USE_CAFFE2
	// Get net blob dim
	m_inputCPU.ShareExternalPointer(m_input);
	m_inputCUDA.CopyFrom(m_inputCPU);
	m_caffe2_workspace.GetBlob("data")->template GetMutable<caffe2::TensorCUDA>()->ShareData(m_inputCUDA);
	m_caffe2_workspace.RunNet(m_caffe2_net.name());

	m_label_size = 0;
	if (m_caffe2_workspace.HasBlob(WeichiConfigure::SL_OUTPUT_NAME)) {
		m_sl_label_size = m_caffe2_workspace.GetBlob(WeichiConfigure::SL_OUTPUT_NAME)->template GetMutable<caffe2::TensorCUDA>()->size() / m_batch_size;
		m_label_size += m_sl_label_size;
		m_slOutput = new float[2 * m_batch_size * (WeichiConfigure::TotalGrids + 1)];
		m_bIsLegal = new bool[2 * m_batch_size * (WeichiConfigure::TotalGrids + 1)];
	}
	if (m_caffe2_workspace.HasBlob(WeichiConfigure::BV_OUTPUT_NAME)) {
		m_bv_label_size = m_caffe2_workspace.GetBlob(WeichiConfigure::BV_OUTPUT_NAME)->template GetMutable<caffe2::TensorCUDA>()->size() / m_batch_size;
		m_label_size += m_bv_label_size;
		m_bvOutput = new float[2 * m_batch_size * MAX_NUM_GRIDS];
	}
	if (m_caffe2_workspace.HasBlob(WeichiConfigure::VN_OUTPUT_NAME)) {
		m_vn_label_size = m_caffe2_workspace.GetBlob(WeichiConfigure::VN_OUTPUT_NAME)->template GetMutable<caffe2::TensorCUDA>()->size() / m_batch_size;
		m_label_size += m_vn_label_size;
		m_vnOutput = new float[2 * m_batch_size * m_vn_label_size];
	}
	if (m_caffe2_workspace.HasBlob(WeichiConfigure::GL_OUTPUT_NAME)) {
		m_gl_label_size = m_caffe2_workspace.GetBlob(WeichiConfigure::GL_OUTPUT_NAME)->template GetMutable<caffe2::TensorCUDA>()->size() / m_batch_size;
		m_label_size += m_gl_label_size;
		m_gl_label_size = m_vn_label_size;
		m_glOutput = new float[2 * m_batch_size * m_gl_label_size];
	}
	if (m_caffe2_workspace.HasBlob(WeichiConfigure::EYE_OUTPUT_NAME)) {
		m_eye_label_size = m_caffe2_workspace.GetBlob(WeichiConfigure::EYE_OUTPUT_NAME)->template GetMutable<caffe2::TensorCUDA>()->size() / m_batch_size;
		m_label_size += m_eye_label_size;
		m_eyeOutput = new float[2 * m_batch_size * MAX_NUM_GRIDS];
	}
	if (m_caffe2_workspace.HasBlob(WeichiConfigure::DRAGON_CONNECT_OUTPUT_NAME)) {
		m_connect_label_size = m_caffe2_workspace.GetBlob(WeichiConfigure::DRAGON_CONNECT_OUTPUT_NAME)->template GetMutable<caffe2::TensorCUDA>()->size() / m_batch_size;
		m_label_size += m_connect_label_size;
		m_connectOutput = new float[2 * m_batch_size * MAX_NUM_GRIDS];
	}
#endif

	if (bDisplayErrorMessage) { CERR() << getNetInfomation() << endl; }
}

void WeichiCNNNet::push_back(const float* data, Color turnColor, const WeichiBitBoard& bmLegal, SymmetryType type/*=SYM_NORMAL*/)
{
	if (getRandomNum() == 0) {
		if (WeichiConfigure::problem_rotation != 0) {
			setBatchData(m_current_batch_size, data, turnColor, bmLegal, m_vSymmetryEntry[WeichiConfigure::problem_rotation]);
			m_current_batch_size++;
			return;
		}
		setBatchData(m_current_batch_size, data, turnColor, bmLegal, type);
		m_current_batch_size++;
	} else {
		for (int i = 0; i < getRandomNum(); i++) {
			int index = Random::nextInt(SYMMETRY_SIZE - i);
			setBatchData(m_current_batch_size, data, turnColor, bmLegal, m_vSymmetryEntry[index]);
			m_vSymmetryEntry.swap(index, SYMMETRY_SIZE - 1 - i);
			m_current_batch_size++;
		}
	}
}

void WeichiCNNNet::set_data(int batch_pos, const float* data, Color turnColor, const WeichiBitBoard& bmLegal, SymmetryType type/* = SYM_NORMAL*/)
{
	// if call set_data, the random number must be 0 or 1
	if (getRandomNum() == 0) {
		setBatchData(batch_pos, data, turnColor, bmLegal, type);
	} else if (getRandomNum() == 1) {
		setBatchData(batch_pos, data, turnColor, bmLegal, m_vSymmetryEntry[Random::nextInt(SYMMETRY_SIZE)]);
	} else {
		CERR() << "The random number must be 0 or 1 when using set_data function" << endl;
	}

	// if call set_data, we can't calculate the real number of forwarding data, so let m_current_batch_size = m_batch_size
	m_current_batch_size = m_batch_size;
}

void WeichiCNNNet::forward(bool bForwardFirstBatch/* = true*/)
{
	int start_batch = (bForwardFirstBatch ? 0 : m_batch_size);
	int end_batch = start_batch + m_batch_size;
	float* fInput = m_input + start_batch * m_input_data_size;

#ifdef USE_CAFFE
	m_data_input_layer->Reset(fInput, m_dummy, m_batch_size);
	m_caffe_net->Forward();
#elif USE_CAFFE2
	m_inputCPU.ShareExternalPointer(fInput);
	m_inputCUDA.CopyFrom(m_inputCPU);
	m_caffe2_workspace.GetBlob("data")->template GetMutable<caffe2::TensorCUDA>()->ShareData(m_inputCUDA);
	m_caffe2_workspace.RunNet(m_caffe2_net.name());
#endif

	// SL output
	if (hasSLOutput()) {
		const float* fSLGPUOutput = nullptr;
#ifdef USE_CAFFE
		fSLGPUOutput = m_caffe_net->blob_by_name(WeichiConfigure::SL_OUTPUT_NAME)->cpu_data();
#elif USE_CAFFE2
		m_outputSLCPU.CopyFrom(m_caffe2_workspace.GetBlob(WeichiConfigure::SL_OUTPUT_NAME)->Get<caffe2::TensorCUDA>());
		fSLGPUOutput = m_outputSLCPU.data<float>();
#endif
		// last output is softmax
		if (!WeichiConfigure::cnn_policy_output_power_softmax) {
			for (int batch = start_batch; batch < end_batch; batch++) {
				int cpu_shift_pos = batch * (WeichiConfigure::TotalGrids + 1);
				int gpu_shift_pos = (batch - start_batch) * (WeichiConfigure::TotalGrids + 1);
				for (uint pos = 0; pos < WeichiConfigure::TotalGrids + 1; pos++) {
					m_slOutput[cpu_shift_pos + pos] = fSLGPUOutput[gpu_shift_pos + WeichiMove::rotateCompactPosition(pos, m_reverseSymmetric[batch])];
				}
			}
		} else {
			// last output is the power of softmax (for only conv + one pass method)
			for (int batch = start_batch; batch < end_batch; batch++) {
				int cpu_shift_pos = batch * (WeichiConfigure::TotalGrids + 1);
				int gpu_shift_pos = (batch - start_batch) * WeichiConfigure::TotalGrids;
				double dMax = -DBL_MAX;
				for (uint pos = 0; pos < WeichiConfigure::TotalGrids; pos++) {
					double dScore = fSLGPUOutput[gpu_shift_pos + WeichiMove::rotateCompactPosition(pos, m_reverseSymmetric[batch])];
					m_slOutput[cpu_shift_pos + pos] = dScore;
					dMax = (dScore > dMax) ? dScore : dMax;
				}
				double dExpSum = 0.0f;
				dMax = (0.0f > dMax) ? 0.0f : dMax;
				m_slOutput[cpu_shift_pos + WeichiConfigure::TotalGrids] = 0.0f;	// init for pass move
				for (uint pos = 0; pos < WeichiConfigure::TotalGrids + 1; pos++) {
					m_slOutput[cpu_shift_pos + pos] = exp(m_slOutput[cpu_shift_pos + pos] - dMax);
					dExpSum += m_slOutput[cpu_shift_pos + pos];
				}
				for (uint pos = 0; pos < WeichiConfigure::TotalGrids + 1; pos++) {
					m_slOutput[cpu_shift_pos + pos] /= dExpSum;
				}
			}
		}
		
	}

	// BV output
	if (hasBVOutput()) {
		const float* fBVGPUOutput = nullptr;
#ifdef USE_CAFFE
		fBVGPUOutput = m_caffe_net->blob_by_name(WeichiConfigure::BV_OUTPUT_NAME)->cpu_data();
#elif USE_CAFFE2
		m_outputBVCPU.CopyFrom(m_caffe2_workspace.GetBlob(WeichiConfigure::BV_OUTPUT_NAME)->Get<caffe2::TensorCUDA>());
		fBVGPUOutput = m_outputBVCPU.data<float>();
#endif
		for (int batch = start_batch; batch < end_batch; batch++) {
			int cpu_shift_pos = batch * MAX_NUM_GRIDS;
			int gpu_shift_pos = (batch - start_batch) * WeichiConfigure::TotalGrids;
			for (uint pos = 0; pos < WeichiConfigure::TotalGrids; pos++) {
				uint softPos = WeichiMove::toSoftPosition(pos);
				m_bvOutput[cpu_shift_pos + softPos] = fBVGPUOutput[gpu_shift_pos + WeichiMove::rotateCompactPosition(pos, m_reverseSymmetric[batch])];
				if (m_color[batch] == COLOR_WHITE) { m_bvOutput[cpu_shift_pos + softPos] = 1 - m_bvOutput[cpu_shift_pos + softPos]; }
				m_bvOutput[cpu_shift_pos + softPos] = m_bvOutput[cpu_shift_pos + softPos] * 2 - 1;
			}
		}
	}

	// VN output
	if (hasVNOutput()) {
		const float* fVNGPUOutput = nullptr;
#ifdef USE_CAFFE
		fVNGPUOutput = m_caffe_net->blob_by_name(WeichiConfigure::VN_OUTPUT_NAME)->cpu_data();
#elif USE_CAFFE2
		m_outputVNCPU.CopyFrom(m_caffe2_workspace.GetBlob(WeichiConfigure::VN_OUTPUT_NAME)->Get<caffe2::TensorCUDA>());
		fVNGPUOutput = m_outputVNCPU.data<float>();
#endif
		for (int batch = start_batch; batch < end_batch; batch++) {
			int cpu_shift_pos = batch * m_vn_label_size;
			int gpu_shift_pos = (batch - start_batch) * m_vn_label_size;
			for (int index = 0; index < m_vn_label_size; index++) {
				//m_vnOutput[cpu_shift_pos + index] = (fVNGPUOutput[gpu_shift_pos + index] + 1) / 2;
				m_vnOutput[cpu_shift_pos + index] = fVNGPUOutput[gpu_shift_pos + index];
				if (WeichiConfigure::vn_turn_opposite && m_color[batch] == COLOR_WHITE) {
					m_vnOutput[cpu_shift_pos + index] = -m_vnOutput[cpu_shift_pos + index];
				}
			}
		}
	}

	// GL output
	if (hasGLOutput()) {
		const float* fGLGPUOutput = nullptr;
#ifdef USE_CAFFE
		fGLGPUOutput = m_caffe_net->blob_by_name(WeichiConfigure::GL_OUTPUT_NAME)->cpu_data();
#elif USE_CAFFE2
		m_outputGLCPU.CopyFrom(m_caffe2_workspace.GetBlob(WeichiConfigure::GL_OUTPUT_NAME)->Get<caffe2::TensorCUDA>());
		fGLGPUOutput = m_outputGLCPU.data<float>();
#endif
		for (int batch = start_batch; batch < end_batch; batch++) {
			int cpu_shift_pos = batch * m_gl_label_size;
			int gpu_shift_pos = (batch - start_batch) * m_gl_label_size;
			for (uint length = 0; length < m_gl_label_size; length++) {
				m_glOutput[cpu_shift_pos + length] = fGLGPUOutput[gpu_shift_pos + length];
			}
		}
	}

	if (hasEyeOutput()) {
#ifdef USE_CAFFE2
		float fOutput[MAX_NUM_GRIDS];
		caffe2::TensorCPU temp;
		temp.CopyFrom(m_caffe2_workspace.GetBlob(WeichiConfigure::EYE_OUTPUT_NAME)->Get<caffe2::TensorCUDA>());
		const float* fEyeOutput = temp.data<float>();

		for(int batch=0; batch<m_current_batch_size; batch++) {
			for( uint pos=0; pos<WeichiConfigure::TotalGrids; pos++ ) {
				fOutput[pos] = fEyeOutput[batch*WeichiConfigure::TotalGrids+pos];
			}
			rotateOneFeature(fOutput,m_reverseSymmetric[batch]);
			float* fEye = m_eyeOutput + batch*MAX_NUM_GRIDS;
			for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
				float fResult = fOutput[WeichiMove(*it).getCompactPosition()] * 2 - 1;
				if (m_color[batch]==COLOR_WHITE) fResult = - fResult;
				fEye[*it] = fResult;
			}
		}
#endif
	}

	if (hasConnectOutput()) {
#ifdef USE_CAFFE2
		float fOutput[MAX_NUM_GRIDS];
		caffe2::TensorCPU temp;
		temp.CopyFrom(m_caffe2_workspace.GetBlob(WeichiConfigure::DRAGON_CONNECT_OUTPUT_NAME)->Get<caffe2::TensorCUDA>());
		const float* fConnectOutput = temp.data<float>();

		for(int batch=0; batch<m_current_batch_size; batch++) {
			for( uint pos=0; pos<WeichiConfigure::TotalGrids; pos++ ) {
				fOutput[pos] = fConnectOutput[batch*WeichiConfigure::TotalGrids+pos];
			}
			rotateOneFeature(fOutput,m_reverseSymmetric[batch]);
			float* fConnect = m_connectOutput + batch*MAX_NUM_GRIDS;
			for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
				float fResult = fOutput[WeichiMove(*it).getCompactPosition()] * 2 - 1;
				if (m_color[batch]==COLOR_WHITE) fResult = - fResult;
				fConnect[*it] = fResult;
			}
		}
#endif
	}

	resetCurrentBatchSize();
}

WeichiMove WeichiCNNNet::getBestCNNMove(uint batch)
{
	double dMaxScore = 0.0f;
	int bestPos = PASS_MOVE.getPosition();
	for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
		if (!isLegal(batch, *it)) { continue; }

		double dScore = getSLResult(batch, *it);
		if (dScore>dMaxScore) {
			bestPos = *it;
			dMaxScore = dScore;
		}
	}

	return WeichiMove(m_color[batch], bestPos);
}

WeichiMove WeichiCNNNet::getSoftMaxMove(uint batch, bool bWithIllegal/*=false*/)
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
	for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
		if (!bWithIllegal && !isLegal(batch, *it)) { continue; }

		double dProb = getSLResult(batch, *it) * 100;
		dSum += dProb;
		double dRand = Random::nextReal(dSum);
		if (dRand<dProb) { position = *it; }
	}

	return WeichiMove(m_color[batch], position);
}

vector<CandidateEntry> WeichiCNNNet::getSLCandidates(uint batch, int size)
{
	vector<CandidateEntry> vCandidates;
	for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
		if (!isLegal(batch, *it)) { continue; }
		vCandidates.push_back(CandidateEntry(*it, getSLResult(batch, *it)));
	}

	// add pass
	if (isLegal(batch, PASS_MOVE.getPosition()) || vCandidates.size() == 0) {
		vCandidates.push_back(CandidateEntry(PASS_MOVE.getPosition(), getSLResult(batch, PASS_MOVE.getPosition())));
	}
	sort(vCandidates.begin(), vCandidates.end());

	if (size<vCandidates.size()) { vCandidates.resize(size); }
	return vCandidates;
}

float WeichiCNNNet::getSLResult(uint batch, uint position)
{
	float dSLScore = 0;
	int randomNum = (getRandomNum() == 0) ? 1 : getRandomNum();
	for (int iRandom = 0; iRandom<randomNum; iRandom++) {
		int index = (batch * randomNum) + iRandom;
		dSLScore += m_slOutput[index * (WeichiConfigure::TotalGrids + 1) + WeichiMove::toCompactPosition(position)];
	}
	return dSLScore / randomNum;
}

vector<float> WeichiCNNNet::getVNResult(uint batch)
{
	if (WeichiConfigure::dcnn_use_ftl) {
		vector<float> vVNScore;
		for (int i = 0; i < getVNLabelSize(); ++i) {
			vVNScore.push_back(getVNResult(batch, i));
		}
		return vVNScore;
	} else {
		float fCenterKomi = (WeichiConfigure::BoardSize == 7 ? 0 : 7.5);	// 7x7 special case
		double dMinKomi = fCenterKomi - (getVNLabelSize() - 1) / 2;
		double dMaxKomi = fCenterKomi + (getVNLabelSize() - 1) / 2;

		vector<float> vVNScore;
		for (double dKomi = dMinKomi; dKomi <= dMaxKomi; dKomi++) {
			vVNScore.push_back(getVNResult(batch, dKomi));
		}

		return vVNScore;
	}
}

float WeichiCNNNet::getVNResult(uint batch, double dKomi)
{
	float fVNScore = 0.0f;
	int komi_index = getVNKomiIndex(dKomi, getVNLabelSize());
	int randomNum = (getRandomNum() == 0) ? 1 : getRandomNum();
	for (int iRandom = 0; iRandom < randomNum; iRandom++) {
		int index = (batch * randomNum) + iRandom;
		fVNScore += m_vnOutput[index * m_vn_label_size + komi_index];
	}
	fVNScore /= randomNum;

	return fVNScore;
}

float WeichiCNNNet::getGLResult(uint batch, uint length) {
	float fGLScore = 0.0f;
	int randomNum = (getRandomNum() == 0) ? 1 : getRandomNum();
	for (int iRandom = 0; iRandom < randomNum; iRandom++) {
		int index = (batch * randomNum) + iRandom;
		fGLScore += m_glOutput[index * m_gl_label_size + length];
	}
	fGLScore /= randomNum;

	return fGLScore;
}

vector<float> WeichiCNNNet::getGLResult(uint batch)
{
	vector<float> vGLResult;
	for (int i = 0; i < m_gl_label_size; ++i) {
		vGLResult.push_back(getGLResult(batch, i));
	}

	return vGLResult;
}

Vector<float, MAX_NUM_GRIDS> WeichiCNNNet::getBVResult(uint batch)
{
	Vector<float, MAX_NUM_GRIDS> vBVScore;
	vBVScore.setAllAs(0.0f, MAX_NUM_GRIDS);
	for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
		vBVScore[*it] = getBVResult(batch, *it);
	}
	return vBVScore;
}

float WeichiCNNNet::getBVResult(uint batch, uint position)
{
	float fBVScore = 0.0f;
	int randomNum = (getRandomNum() == 0) ? 1 : getRandomNum();
	for (int iRandom = 0; iRandom < randomNum; iRandom++) {
		int index = (batch * randomNum) + iRandom;
		fBVScore += m_bvOutput[index * MAX_NUM_GRIDS + position];
	}
	fBVScore /= randomNum;

	return fBVScore;
}

Vector<float, MAX_NUM_GRIDS>  WeichiCNNNet::getEyeResult(uint batch) {
	Vector<float, MAX_NUM_GRIDS> vEyeScore;
	vEyeScore.setAllAs(0.0f, MAX_NUM_GRIDS);
	for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
		vEyeScore[*it] = getEyeResult(batch, *it);
	}
	return vEyeScore;
}

float WeichiCNNNet::getEyeResult(uint batch, uint position){
	float fEyeScore = 0.0f;
	int randomNum = (getRandomNum() == 0) ? 1 : getRandomNum();
	for (int iRandom = 0; iRandom < randomNum; iRandom++) {
		int index = (batch * randomNum) + iRandom;
		fEyeScore += m_eyeOutput[index * MAX_NUM_GRIDS + position];
	}
	fEyeScore /= randomNum;

	return fEyeScore;
}

Vector<float, MAX_NUM_GRIDS>  WeichiCNNNet::getConnectResult(uint batch) {
	Vector<float, MAX_NUM_GRIDS> vConnectScore;
	vConnectScore.setAllAs(0.0f, MAX_NUM_GRIDS);
	for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
		vConnectScore[*it] = getConnectResult(batch, *it);
	}
	return vConnectScore;
}

float WeichiCNNNet::getConnectResult(uint batch, uint position){
	float fConnectScore = 0.0f;
	int randomNum = (getRandomNum() == 0) ? 1 : getRandomNum();
	for (int iRandom = 0; iRandom < randomNum; iRandom++) {
		int index = (batch * randomNum) + iRandom;
		fConnectScore += m_connectOutput[index * MAX_NUM_GRIDS + position];
	}
	fConnectScore /= randomNum;

	return fConnectScore;
}

bool WeichiCNNNet::isLegal(uint batch, uint position)
{
	int randomNum = (getRandomNum() == 0) ? 1 : getRandomNum();
	return m_bIsLegal[batch * randomNum * (WeichiConfigure::TotalGrids + 1) + WeichiMove::toCompactPosition(position)];
}

Color WeichiCNNNet::getColor(uint batch)
{
	int randomNum = (getRandomNum() == 0) ? 1 : getRandomNum();
	return m_color[batch * randomNum];
}

string WeichiCNNNet::getNetInfomation(string sPrefix/* = ""*/)
{
	ostringstream oss;
	oss << sPrefix << "[train net] " << m_param.m_sPrototxt << endl;
	oss << sPrefix << "[train model] " << m_param.m_sCaffeModel << endl;
	oss << sPrefix << "[batch size] " << getBatchSize() << endl;
	oss << sPrefix << "[channel size] " << getChannelSize() << endl;
	oss << sPrefix << "[net type] " << getWeichiCNNNetTypeString(getCNNNetType()) << endl;
	oss << sPrefix << "[feature type] " << getWeichiCNNFeatureTypeString(getCNNFeatureType()) << endl;
	oss << sPrefix << "[random rotate number] " << getRandomNum() << endl;
	oss << sPrefix << "[gpu_list] " << WeichiConfigure::dcnn_train_gpu_list << endl;
	oss << sPrefix << "[label size] " << getLabelSize() << endl;
	if (hasSLOutput()) { oss << sPrefix << "\t[SL label size] " << getSLLabelSize() << endl; }
	if (hasBVOutput()) { oss << sPrefix << "\t[BV label size] " << getBVLabelSize() << endl; }
	if (hasVNOutput()) { oss << sPrefix << "\t[VN label size] " << getVNLabelSize() << endl; }
	if (hasGLOutput()) { oss << sPrefix << "\t[GL label size] " << getGLLabelSize() << endl; }
	if (hasConnectOutput()) { oss << sPrefix << "\t[Connect label size] " << getConnectLabelSize() << endl; }
	if (hasEyeOutput()) { oss << sPrefix << "\t[Eye label size] " << getEyeLabelSize() << endl; }

	return oss.str();
}

int WeichiCNNNet::getVNKomiIndex(double dKomi, int vn_label_size)
{
	if (WeichiConfigure::dcnn_use_ftl) {
		if (dKomi < 0) { dKomi = 0; }
		else if (dKomi >= vn_label_size) { dKomi = vn_label_size - 1; }
		return dKomi;	// for training fast end game
	} else {
		float fCenterKomi = (WeichiConfigure::BoardSize == 7 ? 0 : 7.5);	// 7x7 special case
		int komi_diff = static_cast<int>(dKomi - fCenterKomi);
		int komi_index = (vn_label_size - 1) / 2 + komi_diff;
		if (komi_index < 0) { komi_index = 0; }
		else if (komi_index >= vn_label_size) { komi_index = vn_label_size - 1; }

		return komi_index;
	}
}

void WeichiCNNNet::setBatchData(int batch_pos, const float* data, Color turnColor, const WeichiBitBoard& bmLegal, SymmetryType type/*=SYM_NORMAL*/)
{
	int start_index = batch_pos * (WeichiConfigure::TotalGrids + 1);
	for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
		m_bIsLegal[start_index + WeichiMove::toCompactPosition(*it)] = bmLegal.BitIsOn(WeichiMove::toCompactPosition(*it));
	}
	m_bIsLegal[start_index + PASS_MOVE.getCompactPosition()] = bmLegal.BitIsOn(PASS_MOVE.getCompactPosition());

	{
		m_color[batch_pos] = turnColor;
		//m_reverseSymmetric[batch_pos] = ReverseSymmetricType[type];
		m_reverseSymmetric[batch_pos] = type;
		int start_index = batch_pos*m_input_data_size;
		memcpy((m_input + start_index), data, sizeof(float)*m_input_data_size);
		rotateFeature((m_input + start_index), type);
	}
} 
