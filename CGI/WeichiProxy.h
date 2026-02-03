#ifndef __WEICHIPROXY_H__
#define __WEICHIPROXY_H__

#include <iostream>
#include <sstream>
#include <string>
#include <queue>
#include <vector>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "Logger.h"

using boost::asio::ip::tcp;

class WeichiProxy
{

private:
	boost::asio::io_service m_io_service ;
	tcp::acceptor m_acceptor ;
	tcp::socket m_socket;

	boost::asio::streambuf m_inputBuffer;
	boost::asio::streambuf m_readBuffer;

#if defined(_WIN32) || defined(_WIN64) || defined(_WIN32_WINNT)
	boost::asio::windows::stream_handle m_input;
#else
	boost::asio::posix::stream_descriptor m_input;
#endif

public:
	WeichiProxy( int port )
		: m_acceptor(m_io_service, tcp::endpoint(tcp::v4(), port))
		, m_socket(m_io_service)
#if defined(_WIN32) || defined(_WIN64) || defined(_WIN32_WINNT)
		, m_input(m_io_service, GetStdHandle(STD_INPUT_HANDLE))
#else
		, m_input(m_io_service, ::dup(STDIN_FILENO))
#endif
	{
		CERR() << "CGI proxy is running (port: " << port << ")." << std::endl;
	}

	void run()
	{
		start_accept();
		m_io_service.run();
	}

private:
	void start_accept()
	{
		m_acceptor.async_accept(m_socket,
			boost::bind(&WeichiProxy::handle_accept, this, boost::asio::placeholders::error) );
	}

	void handle_accept(const boost::system::error_code& error)
	{
		if (!error) {
			boost::asio::async_read_until(m_socket,
				m_readBuffer, '\n', 
				boost::bind(&WeichiProxy::handle_read, this,
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

			boost::asio::async_read_until(m_input,
				m_inputBuffer, '\n', 
				boost::bind(&WeichiProxy::handle_input, this,
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
	}

	void handle_input(const boost::system::error_code& error, size_t bytes_read)
	{
		if ( error ) {
			CERR() << error.message() << std::endl;
		}
		else {
			boost::asio::async_write(m_socket,
				m_inputBuffer, 
				boost::bind(&WeichiProxy::handle_write, this, boost::asio::placeholders::error));

			boost::asio::async_read_until(m_input,
				m_inputBuffer, '\n', 
				boost::bind(&WeichiProxy::handle_input, this,
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
	}

	void handle_write(const boost::system::error_code& error)
	{
		if ( error ) {
			CERR() << "handle_write error" << std::endl;
			m_io_service.stop();
		}
	}

	void handle_read(const boost::system::error_code& error, size_t bytes_read)
	{
		if ( error ) {
			if ((error == boost::asio::error::eof) || (error == boost::asio::error::connection_reset)) {
				CERR() << "client disconnected" << std::endl;
			} else {
				CERR() << "handle_read error" << std::endl;
			}
			m_io_service.stop();
		} else {
			handle_output();
			boost::asio::async_read_until(m_socket,
				m_readBuffer, '\n', 
				boost::bind(&WeichiProxy::handle_read, this,
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
	}

	void handle_output()
	{
		istream is(&m_readBuffer) ;
		std::string line ;
		std::getline(is, line);

		COUT() << line << endl;
	}

};

#endif  // __WEICHIPROXY_H__ 
