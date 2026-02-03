#ifndef _WEICHIWORKERSTATUS_H_
#define _WEICHIWORKERSTATUS_H_

#include <queue>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "Vector.h"
#include "WeichiCNNNetType.h"
#include "ClusterLogger.h"

using namespace std;
using boost::asio::ip::tcp;

class WeichiWorkerStatus
{
private:
	boost::shared_ptr<boost::asio::io_service::strand> m_strand;
	boost::shared_ptr<tcp::socket> m_socket;
	Vector<WeichiWorkerStatus*, MAX_NUM_WORKERS>& m_vWorkers;
	boost::mutex& m_workerMutex;
	boost::asio::streambuf m_buffer;
	std::deque<std::string> m_msg_queue;

	bool m_isRunningJob;
	Vector<string, MAX_NUM_THREADS+1> m_threadIDtoJobType;
	Vector<uint, MAX_NUM_THREADS+1> m_threadIDtoJobID;

	uint m_numCpuThread;
	uint m_numGpuThread;
	uint m_numWorkerRunningThread;

	ClusterLogger m_serverLogger;
	string m_ip;
	uint m_port;

public:
	WeichiWorkerStatus(boost::shared_ptr<boost::asio::io_service::strand> strand, boost::shared_ptr<tcp::socket> socket, Vector<WeichiWorkerStatus*, MAX_NUM_WORKERS>& vWorkers, boost::mutex& workerMutex);
	~WeichiWorkerStatus();
	
	void write(std::string msg);
	uint getNumCpuThread() { return m_numCpuThread; }
	uint getNumGpuThread() { return m_numGpuThread; }
	uint getNumWorkerRunningThread() { return m_numWorkerRunningThread; }

private:
	/// write operation
	void do_write(const std::string msg);
	void write_next();
	void handle_write(const boost::system::error_code& error);

	/// read operation
	void start_read();
	void handle_read(const boost::system::error_code& error, size_t bytes_read);
	void handle_reply();

	void handle_reply_initialize(const std::string line);
	void handle_reply_start(const std::string line);
	void handle_reply_job(const std::string line);
	void handle_reply_stop();

	/// close
	void do_close();

	/// job handler operation
	void sendGpuJob(uint threadId, WeichiCNNNetType type, uint batchSize);
	void updateGpuJob(uint threadId, uint sessionId, WeichiCNNNetType type, std::string sJobResult);
};

#endif