#ifndef ZEROSELFPLAY_H
#define ZEROSELFPLAY_H

#include <boost/thread.hpp>
#include "TMiniMCTS.h"
#include "ColorMessage.h"
#include "TBaseMasterSlave.h"

class ZeroSelfPlayMSSharedData {
public:
	int m_gpu_size;
	Color m_turnColor;
	boost::mutex m_mutex;
	vector<TMiniMCTS*> m_vMiniMCTS;
	Dual< vector<WeichiCNNNet*> > m_vCNNNet;

	uint m_miniMCTSIndex;
	bool m_bForwardGPU;

	uint getNextMiniMCTSIndex();
};

class ZeroSelfPlaySlave : public TBaseSlave<ZeroSelfPlayMSSharedData> {
private:
	bool m_bIsInitialize;
	Dual<WeichiCNNNet*> m_cnnNet;

public:
	ZeroSelfPlaySlave(int id, ZeroSelfPlayMSSharedData& sharedData)
		: TBaseSlave(id, sharedData), m_bIsInitialize(false)
	{
	}

	bool isOver() { return false; }
	void reset() {}
	void doSlaveJob();

	inline bool isInitialize() { return m_bIsInitialize; }

private:
	inline bool isGPUThread() const { return (m_id < m_sharedData.m_gpu_size); }
	inline bool isCPUThread() const { return (m_id >= m_sharedData.m_gpu_size); }

	void initialize();
	void doCPUSelfPlay(uint gameIndex);
};

class ZeroSelfPlayMaster : public TBaseMaster<ZeroSelfPlayMSSharedData, ZeroSelfPlaySlave> {
private:
	string m_command;
	boost::thread m_input_thread;
	boost::atomic<bool> m_bHasCommand;

public:
	ZeroSelfPlayMaster(int NumThread)
		: TBaseMaster(NumThread + static_cast<int>(WeichiConfigure::dcnn_train_gpu_list.size()))
		, m_bHasCommand(false)
	{
	}

	void run();
	bool initialize();

	void summarizeSlavesData() {}

private:
	inline string getTimString() { return TimeSystem::getTimeString("[Y/m/d_H:i:s.f] "); }
	void readInput();
	void replaceNetworkModel();
};

#endif 
