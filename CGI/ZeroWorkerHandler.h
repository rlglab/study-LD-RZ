#ifndef ZEROWORKERHANDLER_H
#define ZEROWORKERHANDLER_H

#include "BasicType.h"
#include "Dual.h"
#include "StatisticData.h"
#include "FileDirectoryExplorer.h"
#include "TBaseWorkerHandler.h"
#include <cfloat>
#include <boost/algorithm/string.hpp>

enum ZeroType {
	ZERO_UNKNOWN,
	ZERO_SELFPLAY,
	ZERO_OPTIMIZAITON,

	ZERO_WORKER_TYPE_SIZE,
};

inline string getZeroTypeString(ZeroType type) {
	switch (type) {
		case ZERO_UNKNOWN:	return "Unknown";
		case ZERO_SELFPLAY:	return "Self-play";
		case ZERO_OPTIMIZAITON:	return "Optimization";
		default:	return "error type.";
	}
}

class ZeroWorkerSharedData {
public:
	boost::mutex m_mutex;

	uint m_train_step;
	uint m_total_games;
	fstream& m_fWorkerLog;
	boost::mutex& m_workerMutex;

	boost::mutex m_selfPlayQueueMutex;
	boost::mutex m_optimizationQueueMutex;
	std::deque<string> m_selfPlayQueue;
	std::deque<string> m_optimizationQueue;

	ZeroWorkerSharedData(boost::mutex& workerMutex, fstream& fWorkerLog)
		: m_workerMutex(workerMutex), m_fWorkerLog(fWorkerLog)
	{
	}

	string getOneSelfPlay();
	string getOptimizationResult();
};

class ZeroWorkerStatus : public TBaseWorkerStatus
{
private:
	bool m_bIdle;
	string m_sName;
	string m_sGPUList;
	ZeroType m_type;
	ZeroWorkerSharedData& m_sharedData;

public:
	ZeroWorkerStatus(boost::shared_ptr<tcp::socket> socket, ZeroWorkerSharedData& sharedData)
		: TBaseWorkerStatus(socket)
		, m_sharedData(sharedData)
		, m_bIdle(false)
		, m_type(ZERO_UNKNOWN)
	{
	}

	~ZeroWorkerStatus()
	{
	}

	inline bool isIdle() const { return m_bIdle; }
	inline string getName() const { return m_sName; }
	inline string getGPUList() const { return m_sGPUList; }
	inline ZeroType getType() const { return m_type; }
	inline void setIdle(bool bIdle) { m_bIdle = bIdle; }

	boost::shared_ptr<ZeroWorkerStatus> shared_from_this()
	{
		return boost::static_pointer_cast<ZeroWorkerStatus>(TBaseWorkerStatus::shared_from_this());
	}

	void handle_msg(const std::string msg);
	void do_close();
};

class ZeroLogger {
public:
	fstream m_fWorkerLog;
	fstream m_fSelfPlayGame;
	fstream m_fSelfPlayDebugGame;
	fstream m_fTrainingLog;
	map< string, pair<fstream, int> > m_mSelfplay;
	FileDirectoryExplorer m_logDirExplorer;

	ZeroLogger() {}
	void createLogDirectoryAndFiles() {
		// create log file
		string sWorkerLogFileName = WeichiConfigure::zero_train_directory + "/Worker.log";
		m_fWorkerLog.open(sWorkerLogFileName.c_str(), ios::out | ios::app);
		
		string sTrainingLogFileName = WeichiConfigure::zero_train_directory + "/Training.log";
		m_fTrainingLog.open(sTrainingLogFileName.c_str(), ios::out | ios::app);

		// Separation line
		for (int i = 0; i < 100; i++) {
			m_fWorkerLog << "=";
			m_fTrainingLog << "=";
		}
		m_fWorkerLog << endl;
		m_fTrainingLog << endl;
	}
};

class ZeroWorkerHandler : public TBaseWorkerHandler<ZeroWorkerStatus>
{
private:
	uint m_seed;
	uint m_iteration;
	uint m_total_games;
	float m_fKomi;
	string m_sLatestModel;
	ZeroType m_stage;
	ZeroLogger m_logger;
	ZeroWorkerSharedData m_sharedData;
	vector<float> m_vResignThreshold;
	vector< pair<string, StatisticData> > m_vBestAgentPool;

	boost::asio::deadline_timer m_keepAliveTimer;

	string m_sSelfPlayConfigure;
	string m_sOptimizationConfigure;

	static const int MAX_NUM_CPU = 48;

public:
	ZeroWorkerHandler(int port)
		: TBaseWorkerHandler(port)
		, m_sharedData(m_workerMutex, m_logger.m_fWorkerLog)
		, m_keepAliveTimer(m_io_service)
	{
		m_seed = Configure::UseTimeSeed ? static_cast<uint>(time(NULL)) : Configure::RandomSeed;
		keepAliveTimer();
	}

	void run();
	boost::shared_ptr<ZeroWorkerStatus> handle_accept_new_worker(boost::shared_ptr<tcp::socket> socket);

	void broadcast(const std::string& msg) {
		boost::lock_guard<boost::mutex> lock(m_workerMutex);
		for (auto worker : m_vWorkers) { worker->write(msg); }
	}

private:
	void initialize();
	
	void SelfPlay();
	void Optimization();

	string getSelfPlayConfigure(boost::shared_ptr<ZeroWorkerStatus> worker);
	string getOptimizationConfigure();

	void adjustResignThreshold();
	void addResignThreshold(string sSgfString);
	string deleteUnusedSgfTag(string sSgfString);

	void keepAlive();
	void keepAliveTimer();

	inline uint getNextSeed() {
		m_seed = (m_seed + MAX_NUM_CPU) % INT_MAX;
		return m_seed;
	}
	inline int getModelIteration(string sModelName) {
		int start = sModelName.find("iter_") + 5;
		int end = sModelName.find(".pb");
		return atoi(sModelName.substr(start, end - start).c_str());
	}
};

#endif 
