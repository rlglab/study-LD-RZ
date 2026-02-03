#ifndef WEICHICNNGTPENGINE_H
#define WEICHICNNGTPENGINE_H

#include "WeichiBaseGtpEngine.h"
#include "TMiniMCTS.h"

class WeichiCNNGtpEngine: public WeichiBaseGtpEngine
{
private:
	int m_slNetID;
	int m_bvvnNetID;
	uint m_maxOrder;
	uint m_iTreeSearchSimulationNum;
	SymmetryType m_cnnRotateType;

public:
	WeichiCNNGtpEngine( std::ostream& os, MCTS& instance )
		: WeichiBaseGtpEngine(os,instance)
	{
		m_slNetID = 0;
		m_bvvnNetID = 0;
		m_maxOrder = MAX_NUM_GRIDS;
		m_iTreeSearchSimulationNum = 100;
		m_cnnRotateType = SYM_NORMAL;
	}

	void Register();
	string getGoguiAnalyzeCommandsString();

private:
	void cmdDCNNSimTerritory();
	void cmdDCNNMovePredictor();
	void cmdDCNNMoveOrder();
	void cmdDCNNMoveScore();
	void cmdDCNNBVVN();
	void cmdDCNNEye();
	void cmdDCNNConnect();
	void cmdDCNNConnectValue();
	void cmdDCNNParamSetting();
	void cmdDCNNNet();
	void cmdDCNNReloadNet();

	vector<CandidateEntry> calculateSLCandidate( SymmetryType type=SYM_NORMAL );
};

#endif