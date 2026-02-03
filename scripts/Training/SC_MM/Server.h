#ifndef SERVER_H
#define SERVER_H

#include "Basic.h"
#include "global.h"
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class ClientUnit {
private:
	string m_sResult;
	tcp::socket m_socket;
	tcp::acceptor::endpoint_type m_end_type;
public:
	ClientUnit( boost::asio::io_service& m_io_service )
		: m_socket(m_io_service) {}

	inline tcp::socket& socket() { return m_socket; }
	inline tcp::acceptor::endpoint_type& getEndPoint() { return m_end_type; }
	inline int getPort() const { return m_end_type.port(); }
	inline string getIP() const { return m_end_type.address().to_string(); }
	inline string getResult() const { return m_sResult; }
	inline void exit()
	{
		boost::system::error_code ec;
		m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both,ec);
		m_socket.close();
	}

	inline void setResult( string sResult ) { m_sResult = sResult; }
};

class Server {
private:
	vector<ClientUnit*> m_vClient;
	boost::asio::io_service m_io_service;
	
	const int PORT;
	const int NUM_CLIENT;
	static const int MAX_ITERATION = 40;

public:
	Server( int port, int numClient )
		: PORT(port)
		, NUM_CLIENT(numClient) { g_start = time(NULL); }

	void run();

private:
	// for socket used
	void waitForClientConnect();
	void sendHeaderToClient( tcp::socket& socket, bool bNeedToWaitClientResult=true );
	void broadcastCommand( string sCommand );
	void waitForClientResult();
	void disconnectAllClient();

	void mmUserFriendlyProgress( int numIteration, int iFeature, vector<double>& vDelta, double dLogLikelihood );
	void writeRatingsAndDB( int iteration );
	
	// for algorithm
	void computeVictory();
	double computeLogLikelihood();
	void doOneMMIteration( int iFeature );
};

void Server::run()
{
	g_gcol.readFeatureInfo(cin);
	
	waitForClientConnect();
	computeVictory();

	// calculate likelihood
	double dLogLikelihood = computeLogLikelihood() / g_gcol.m_numGames;

	vector<double> vDelta;
	const int iNumFeatureGroups = g_gcol.m_vFeatureName.size();
	for( int iNum=1; iNum<=MAX_ITERATION; iNum++ ) {
		vDelta.clear();
		vDelta.resize(iNumFeatureGroups,10);

		while( true ) {
			// select feature with max delta
			int iFeature = 0;
			double dMaxDelta = vDelta[0];
			for( int i=0; i<iNumFeatureGroups; i++ ) {
				if( vDelta[i]>dMaxDelta ) { dMaxDelta = vDelta[iFeature=i]; }
			}
			if( dMaxDelta<0.0001 ) { break; }

			mmUserFriendlyProgress(iNum,iFeature,vDelta,dLogLikelihood);

			// run one MM iteration over this feature
			doOneMMIteration(iFeature);
			double dNewLogLikelihood = computeLogLikelihood() / g_gcol.m_numGames;
			vDelta[iFeature] = dNewLogLikelihood - dLogLikelihood;
			dLogLikelihood = dNewLogLikelihood;
		}
		
		// write training result
		writeRatingsAndDB(iNum);
	}

	// end training, close all client
	disconnectAllClient();
}

void Server::mmUserFriendlyProgress( int numIteration, int iFeature, vector<double>& vDelta, double dLogLikelihood )
{
	cerr << "\033[2J\033[1;1H";
	cerr << progressColor("=================================================================================================================================================================") << endl;
	ostringstream oss;
	oss << "MM Iteration: " << numIteration << "/" << MAX_ITERATION << endl;
	cerr << progressColor(oss.str());

	int iFinish = 0;
	const int iNumFeatureGroups = g_gcol.m_vFeatureName.size();
	for( int i=0; i<iNumFeatureGroups; i++ ) {
		ostringstream oss;
		oss << setw(40) << g_gcol.m_vFeatureName[i] << ' ' << setw(15) << vDelta[i] << '\t';

		if( vDelta[i]<0.0001 ) { cerr << finishColor(oss.str()); ++iFinish; }
		else if( i==iFeature ) { cerr << errorColor(oss.str()); }
		else { cerr << progressColor(oss.str()); }

		if( i>0 && i%3==2 ) { cerr << endl; }
	}

	if( iNumFeatureGroups%3!=0 ) { cerr << endl; }
	oss.str("");
	oss.clear();
	time_t finish = time(NULL);
	oss << "during time = " << showTime(static_cast<int>(difftime(finish,g_start))) << endl;
	oss << "finish/total: " << iFinish << "/" << iNumFeatureGroups << endl;
	oss << "LogLikelihood= " << setw(15) << dLogLikelihood << "; exp= " << setw(15) << exp(-dLogLikelihood);
	cerr << receieveColor(oss.str()) << endl;
	
	cerr << progressColor("=================================================================================================================================================================") << endl;
}

