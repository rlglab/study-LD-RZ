#include <boost/algorithm/string.hpp>

#include "WeichiWorkerStatus.h"
#include "Logger.h"
#include "WeichiCNNNetType.h"
#include "WeichiGlobalInfo.h"

WeichiWorkerStatus::WeichiWorkerStatus(boost::shared_ptr<boost::asio::io_service::strand> strand, boost::shared_ptr<tcp::socket> socket, Vector<WeichiWorkerStatus*, MAX_NUM_WORKERS>& vWorkers, boost::mutex& workerMutex)
	: m_strand(strand)
	, m_socket(socket)
	, m_vWorkers(vWorkers)
	, m_workerMutex(workerMutex)
	, m_isRunningJob(false)
	, m_numWorkerRunningThread(0)
	, m_ip("0.0.0.0")
	, m_port(0)
{
	m_threadIDtoJobType.resize(MAX_NUM_THREADS+1);
	m_threadIDtoJobID.resize(MAX_NUM_THREADS+1);
	socket->set_option(tcp::no_delay(true));
	m_ip = socket->remote_endpoint().address().to_string();
	m_port = socket->remote_endpoint().port();
	m_strand->dispatch(boost::bind(&WeichiWorkerStatus::start_read, this));

	if ( Configure::LogServerInformation ) {
		uint ithWorker;
		// m_workerMutex is locked by WeichiAcceptor::handle_accept
		ithWorker = m_vWorkers.size();
		m_serverLogger.openFile( string("ServerLog_")+ToString(ithWorker)+".txt" );
		m_serverLogger.logLine( ClusterLogger::COMMENT, "CGI cnnserver start" );
	}
}

WeichiWorkerStatus::~WeichiWorkerStatus()
{
	if ( m_socket->is_open() ) {
		boost::system::error_code ec;
		boost::asio::write(*m_socket, boost::asio::buffer("quit\n"), ec);
		boost::this_thread::sleep(boost::posix_time::milliseconds(20));
	}

	if ( Configure::LogServerInformation ) {
		m_serverLogger.logLine( ClusterLogger::COMMENT, "CGI cnnserver stop" );
		m_serverLogger.closeFile();
	}
}

void WeichiWorkerStatus::write(std::string msg)
{
	if ( msg.empty() ) { return; }

	istringstream iss(msg);
	std::string cmd;
	iss >> cmd;
	if ( cmd == "start" ) { m_isRunningJob = true; }
	if ( cmd == "stop" ) { m_isRunningJob = false; }

	if ( msg[msg.length()-1] != '\n' ) msg += '\n';
	m_strand->dispatch(boost::bind(&WeichiWorkerStatus::do_write, this, msg));
}

void WeichiWorkerStatus::do_write(const std::string msg)
{
	m_msg_queue.push_back(msg) ;
	if ( m_msg_queue.size() > 1 ) {
		// outstanding async_write, will call handle_write later
		return;
	} else {
		// all async_write is completed, safe to start writing
		write_next();
	}
}

void WeichiWorkerStatus::write_next()
{
	const std::string& cmd_sending = m_msg_queue.front();

	boost::asio::async_write(*m_socket,
		boost::asio::buffer(cmd_sending),
		m_strand->wrap(boost::bind(&WeichiWorkerStatus::handle_write, this, boost::asio::placeholders::error)));

	if ( Configure::LogServerInformation ) {
		m_serverLogger.logLine( ClusterLogger::NET_OUT, boost::algorithm::trim_copy(cmd_sending) );
	}
}

void WeichiWorkerStatus::handle_write(const boost::system::error_code& error)
{
	m_msg_queue.pop_front();

	if ( error ) {
		CERR() << "Could not write to " << m_ip << ":" << m_port
			   << ", response: " << boost::system::system_error(error).what() << std::endl;
		do_close();
		return;
	}

	if ( !m_msg_queue.empty() ) {
		write_next();
	}
}

void WeichiWorkerStatus::start_read()
{
	boost::asio::async_read_until(*m_socket,
		m_buffer, '\n', 
		m_strand->wrap(boost::bind(&WeichiWorkerStatus::handle_read, this,
		boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)));
}

void WeichiWorkerStatus::handle_read(const boost::system::error_code& error, size_t bytes_read)
{
	if ( error ) {
		CERR() << "Could not read from " << m_ip << ":" << m_port
			   << ", response: " << boost::system::system_error(error).what() << std::endl;
		do_close();
	} else {
		handle_reply();
		start_read();
	}
}

