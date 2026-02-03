#ifndef ZEROOPTIMIZATION_H
#define ZEROOPTIMIZATION_H

#include "TMiniMCTS.h"
#include "ColorMessage.h"
#include "TBaseMasterSlave.h"
#include "WeichiCNNTrainNet.h"
#include "FileDirectoryExplorer.h"
#include "SgfLoader.h"

class ZeroOptimizationMSSharedData {
public:
	boost::mutex m_mutex;
	bool m_bGPUEvaluateFirstBatch;
	WeichiCNNTrainNet* m_trainNet;
	deque<int> m_selfplayGameIndex;
	deque< pair<string, int> > m_selfplayGameRecords;
};

class ZeroOptimizationSlave : public TBaseSlave<ZeroOptimizationMSSharedData> {
private:
	float *m_label;
	bool m_bIsInitialize;
	WeichiThreadState m_state;

public:
	ZeroOptimizationSlave(int id, ZeroOptimizationMSSharedData& sharedData)
		: TBaseSlave(id, sharedData), m_bIsInitialize(false)
	{
	}

	bool isOver() { return false; }
	void reset() {}
	void doSlaveJob();

	inline bool isInitialize() { return m_bIsInitialize; }

private:
	void initialize();
	void calculateFeatures();
	void setLADSgfTags(const SgfLoader& sgfLoader);
	void setLADBitBoard(string sSgfPos, WeichiBitBoard& bmStone);
	map<uint, float> calculateMCTSSearchDistribution(string sMCTSSearchDistribution, const WeichiMove& selectMove);
};

class ZeroOptimizationMaster : public TBaseMaster<ZeroOptimizationMSSharedData, ZeroOptimizationSlave> {
private:
	int m_optimizaionIteration;
	string m_command;
	boost::thread m_input_thread;
	boost::atomic<bool> m_bHasCommand;

public:
	ZeroOptimizationMaster(int NumThread)
		: TBaseMaster(NumThread)
		, m_optimizaionIteration(-1)
		, m_bHasCommand(false)
	{
	}

	void run();
	bool initialize();

	void summarizeSlavesData() {}

private:
	void runOneOptimization();
	void readInput();
	void loadSgf();
	void loadSgf(string sFileName);
	void shiftWindow();
};

#endif 
