#include <boost/algorithm/string.hpp>

#include "WeichiWorker.h"
#include "WeichiMonteCarlo.h"

WeichiWorker *WeichiWorker::s_instance = nullptr;
WeichiWorker::WeichiWorker( MCTS& engine, std::string host, int port )
	: m_mcts(engine)
	, m_rootTurnColor(COLOR_BLACK)
	, m_socket(m_io_service)
	, m_bIsServerRunning(false)
	, m_numActiveThread(0)
{
	CERR() << "Try to connect to " << host << ":" << port << endl;

	tcp::resolver resolver(m_io_service);
	tcp::resolver::query query(host, ToString(port));
	tcp::resolver::iterator iterator = resolver.resolve(query);

	m_socket.async_connect(*iterator,
		boost::bind(&WeichiWorker::handle_connect, this,
		boost::asio::placeholders::error));

	for ( uint i=0; i<MAX_NUM_THREADS+1; ++i ) {
		m_bIsJobRunning[i] = false;
	}

	if ( Configure::LogWorkerInformation ) {
		m_workerLogger.openFile( string("WorkerLog")+".txt" );
		m_workerLogger.logLine( ClusterLogger::COMMENT, "CGI cnnworker start" );
	}
}

WeichiWorker::~WeichiWorker()
{
	if ( s_instance != nullptr ) { delete s_instance; }
	if ( Configure::LogWorkerInformation ) {
		m_workerLogger.logLine( ClusterLogger::COMMENT, "CGI cnnworker stop" );
	}
}

void WeichiWorker::initialize( MCTS& engine, std::string host, int port )
{
	if ( s_instance == nullptr ) { s_instance = new WeichiWorker(engine, host, port); }
}

void WeichiWorker::run()
{
	s_instance->m_io_service.run();
}

void WeichiWorker::write( std::string msg )
{
	if ( msg.empty() ) { return; }
	if ( msg[msg.length()-1] != '\n' ) { msg += '\n'; }
	s_instance->m_io_service.dispatch(boost::bind(&WeichiWorker::do_write, s_instance, msg));
}

void WeichiWorker::do_write( const std::string msg )
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

void WeichiWorker::write_next()
{
	const std::string& cmd_sending = m_msg_queue.front();

	boost::asio::async_write(m_socket,
		boost::asio::buffer(cmd_sending),
		boost::bind(&WeichiWorker::handle_write, this, boost::asio::placeholders::error));

	istringstream iss (cmd_sending);
	std::string cmd;
	iss >> cmd;
	if ( cmd == "reply_job" ) {
		uint threadId;
		iss >> threadId;
		s_instance->m_bIsJobRunning[threadId] = false;
	} else if ( cmd == "reply_start" ) {
		++(s_instance->m_numActiveThread);
	}

	if ( Configure::LogWorkerInformation ) {
		m_workerLogger.logLine( ClusterLogger::NET_OUT, boost::algorithm::trim_copy(cmd_sending) );
	}
}

void WeichiWorker::handle_write( const boost::system::error_code& error )
{
	m_msg_queue.pop_front();

	if ( error ) {
		CERR() << "could not write: " << boost::system::system_error(error).what() << std::endl;
		do_close();
		return;
	}

	if ( !m_msg_queue.empty() ) {
		write_next();
	}
}

void WeichiWorker::handle_connect( const boost::system::error_code& error )
{
	if ( error ) {
		CERR() << "Connection with CGI server has failed." << endl;
	} else {
		m_socket.set_option(tcp::no_delay(true));
		CERR() << "Connected to CGI server." << endl;
		start_read();
	}
}