void Server::waitForClientConnect()
{
	cerr << progressColor("=== Server setup ready, wait for client to connect.") << endl;

	tcp::acceptor acceptor(m_io_service,tcp::endpoint(tcp::v4(),PORT));
	
	while( true ) {
		m_vClient.push_back(new ClientUnit(m_io_service));
		acceptor.accept(m_vClient.back()->socket(),m_vClient.back()->getEndPoint());
	
		ostringstream oss;
		oss << "<< receive one client from \"" << m_vClient.back()->getIP() << "/"
											   << m_vClient.back()->getPort() << "\", total client="
											   << m_vClient.size() << endl;
		cerr << receieveColor(oss.str());
		
		sendHeaderToClient(m_vClient.back()->socket(),false);
		cerr << sendColor(">> send header to client") << endl;
	
		if( (int)m_vClient.size()==NUM_CLIENT ) { break; }
	}

	waitForClientResult();
	cerr << progressColor("=== all client is connected and finish reading data!!") << endl << endl;
}

void Server::sendHeaderToClient( tcp::socket& socket, bool bNeedToWaitClientResult/*=true*/ )
{
	ostringstream oss;
	
	oss << g_gcol.m_vGamma.size() << ' ' << g_gcol.m_vFeatureName.size();
	for( unsigned int i=0; i<g_gcol.m_vFeatureName.size(); i++ ) {
		oss << ' ' << g_gcol.m_vFeatureIndex[i+1] << ' ' << g_gcol.m_vFeatureName[i];
	}
	oss << endl;
	
	if( oss.str().length()!=boost::asio::write(socket,boost::asio::buffer(oss.str().c_str(),oss.str().length())) ) {
		cerr << errorColor("send error") << endl;
	}
	
	if( bNeedToWaitClientResult ) { waitForClientResult(); }
}

void Server::broadcastCommand( string sCommand )
{
	// send command to each client
	string s = sCommand;
	if( sCommand.find(" ")!=string::npos ) { s = sCommand.substr(0,sCommand.find(" ")); }
	ostringstream oss;
	oss << ">> broadcast command \"" << s << "\" to each client" << endl;
	cerr << sendColor(oss.str());
	
	sCommand.append(1,'\n');
	for( unsigned int i=0; i<m_vClient.size(); i++ ) {
		if( sCommand.length()!=boost::asio::write(m_vClient[i]->socket(),boost::asio::buffer(sCommand.c_str(),sCommand.length())) ) {
			cerr << errorColor("send error") << endl;
		}
	}
}

void Server::waitForClientResult()
{
	// wait for client result
	for( unsigned int i=0; i<m_vClient.size(); i++ ) {
		boost::asio::streambuf streambuf;
		boost::asio::read_until(m_vClient[i]->socket(),streambuf,'\n');
		
		boost::asio::streambuf::const_buffers_type bufs = streambuf.data();
		string sResult(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs)+streambuf.size());
		m_vClient[i]->setResult(sResult);
		
		ostringstream oss;
		oss << "<< receive client \"" << m_vClient[i]->getIP() << "/" << m_vClient[i]->getPort() << "\" result"
			<< "; receive/total = " << (i+1) << "/" << NUM_CLIENT << "       \r";
		cerr << receieveColor(oss.str());
	}

	cerr << progressColor("=== receive all client result                                                  ") << endl << endl;
}