void WeichiWorkerStatus::handle_reply()
{
	istream is(&m_buffer);
	std::string line;
	std::getline(is, line);
	istringstream iss (line);

	if ( Configure::LogServerInformation ) {
		m_serverLogger.logLine( ClusterLogger::NET_IN, line );
	}

	std::string cmd;
	iss >> cmd;

	//if ( cmd != "reply_job" ) { CERR() << "Reply: " << line << std::endl; }
	//else { CERR() << "Reply: " << line << std::endl; }

	if ( cmd == "reply_initialize" ) { handle_reply_initialize(line); }
	else if ( cmd == "reply_start" ) { handle_reply_start(line); }
	else if ( cmd == "reply_job" ) { handle_reply_job(line); }
	else if ( cmd == "reply_stop" ) { handle_reply_stop(); }
	else {
		CERR() << "Worker reply unknown command: " << line << std::endl;
	}
}

void WeichiWorkerStatus::handle_reply_initialize(const std::string line)
{
	istringstream iss (line);
	std::string cmd;
	iss >> cmd >> m_numGpuThread;
}

void WeichiWorkerStatus::handle_reply_start(const std::string line)
{
	istringstream iss (line);
	std::string cmd;
	uint threadId;
	uint sessionId;
	std::string sJobType;
	iss >> cmd >> threadId >> sessionId >> sJobType;

	if ( sessionId != WeichiGlobalInfo::getRemoteInfo().m_jobSessionId ) { return; }

	if ( sJobType == "GPU" ) {
		std::string sJobCnnType;
		uint batchSize;
		iss >> sJobCnnType >> batchSize;
		sendGpuJob(threadId, getWeichiCNNNetType(sJobCnnType), batchSize);
	} else {
		CERR() << "error job type: " << sJobType << endl;
	}
	++m_numWorkerRunningThread;
}

void WeichiWorkerStatus::handle_reply_job(const std::string line)
{
	istringstream iss (line);
	std::string cmd;
	uint threadId;
	uint sessionId;
	std::string sJobType;
	std::string sJobResult;
	iss >> cmd >> threadId >> sessionId >> sJobType;

	if ( sessionId != WeichiGlobalInfo::getRemoteInfo().m_jobSessionId ) { CERR() << "handle_reply_job: sessionId mismatch" << std::endl; return; }

	if ( sJobType == "GPU" ) {
		std::string sJobNextCnnType;
		uint batchSize;
		std::string sJobCurrentCnnType;
		iss >> sJobNextCnnType >> batchSize >> sJobCurrentCnnType;
		getline(iss, sJobResult);

		if ( m_threadIDtoJobID[threadId]!=-1 ) { updateGpuJob(threadId, sessionId, getWeichiCNNNetType(sJobCurrentCnnType), sJobResult); }
		if ( !m_isRunningJob ) { return; }
		sendGpuJob(threadId, getWeichiCNNNetType(sJobNextCnnType), batchSize);
	} else {
		CERR() << "error job type: " << sJobType << endl;
	}
}

void WeichiWorkerStatus::handle_reply_stop()
{
	m_numWorkerRunningThread = 0;
}

void WeichiWorkerStatus::do_close()
{
	m_socket->close();

	// revert all outstanding job
	if ( m_isRunningJob ) {
		for ( uint i = 0; i < m_threadIDtoJobType.size(); ++i ) {
			if ( m_threadIDtoJobID[i] != -1 ) {
				uint jobID = m_threadIDtoJobID[i];
				if ( m_threadIDtoJobType[i] == "GPU" ) { WeichiGlobalInfo::getRemoteInfo().m_cnnJobTable.revertJob(jobID); }
			}
		}
	}

	// delete self from worker status vector
	boost::lock_guard<boost::mutex> lock(m_workerMutex);
	for ( uint i=0; i<m_vWorkers.size(); ++i ) {
		if ( m_vWorkers[i] == this ) {
			delete m_vWorkers[i];
			m_vWorkers.erase_no_order(i);
			CERR() << "A worker disconnected. (" << m_vWorkers.size() << " workers now)" << std::endl;
			break;
		}
	}
}

void WeichiWorkerStatus::sendGpuJob(uint threadId, WeichiCNNNetType type, uint batchSize)
{
	string serializedString;
	uint jobID = WeichiGlobalInfo::getRemoteInfo().m_cnnJobTable.produceNewJob( WeichiGlobalInfo::getRemoteInfo().m_jobSessionId, type, batchSize, serializedString );
	m_threadIDtoJobType[threadId] = "GPU";
	m_threadIDtoJobID[threadId] = jobID;

	ostringstream replyStream;
	replyStream << "job " << threadId << " GPU " << serializedString;
	write( replyStream.str() );

}

void WeichiWorkerStatus::updateGpuJob(uint threadId, uint sessionId, WeichiCNNNetType type, std::string sJobResult)
{
	uint jobID = m_threadIDtoJobID[threadId];
	if ( jobID == -1 ) { return; } // empty job
	WeichiGlobalInfo::getRemoteInfo().m_cnnJobTable.pushUpdateJob(jobID, sJobResult);
}