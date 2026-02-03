#ifndef __WEICHICNNSERVER_H__
#define __WEICHICNNSERVER_H__

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "WeichiGtpEngine.h"

using boost::asio::ip::tcp;

class WeichiCNNServer
{
private:
	MCTS m_mcts;
	WeichiGtpEngine m_engine;

	boost::thread m_cinThread;
	boost::asio::io_service m_io_service;
	boost::asio::io_service::work m_work;
	boost::asio::deadline_timer m_checkPoolTimer;

public:
	WeichiCNNServer()
		: m_engine(Logger::getTeeOut(), m_mcts)
		, m_work(m_io_service)
		, m_checkPoolTimer(m_io_service)
	{
		if ( Configure::Pondering ) {
			start_checkpool();
		}
		m_cinThread = boost::thread(&WeichiCNNServer::read_input, this);
	}

	~WeichiCNNServer()
	{
		m_cinThread.join();
	}

	MCTS& getMCTS()
	{
		return m_mcts;
	}

	void run()
	{
		CERR() << "Program start successfully." << endl;
		m_io_service.run();
	}

private:
	// this function runs on another thread
	void read_input()
	{
		std::string line;
		while ( getline(cin, line) ) {
			boost::algorithm::trim(line);
			this->m_io_service.post(boost::bind(&WeichiCNNServer::handle_input, this, line));
			if ( line == "quit" ) { break; }
		}
		if ( line != "quit" ) {
			// quit by EOF, send quit to engine
			line = "quit";
			this->m_io_service.post(boost::bind(&WeichiCNNServer::handle_input, this, line));
		}
	}

	void handle_input(const std::string line)
	{
		m_engine.runCommand(line);
		if ( m_engine.hasQuit() ) {
			m_io_service.stop();
		}
	}

	void start_checkpool()
	{
		m_checkPoolTimer.expires_from_now(boost::posix_time::milliseconds(10)) ;
		m_checkPoolTimer.async_wait(boost::bind(&WeichiCNNServer::handle_checkpool, this));
	}

	void handle_checkpool()
	{
		m_mcts.ponder_checkpool();
		start_checkpool();
	}
};

#endif 