void WeichiWorker::start_read()
{
	boost::asio::async_read_until(m_socket,
		m_buffer, '\n', 
		boost::bind(&WeichiWorker::handle_read, this,
		boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void WeichiWorker::handle_read( const boost::system::error_code& error, size_t bytes_read )
{
	if ( error ) do_close();
	else {
		handle_command();
		start_read();
	}
}

void WeichiWorker::handle_command()
{
	using namespace std;
	istream is(&m_buffer);
	std::string line;
	std::getline(is, line);
	istringstream iss (line);

	if ( Configure::LogWorkerInformation ) {
		m_workerLogger.logLine( ClusterLogger::NET_IN, line );
	}

	std::string cmd;
	iss >> cmd;

	if ( cmd != "job" && cmd.substr(0, 12) != "cache_update" ) { CERR() << "Command: " << line << std::endl; }
	//else { CERR() << "Command: " << cmd << std::endl; }

	if ( cmd == "play" ) { handle_play(line); }
	else if ( cmd == "clear_board" ) { handle_clearboard(); }
	else if ( cmd == "initialize" ) { handle_initialize(); }
	else if ( cmd == "start" ) { handle_start(line); }
	else if ( cmd == "job" ) { handle_job(line); }
	else if ( cmd == "stop" ) { handle_stop(); }
	else if ( cmd == "quit" ) { handle_quit(); }
	else {
		CERR() << "Server reply unknown command: " << line << std::endl;
	}
}

void WeichiWorker::handle_play( const std::string& line )
{
	if ( m_bIsServerRunning ) {
		CERR() << "handle_play error: server is running" << std::endl;
		return;
	}

	istringstream iss (line);
	std::string cmd;
	std::string color;
	std::string position;

	iss >> cmd >> color >> position;
	WeichiMove m (toColor(color[0]), position);
	if ( !m_mcts.play(m) ) { do_close(); }
	m_rootTurnColor = AgainstColor(m.getColor());
}

void WeichiWorker::handle_clearboard()
{
	if ( m_bIsServerRunning ) {
		CERR() << "handle_clearboard error: server is running" << std::endl;
		return;
	}
	m_mcts.newGame();
	m_rootTurnColor = COLOR_BLACK;
}

void WeichiWorker::handle_initialize()
{
	ostringstream replyStream;
	m_numGpuThread = WeichiConfigure::TotalGpuNum;
	replyStream << "reply_initialize " << m_numGpuThread;
	write(replyStream.str());
}

void WeichiWorker::handle_start( const std::string& line )
{
	istringstream iss (line);
	std::string cmd;
	uint sessionId;
	iss >> cmd >> sessionId >> WeichiDynamicKomi::Internal_komi;
	m_mcts.replyStart(sessionId);
	m_bIsServerRunning = true;
}

void WeichiWorker::handle_job( const std::string& line )
{
	if ( !m_bIsServerRunning ) { return; }

	// job <threadID> CPU <current_job_content>
	// job <threadID> GPU <current_job_content>
	istringstream iss (line);
	std::string cmd;
	int threadId;
	std::string jobType;
	std::string jobContent;
	iss >> cmd >> threadId >> jobType;
	getline(iss, jobContent);

	m_bIsJobRunning[threadId] = true;
	m_mcts.setupJobs(threadId, jobContent);
	m_mcts.startCompute(threadId);
}

void WeichiWorker::handle_stop()
{
	if ( !m_bIsServerRunning ) { return; }

	bool bNeedDelaySendStop = false;
	if ( m_numActiveThread != m_numGpuThread ) {
		bNeedDelaySendStop = true;
	}

	for ( uint i = 1; i <= Configure::NumThread && !bNeedDelaySendStop; ++i ) {
		bNeedDelaySendStop |= m_bIsJobRunning[i];
	}

	if ( bNeedDelaySendStop ) {
		s_instance->m_io_service.post(boost::bind(&WeichiWorker::handle_stop, s_instance));
		return;
	}

	m_bIsServerRunning = false;
	m_numActiveThread = 0;
	write("reply_stop");
}

void WeichiWorker::handle_quit()
{
	do_close();
}

void WeichiWorker::do_close()
{
	m_mcts.ponder_stop();
	m_socket.close();
	m_io_service.stop();
}
 
