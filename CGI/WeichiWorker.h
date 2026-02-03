#ifndef _WEICHIWORKER_H_
#define _WEICHIWORKER_H_

#include <iostream>
#include <sstream>
#include <string>
#include <queue>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/atomic.hpp>

#include "Color.h"
#include "Logger.h"
#include "strops.h"
#include "ClusterLogger.h"

using boost::asio::ip::tcp;

class WeichiMonteCarlo;
typedef WeichiMonteCarlo MCTS;
typedef unsigned int uint;

class WeichiWorker
{
private:
	static WeichiWorker* s_instance;

	MCTS& m_mcts;
	Color m_rootTurnColor;
	boost::asio::io_service m_io_service;
	tcp::socket m_socket;
	boost::asio::streambuf m_buffer;
	std::deque<std::string> m_msg_queue;
	std::string m_msg_sending;
	uint m_numGpuThread;
	uint m_numActiveThread;

	bool m_bIsServerRunning;
	boost::atomic<bool> m_bIsJobRunning[MAX_NUM_THREADS+1];

	ClusterLogger m_workerLogger;

public:
	WeichiWorker ( MCTS& engine, std::string host, int port );
	~WeichiWorker();

	// called by main
	static void initialize( MCTS& engine, std::string host, int port );
	static void run();

	// called by other slave threads
	static void write( std::string msg );

private:
	void handle_connect( const boost::system::error_code& error );

	/// write operation
	void do_write( const std::string msg );
	void write_next();
	void handle_write( const boost::system::error_code& error );

	/// read operation
	void start_read();
	void handle_read( const boost::system::error_code& error, size_t bytes_read );
	void handle_command();

	void handle_play( const std::string& line );
	void handle_clearboard();
	void handle_initialize();
	void handle_start( const std::string& line );
	void handle_job( const std::string& line );
	void handle_stop();
	void handle_quit();

	/// close
	void do_close();
};


#endif // _WEICHIWORKER_H_ 
