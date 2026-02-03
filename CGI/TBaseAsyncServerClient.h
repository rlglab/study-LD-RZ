#ifndef TBASEASYNCSERVERCLIENT_H
#define TBASEASYNCSERVERCLIENT_H

#include <string>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "ColorMessage.h"

using boost::asio::ip::tcp;

class TBaseSession
{
private:
	tcp::socket m_socket;

protected:
	boost::asio::streambuf m_buffer;

public:
	TBaseSession(boost::asio::io_service& io_service)
		: m_socket(io_service)
	{
	}

	void start_read()
	{
		boost::asio::async_read_until(m_socket,
			m_buffer, '\n', 
			boost::bind(&TBaseSession::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

	inline tcp::socket& socket() { return m_socket; }
	inline string getIP() const { return m_socket.remote_endpoint().address().to_string(); }
	inline unsigned short getPort() const { return m_socket.remote_endpoint().port(); }

private:
	void handle_read( const boost::system::error_code& error, size_t bytes_transferred )
	{
		if( !error ) {
			std::istream is(&m_buffer);
			std::string sLine;
			std::getline(is, sLine);
			
			handle_read(sLine);
			start_read();
		} else {
			handle_disconnect();
			delete this;
		}
	}

	void handle_write( const boost::system::error_code& error )
	{
		if( !error ) {
			handle_write();
		} else {
			handle_disconnect();
			delete this;
		}
	}

protected:
	void write( std::string sMessage, bool bDisplayMessage=false )
	{
		if( bDisplayMessage ) {
			if( sMessage.back()=='\n' ) { sMessage.pop_back(); }
			string sDisplayMessage = ">> sent \"" + sMessage + "\" to client.";
			CERR() << getColorMessage(sDisplayMessage,ANSITYPE_BOLD,ANSICOLOR_GREEN,ANSICOLOR_BLACK) << endl;
		}

		if( sMessage[sMessage.length()-1]!='\n' ) { sMessage.append("\n"); }
		boost::asio::async_write(m_socket,
			boost::asio::buffer(sMessage),
			boost::bind(&TBaseSession::handle_write, this, boost::asio::placeholders::error));
	}

	virtual void handle_read( std::string sResult )=0;
	virtual void handle_write()=0;
	virtual void handle_disconnect()=0;
};

template<class _Session> class TBaseAsyncServer {
protected:
	boost::asio::io_service m_io_service;
	tcp::acceptor m_acceptor;

public:
	TBaseAsyncServer( short port )
		: m_acceptor(m_io_service, tcp::endpoint(tcp::v4(), port))
	{
	}

	inline void run() {
		initialize();
		start_accept();
		m_io_service.run();
	}

private:
	void start_accept()
	{
		_Session* new_session = getNewSession();
		m_acceptor.async_accept(new_session->socket(),
			boost::bind(&TBaseAsyncServer::handle_accept, this, new_session,
			boost::asio::placeholders::error));
	}

	void handle_accept( _Session* new_session, const boost::system::error_code& error )
	{
		if( !error ) {
			handle_accept(new_session);
			new_session->start_read();
		} else {
			delete new_session;
		}

		start_accept();
	}

protected:
	virtual void initialize()=0;
	virtual _Session* getNewSession()=0;
	virtual void handle_accept( _Session* new_session )=0;
};

class TBaseAsyncClient {
protected:
	const std::string SERVER_IP;
	const std::string SERVER_PORT;

	boost::asio::io_service m_io_service;
	boost::asio::ip::tcp::socket m_socket;

public:
	TBaseAsyncClient( std::string sIP, std::string sPort )
		: SERVER_IP(sIP)
		, SERVER_PORT(sPort)
		, m_socket(m_io_service)
	{
	}

	virtual void run()=0;

	bool connectToServer()
	{
		try {
			boost::asio::ip::tcp::resolver resolver(m_io_service);
			boost::asio::ip::tcp::resolver::query query(tcp::v4(),SERVER_IP.c_str(),SERVER_PORT.c_str());
			boost::asio::connect(m_socket,resolver.resolve(query));
		} catch( std::exception& e ) {
			CERR() << e.what() << endl;
			return false;
		}

		return true;
	}

	bool writeToServer( std::string sResult, bool bDisplayMessage=false )
	{
		if( bDisplayMessage ) {
			if( sResult.back()=='\n' ) { sResult.pop_back(); }
			string sMessage = ">> sent \"" + sResult + "\" to server.";
			CERR() << getColorMessage(sMessage,ANSITYPE_BOLD,ANSICOLOR_GREEN,ANSICOLOR_BLACK) << endl;
		}

		if( sResult[sResult.length()-1]!='\n' ) { sResult.append("\n"); }
		return (sResult.length()==boost::asio::write(m_socket,boost::asio::buffer(sResult.c_str(),sResult.length())));
	}

	std::string readFromServer( bool bDisplayMessage=false )
	{
		boost::asio::streambuf streambuf;
		boost::asio::read_until(m_socket,streambuf,'\n');

		boost::asio::streambuf::const_buffers_type bufs = streambuf.data();
		std::string sCommand(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs)+streambuf.size());

		if( sCommand.back()=='\n' ) { sCommand.pop_back(); }	// remove last char
		if( bDisplayMessage ) {
			string sMessage = "<< receive \"" + sCommand + "\" from server.";
			CERR() << getColorMessage(sMessage,ANSITYPE_BOLD,ANSICOLOR_BLUE,ANSICOLOR_BLACK) << endl;
		}

		return sCommand;
	}

	inline string getIP() const { return m_socket.remote_endpoint().address().to_string(); }
	inline unsigned short getPort() const { return m_socket.remote_endpoint().port(); }
};

#endif