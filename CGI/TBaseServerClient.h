#ifndef TBASESERVERCLIENT_H
#define TBASESERVERCLIENT_H

#include <boost/asio.hpp>
#include "BasicType.h"
#include "Configure.h"

using boost::asio::ip::tcp;

class TClientUniut {
private:
	string m_sResult;
	tcp::socket m_socket;
	tcp::acceptor::endpoint_type m_end_type;
public:
	TClientUniut( boost::asio::io_service& m_io_service )
		: m_socket(m_io_service) {}

	inline tcp::socket& socket() { return m_socket; }
	inline tcp::acceptor::endpoint_type& getEndPoint() { return m_end_type; }
	inline int getPort() const { return m_end_type.port(); }
	inline string getIP() const { return m_end_type.address().to_string(); }
	inline string getResult() const { return m_sResult; }
	inline void setResult( string sResult ) { m_sResult = sResult; }
	inline void exit()
	{
		boost::system::error_code ec;
		m_socket.shutdown(tcp::socket::shutdown_both,ec);
		m_socket.close();
	}
};

class TBaseServer {
protected:
	vector<TClientUniut*> m_vClient;
	boost::asio::io_service m_io_service;

	const int PORT;
	const int NUM_CLIENT;

public:
	TBaseServer( int port, int numClient )
		: PORT(port), NUM_CLIENT(numClient) {}

	virtual void run()=0;

protected:
	void waitAllClinet()
	{
		tcp::acceptor acceptor(m_io_service,tcp::endpoint(tcp::v4(),PORT));

		CERR() << "wait for all client connect" << endl;

		while( m_vClient.size()<NUM_CLIENT ) {
			m_vClient.push_back(new TClientUniut(m_io_service));
			acceptor.accept(m_vClient.back()->socket(),m_vClient.back()->getEndPoint());

			CERR() << "receive one client from \"" << m_vClient.back()->getIP() << "/"
				<< m_vClient.back()->getPort() << "\", total client="
				<< m_vClient.size() << endl;
		}

		CERR() << "receive all clients" << endl;
	}

	bool writeToClient( TClientUniut* cleint, string sCommand )
	{
		return (sCommand.length()==boost::asio::write(cleint->socket(),boost::asio::buffer(sCommand.c_str(),sCommand.length())));
	}

	void readFromClient( TClientUniut* cleint )
	{
		boost::asio::streambuf streambuf;
		boost::asio::read_until(cleint->socket(),streambuf,'\n');

		boost::asio::streambuf::const_buffers_type bufs = streambuf.data();
		string sResult(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs)+streambuf.size());
		cleint->setResult(sResult);
	}
};

class TBaseClient {
protected:
	int m_clientFD;
	const string SERVER_IP;
	const string SERVER_PORT;
	const int NUM_THREAD;

	boost::asio::io_service m_io_service;
	boost::asio::ip::tcp::socket m_socket;

public:
	TBaseClient( string sIP, string sPort, int numThread )
		: SERVER_IP(sIP)
		, SERVER_PORT(sPort)
		, NUM_THREAD(numThread)
		, m_socket(m_io_service)
	{
	}

	virtual void run()=0;

protected:
	bool connectToServer()
	{
		try {
			boost::asio::ip::tcp::resolver resolver(m_io_service);
			boost::asio::ip::tcp::resolver::query query(tcp::v4(),SERVER_IP.c_str(),SERVER_PORT.c_str());
			boost::asio::connect(m_socket,resolver.resolve(query));
		} catch( std::exception& e ) {
			cerr << e.what() << endl;
			return false;
		}

		return true;
	}

	bool writeToServer( string sResult )
	{
		return (sResult.length()==boost::asio::write(m_socket,boost::asio::buffer(sResult.c_str(),sResult.length())));
	}

	string readFromServer()
	{
		boost::asio::streambuf streambuf;
		boost::asio::read_until(m_socket,streambuf,'\n');

		boost::asio::streambuf::const_buffers_type bufs = streambuf.data();
		string sCommand(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs)+streambuf.size());

		return sCommand;
	}
};

#endif