void Server::disconnectAllClient()
{	
	broadcastCommand("exit");
	for( unsigned int i=0; i<m_vClient.size(); i++ ) {
		m_vClient[i]->exit();
	}
}

void Server::writeRatingsAndDB( int iteration )
{
	std::stringstream ss;
	ss << "mm-with-freq_iter" << iteration << ".dat";
	
	ofstream ofs(ss.str().c_str());
	for( unsigned int i=0; i<g_gcol.m_vGamma.size(); i++ ) {
		ofs << std::setw(3) << i << ' ' << std::setw(10) << g_gcol.m_vGamma[i] << ' ';
		ofs << std::setw(11) << g_gcol.m_vVictories[i];
		ofs << std::setw(11) << g_gcol.m_vParticipations[i];
		ofs << std::setw(11) << g_gcol.m_vPresences[i];
		ofs << '\n';
	}

	ss.str("");
	ss << "MM_weight_iter" << iteration << ".db";
	ofstream fout(ss.str().c_str(),std::ios::out|std::ios::binary);
	fout.write(reinterpret_cast<char*>(g_gcol.m_vGamma.data()), sizeof(double)*g_gcol.m_vGamma.size());
	fout.close();
}

void Server::computeVictory()
{
	broadcastCommand("compute_victory");
	waitForClientResult();

	// compute victory & send to each worker
	g_gcol.m_numGames = 0;
	for( unsigned int i=0; i<m_vClient.size(); i++ ) {
		string sResult = m_vClient[i]->getResult();
		stringstream ss(sResult);

		int numGames;
		ss >> numGames;
		g_gcol.m_numGames += numGames;
		for( int j=0; j<g_gcol.m_numFeatures; j++ ) {
			double dVictories, dParticipations, dPresences;
			dVictories = dParticipations = dPresences = 0.0f;
			ss >> dVictories >> dParticipations >> dPresences;
			
			g_gcol.m_vVictories[j] += dVictories;
			g_gcol.m_vParticipations[j] += dParticipations;
			g_gcol.m_vPresences[j] += dPresences;
		}
	}
}

double Server::computeLogLikelihood()
{
	broadcastCommand("compute_loglikelihood");
	waitForClientResult();

	double dLogLikelihood = 0.0f;
	for( unsigned int i=0; i<m_vClient.size(); i++ ) {
		double dResult = 0.0f;
		string sResult = m_vClient[i]->getResult();
		stringstream ss(sResult);
		ss >> dResult;
		dLogLikelihood += dResult;
	}
	return dLogLikelihood;
}

void Server::doOneMMIteration( int iFeature )
{
	ostringstream oss;
	oss << "compute_oneMMIteration " << iFeature;
	broadcastCommand(oss.str());
	waitForClientResult();

	const double dPriorVictories = 1.0;
	const double dPriorGames = 2.0;
	const double dPriorOpponentGamma = 1.0;

	// interval for this feature
	const int iMax = g_gcol.m_vFeatureIndex[iFeature+1];
	const int iMin = g_gcol.m_vFeatureIndex[iFeature];

	vector<double> vDen(g_gcol.m_numFeatures,0.0);
	// collect denominator from each worker
	for( unsigned int i=0; i<m_vClient.size(); i++ ) {
		string sResult = m_vClient[i]->getResult();
		stringstream ss(sResult);
		for( int j=0; j<g_gcol.m_numFeatures; j++ ) {
			double dDen = 0.0f;
			ss >> dDen;
			vDen[j] += dDen;
		}
	}

	// update gammas & broadcast to each client
	oss.str("");
	oss.clear();
	oss << "update_Gammas " << iFeature;
	for( int i=iMin; i<iMax; i++ ) {
		double dNewGamms = (g_gcol.m_vVictories[i] + dPriorVictories) / 
						   (vDen[i] + dPriorGames / (g_gcol.m_vGamma[i] + dPriorOpponentGamma));
		g_gcol.m_vGamma[i] = dNewGamms;
		oss << ' ' << dNewGamms;
	}
	broadcastCommand(oss.str());
	waitForClientResult();
}

#endif
