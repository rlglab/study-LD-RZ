#ifndef _WEICHIACCEPTOR_H_
#define _WEICHIACCEPTOR_H_

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>

#include "types.h"
#include "WeichiWorkerStatus.h"

using boost::asio::ip::tcp;

class WeichiAcceptor
{
private:
	boost::asio::io_service m_io_service;
	boost::asio::io_service::strand m_strand;
	boost::shared_ptr<tcp::socket> m_nextSocket;
	tcp::acceptor m_acceptor;

	Vector<WeichiWorkerStatus*, MAX_NUM_WORKERS> m_vWorkers;
	boost::mutex m_workerMutex;

	std::vector<std::string> m_buffered_command;
	uint m_totalNumGpuThread;

public:
	WeichiAcceptor(uint port);
	~WeichiAcceptor();

	// called by master
	void run();
	void broadcast(const std::string& msg, bool bBroadcastGPU=true);
	void recalculateNumThread();
	bool isAllWorkerRunning();
	bool isAnyWorkerRunning();

	// called by job handler
	uint getTotalNumGpuThread() { return m_totalNumGpuThread; }

private:
	/// accept operation
	void start_accept();
	void handle_accept(const boost::system::error_code& error);
};


#endif