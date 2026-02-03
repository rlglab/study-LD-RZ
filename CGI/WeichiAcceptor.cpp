#include "WeichiAcceptor.h"
#include "arguments.h"
#include "Logger.h"

WeichiAcceptor::WeichiAcceptor(uint port) : m_acceptor(m_io_service)
	, m_strand(m_io_service)
{
	if ( arguments::mode == "cnnserver" ) {
		tcp::endpoint endpoint(tcp::v4(), port);
		m_acceptor.open(endpoint.protocol());
		boost::asio::socket_base::reuse_address option(true);
		m_acceptor.set_option(option);
		m_acceptor.bind(endpoint);
		m_acceptor.listen();
		m_strand.dispatch(boost::bind(&WeichiAcceptor::start_accept, this));
		CERR() << "CGI CNN server is running (port: " << port << ")." << std::endl;
	}
}

WeichiAcceptor::~WeichiAcceptor()
{
	m_io_service.stop();
	m_acceptor.close();

	boost::lock_guard<boost::mutex> lock(m_workerMutex);
	for ( uint i=0; i<m_vWorkers.size(); ++i ) {
		delete m_vWorkers[i];
	}
	m_vWorkers.clear();
}

void WeichiAcceptor::run()
{
	m_io_service.run();
}

void WeichiAcceptor::broadcast(const std::string& msg, bool bBroadcastGPU)
{
	istringstream iss(msg);
	std::string cmd;
	iss >> cmd;
	if ( cmd == "clear_board" ) {
		m_buffered_command.clear();
	} else if ( cmd == "play" || cmd == "komi" ) {
		m_buffered_command.push_back(msg);
	}

	boost::unique_lock<boost::mutex> lock(m_workerMutex);
	for ( uint i=0; i<m_vWorkers.size(); ++i ) {
		bool bNeedWrite = false;
		if ( bBroadcastGPU && m_vWorkers[i]->getNumGpuThread()>0 ) { bNeedWrite = true; }
		if ( bNeedWrite ) { m_vWorkers[i]->write(msg); }
	}
	lock.unlock();

	if ( cmd == "start" ) {
		while ( !isAllWorkerRunning() );
	} else if ( cmd == "stop" ) {
		while ( isAnyWorkerRunning() );
	}
}

void WeichiAcceptor::recalculateNumThread()
{
	m_totalNumGpuThread = 0;
	boost::lock_guard<boost::mutex> lock(m_workerMutex);
	for ( uint i=0; i<m_vWorkers.size(); ++i ) {
		m_totalNumGpuThread += m_vWorkers[i]->getNumGpuThread();
	}
}

bool WeichiAcceptor::isAllWorkerRunning()
{
	boost::lock_guard<boost::mutex> lock(m_workerMutex);
	bool bIsAllWorkerRunning = true;
	for ( uint i = 0; i < m_vWorkers.size() && bIsAllWorkerRunning; ++i ) {
		bool bWorkerRunning = m_vWorkers[i]->getNumWorkerRunningThread() == m_vWorkers[i]->getNumGpuThread();
		bIsAllWorkerRunning &= bWorkerRunning;
	}
	return bIsAllWorkerRunning;
}

bool WeichiAcceptor::isAnyWorkerRunning()
{
	boost::lock_guard<boost::mutex> lock(m_workerMutex);
	bool bIsAnyWorkerRunning = false;
	for ( uint i = 0; i < m_vWorkers.size() && !bIsAnyWorkerRunning; ++i ) {
		bool bWorkerIdle = m_vWorkers[i]->getNumWorkerRunningThread() == 0;
		bIsAnyWorkerRunning |= !bWorkerIdle;
	}
	return bIsAnyWorkerRunning;
}

void WeichiAcceptor::start_accept()
{
	m_nextSocket = boost::make_shared<tcp::socket>(m_io_service);
	m_acceptor.async_accept(*m_nextSocket, m_strand.wrap(boost::bind(&WeichiAcceptor::handle_accept, this, boost::asio::placeholders::error)) );
}

void WeichiAcceptor::handle_accept(const boost::system::error_code& error)
{
	if ( !error ) {
		if ( m_vWorkers.size() == MAX_NUM_WORKERS ) {
			CERR() << "Too many workers" << std::endl;
		} else {
			boost::lock_guard<boost::mutex> lock(m_workerMutex);
			m_vWorkers.push_back(new WeichiWorkerStatus(boost::make_shared<boost::asio::io_service::strand>(m_io_service), m_nextSocket, m_vWorkers, m_workerMutex));

			// synchronize board
			for ( uint i=0; i<m_buffered_command.size(); ++i ) {
				m_vWorkers[m_vWorkers.size()-1]->write(m_buffered_command[i]);
			}

			// initialize command
			m_vWorkers[m_vWorkers.size()-1]->write("initialize");

			CERR() << "New worker " << m_nextSocket->remote_endpoint().address().to_string() << ":" << m_nextSocket->remote_endpoint().port()
				   << " connected. (" << m_vWorkers.size() << " workers now)" << std::endl;
		}
	}
	start_accept();
} 
