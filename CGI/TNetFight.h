#ifndef TNETFIGHT_H
#define TNETFIGHT_H

#include "BasicType.h"
#include "SgfLoader.h"
#include "TBaseMasterSlave.h"
#include "BaseCNNPlayoutGenerator.h"
#include <boost/format.hpp>
#include <boost/filesystem.hpp>

class TNetFightSharedData {
public:
	int m_nFight;
	int m_iteration;
	string m_sModelName;
};

class TNetFightSlave : public TBaseSlave<TNetFightSharedData> {
private:
	WeichiThreadState m_state;
	Dual<StatisticData> m_winRate;
	BaseCNNPlayoutGenerator m_cnnPlayoutGenerator;

public:
	TNetFightSlave(int id, TNetFightSharedData& sharedData)
		: TBaseSlave(id, sharedData)
		, m_cnnPlayoutGenerator(m_state)
	{
	}

	~TNetFightSlave()
	{
	}

	inline WeichiThreadState& getState() { return m_state; }
	Dual<StatisticData> getWinRate() { return m_winRate; }
	inline BaseCNNPlayoutGenerator& getCNNPlayoutGenerator() { return m_cnnPlayoutGenerator; }

private:
	void initialize()
	{
		int total_gpu_size = static_cast<int>(WeichiConfigure::dcnn_train_gpu_list.size());
		int gpuID = (total_gpu_size > 0) ? (WeichiConfigure::dcnn_train_gpu_list[m_id%total_gpu_size] - '0') : -1;
		vector<string> vParamString = splitToVector(WeichiConfigure::dcnn_net, ':');
		m_state.m_dcnnNetCollection.initialize(1, vParamString, gpuID);

		m_cnnPlayoutGenerator.initialize(m_state.m_dcnnNetCollection.getSLNet()->getBatchSize());
		TBaseSlave::initialize();
	}

	bool isOver() { return false; }
	
	void reset()
	{
		m_state.m_dcnnNetCollection.getSLNet()->reloadNetWork(m_sharedData.m_sModelName);
	}
	
	void doSlaveJob()
	{
		m_cnnPlayoutGenerator.setSLNet(COLOR_BLACK, m_state.m_dcnnNetCollection.getSLNet(0));
		m_cnnPlayoutGenerator.setSLNet(COLOR_WHITE, m_state.m_dcnnNetCollection.getSLNet(1));
		m_winRate.m_black = m_cnnPlayoutGenerator.run(m_sharedData.m_nFight / Configure::NumThread);
		
		m_cnnPlayoutGenerator.setSLNet(COLOR_BLACK, m_state.m_dcnnNetCollection.getSLNet(1));
		m_cnnPlayoutGenerator.setSLNet(COLOR_WHITE, m_state.m_dcnnNetCollection.getSLNet(0));
		m_winRate.m_white = m_cnnPlayoutGenerator.run(m_sharedData.m_nFight / Configure::NumThread);
	}
};

class TNetFightMaster : public TBaseMaster<TNetFightSharedData, TNetFightSlave> {
private:
	int m_nModel;
	WeichiThreadState m_state;
	boost::filesystem::recursive_directory_iterator m_dirIterator;

public:
	TNetFightMaster()
		: TBaseMaster(Configure::NumThread)
	{
		initialize();
	}

	void run()
	{
		m_sharedData.m_iteration = 0;
		while (m_dirIterator != boost::filesystem::recursive_directory_iterator()) {
			if (is_directory(m_dirIterator->status())) { ++m_dirIterator; continue; }

			m_sharedData.m_sModelName = m_dirIterator->path().string();
			runOneIteration();

			++m_dirIterator;
			++m_sharedData.m_iteration;
		}
	}

private:
	bool initialize()
	{
		string sDirectory;
		CERR() << "input directory: ";
		cin >> sDirectory;
		CERR() << sDirectory << endl;
		m_dirIterator = boost::filesystem::recursive_directory_iterator(sDirectory);

		CERR() << "input number of fight games: ";
		cin >> m_sharedData.m_nFight;
		CERR() << m_sharedData.m_nFight << endl;

		return TBaseMaster::initialize();
	}

	void runOneIteration()
	{
		boost::posix_time::ptime pStart = TimeSystem::getLocalTime();

		// do one iteration
		for (int i = 0; i < m_nThread; i++) { m_vSlaves[i]->startRun(); }
		for (int i = 0; i < m_nThread; i++) { m_vSlaves[i]->finishRun(); }

		summarizeSlavesData();

		boost::posix_time::ptime pEnd = TimeSystem::getLocalTime();
		boost::posix_time::time_duration duration = pEnd - pStart;
		CERR() << ". Total use " << duration.total_seconds() << "(s)." << endl;
	}

	void summarizeSlavesData()
	{
		Dual<StatisticData> winRate;
		for (int i = 0; i < m_nThread; i++) {
			winRate.m_black.add(m_vSlaves[i]->getWinRate().m_black);
			winRate.m_white.add(m_vSlaves[i]->getWinRate().m_white);
		}

		CERR() << "opponent: " << m_sharedData.m_sModelName
			<< ", black_winrate: " << winRate.m_black.getMean() * 100 << "/" << winRate.m_black.getCount()
			<< ", white_winrate: " << (1 - winRate.m_white.getMean()) * 100 << "/" << winRate.m_white.getCount()
			<< ", avg_winrate: " << (winRate.m_black.getMean() + (1 - winRate.m_white.getMean())) / 2 * 100
			<< "/" << (winRate.m_black.getCount() + winRate.m_white.getCount());
	}
};

#endif