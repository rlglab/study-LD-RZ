#ifndef WEICHITRAINTGTPENGINE_H
#define WEICHITRAINTGTPENGINE_H

#include "WeichiBaseGtpEngine.h"

class WeichiTrainGtpEngine: public WeichiBaseGtpEngine
{
private:
	fstream m_fout;
	string m_sFileName;
	BaseCNNPlayoutGenerator m_cnnPlayoutGenerator;
	uint m_iCNNSearchStack;
public:
	WeichiTrainGtpEngine( std::ostream& os, MCTS& instance )
		: WeichiBaseGtpEngine(os,instance)
		, m_cnnPlayoutGenerator(const_cast<WeichiThreadState&>(instance.getState()))
	{
	}

	void Register();
	string getGoguiAnalyzeCommandsString();

private:
	void cmdOpenFile();
	void cmdWriteFile();
	void cmdClearFile();
	void TSLFeatures();
	void TDCNNBVTerritory();
	void TPurePlayoutWinRate();
	void TBVPlayoutWinRate();
	void TMCTSWinRate();
	void TCNNPlayoutWinRate();
};

#endif