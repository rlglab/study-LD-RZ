#ifndef WORKER_H
#define WORKER_H

#include "global.h"
#include <map>
#include <string>
#include <cstdio>
#include <stdlib.h>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>

using boost::asio::ip::tcp;

class ThreadUnit {
private:
	int m_id;
	int m_step;
	int m_doMMFeatureGroupIndex;
	boost::barrier m_barStart;
	boost::barrier m_barFinish;
public:
	ThreadUnit( int id, int step )
		: m_id(id)
		, m_step(step)
		, m_barStart(2)
		, m_barFinish(2) {}

	void run();
	inline void startRun() { m_barStart.wait(); }
	inline void finishRun() { m_barFinish.wait(); }
	inline void setMMFeatureGroupIndex( int iFeature ) { m_doMMFeatureGroupIndex = iFeature; }
private:
	void computeVictories();
	void computeLogLikelihood();
	void computeMMDenominator();
	double computeOneTeamDenominator( const Team& team, int iMax, int iMin, map<int,double>& tMul );
};

void ThreadUnit::computeVictories()
{
	g_vVictories[m_id].resize(g_gcol.m_numFeatures);
	g_vParticipations[m_id].resize(g_gcol.m_numFeatures);
	g_vPresences[m_id].resize(g_gcol.m_numFeatures);
	for( int i=0; i<g_gcol.m_numFeatures; i++ ) {
		g_vVictories[m_id][i] = g_vParticipations[m_id][i] = g_vPresences[m_id][i] = 0;
	}

	for( unsigned int i=m_id; i<g_gcol.m_vGame.size(); i+=m_step ) {
		// for winner
		const Team& winner = g_gcol.m_vGame[i].m_winner;
		for( unsigned int j=0; j<winner.getSize(); j++ ) {
			g_vVictories[m_id][winner.getIndex(j)]++;
		}
		
		// for participations
		vector<bool> vTempParticipations;
		vTempParticipations.resize(g_gcol.m_numFeatures,false);
		const vector<Team>& vParticipations = g_gcol.m_vGame[i].m_vParticipants;
		for( unsigned int j=0; j<vParticipations.size(); j++ ) {
			for( unsigned int k=0; k<vParticipations[j].getSize(); k++ ) {
				int iIndex = vParticipations[j].getIndex(k);
				g_vParticipations[m_id][iIndex]++;
				vTempParticipations[iIndex] = true;
			}
		}

		// for presences
		for( int j=0; j<g_gcol.m_numFeatures; j++ ) {
			if( vTempParticipations[j] ) { g_vPresences[m_id][j]++; }
		}
	}
}

void ThreadUnit::computeLogLikelihood()
{
	g_vLogLikelihood[m_id] = 0.0f;
	for( unsigned int i=m_id; i<g_gcol.m_vGame.size(); i+=m_step ) {
		const Game& game = g_gcol.m_vGame[i];

		double dOpponents = 0;
		const vector<Team>& vParticipations = game.m_vParticipants;
		for( unsigned int j=0; j<vParticipations.size(); j++ ) {
			dOpponents += g_gcol.computeTeamGamma(vParticipations[j]);
		}

		g_vLogLikelihood[m_id] += log(g_gcol.computeTeamGamma(game.m_winner));
		g_vLogLikelihood[m_id] -= log(dOpponents);
	}
}

void ThreadUnit::computeMMDenominator()
{
	const int iMax = g_gcol.m_vFeatureIndex[m_doMMFeatureGroupIndex+1];
	const int iMin = g_gcol.m_vFeatureIndex[m_doMMFeatureGroupIndex];

	g_vDen[m_id].resize(g_gcol.m_numFeatures);
	for( unsigned int i=0; i<g_vDen[m_id].size(); i++ ) {
		g_vDen[m_id][i] = 0.0f;
	}

	// compute for each game
	map<int,double> tMul;
	for( unsigned int i=m_id; i<g_gcol.m_vGame.size(); i+=m_step ) {
		tMul.clear();
		double dDen = 0.0;

		// for win team
		dDen += computeOneTeamDenominator(g_gcol.m_vGame[i].m_winner,iMax,iMin,tMul);

		// for participants
		const vector<Team>& vParticipants = g_gcol.m_vGame[i].m_vParticipants;
		for( unsigned int j=0; j<vParticipants.size(); j++ ) {
			dDen += computeOneTeamDenominator(vParticipants[j],iMax,iMin,tMul);
		}

		for( map<int,double>::iterator it=tMul.begin(); it!=tMul.end(); ++it ) {
			int key = it->first;
			g_vDen[m_id][key] += it->second / dDen;
		}
	}
}

double ThreadUnit::computeOneTeamDenominator( const Team& team, int iMax, int iMin, map<int,double>& tMul )
{
	double dProduct = 1.0;
	int iFeatureIndex = -1;

	for( unsigned int i=0; i<team.getSize(); i++ ) {
		int iIndex = team.getIndex(i);
		if( iIndex>=iMin && iIndex<iMax ) { iFeatureIndex = iIndex; }
		else { dProduct *= g_gcol.m_vGamma[iIndex]; }
	}

	if( iFeatureIndex>=0 ) {
		if( tMul.count(iFeatureIndex) ) { tMul[iFeatureIndex] += dProduct; }
		else { tMul[iFeatureIndex] = dProduct; }

		dProduct *= g_gcol.m_vGamma[iFeatureIndex];
	}

	return dProduct;
}

void ThreadUnit::run()
{
	bool bIsOver = false;
	
	while( !bIsOver ) {
		m_barStart.wait();

		if( g_type==TYPE_DOMM ) { computeMMDenominator(); }
		else if( g_type==TYPE_LOGLIKELIHOOD ) { computeLogLikelihood(); }
		else if( g_type==TYPE_COMPUTEVIC ) { computeVictories(); }
		else if( g_type==TYPE_EXIT ) { bIsOver = true; }
		
		m_barFinish.wait();
	}
}

class Client {
private:
	int m_clientFD;
	const string SERVER_IP;
	const string SERVER_PORT;
	const int NUM_THREAD;
	int m_doMMFeatureGroupIndex;
	vector<ThreadUnit*> m_vThreads;
	
	boost::thread_group m_threads;
	boost::asio::io_service m_io_service;
	boost::asio::ip::tcp::socket m_socket;

public:
	Client( string sIP, string sPort, int numThread )
		: SERVER_IP(sIP)
		, SERVER_PORT(sPort)
		, NUM_THREAD(numThread)
		, m_socket(m_io_service)
	{
		g_vDen.resize(numThread);
		g_vLogLikelihood.resize(numThread);
		g_vVictories.resize(numThread);
		g_vParticipations.resize(numThread);
		g_vPresences.resize(numThread);
	}

	void run();

private:
	bool connectToServer();
	void acceptHeaderAndReadGameCollection();
	void createThread();
	string waitForServerData();
	string doCommand( string sCommand );
	void sendResultToServer( string sResult );
	
	string mergeVictoryResult();
	string mergeLogLikelihoodResult();
	string mergeMMDenominatorResult();
	string updateMMGamma( string sUpdate );
	void waitForThreadCompute();

	static void *threadCompute( void *param );
};

void Client::run()
{
	if( !connectToServer() ) {
		cerr << errorColor("Connect failed.") << endl;
		return;
	}
	
	acceptHeaderAndReadGameCollection();
	createThread();
	
	while( true ) {
		string sCommand = waitForServerData();
		string sResult = doCommand(sCommand);
		
		if( g_type==TYPE_EXIT ) { break; }
		
		sendResultToServer(sResult);
	}
}

bool Client::connectToServer()
{
	try {
		boost::asio::ip::tcp::resolver resolver(m_io_service);
		boost::asio::ip::tcp::resolver::query query(tcp::v4(),SERVER_IP.c_str(),SERVER_PORT.c_str());
		boost::asio::connect(m_socket,resolver.resolve(query));
		
		ostringstream oss;
		oss << "=== Connect to server: " << SERVER_IP << "/" << SERVER_PORT << endl;
		cerr << progressColor(oss.str());
	} catch( std::exception& e ) {
		cerr << e.what() << endl;
		return false;
	}
	
	return true;
}

void Client::acceptHeaderAndReadGameCollection()
{
	string sHeader = waitForServerData();
	
	istringstream ss(sHeader);
	
	int numGammas = 0;
	ss >> numGammas;
	g_gcol.m_numFeatures = numGammas;
	g_gcol.m_vGamma.resize(numGammas);
	g_gcol.m_vVictories.resize(numGammas);
	g_gcol.m_vParticipations.resize(numGammas);
	g_gcol.m_vPresences.resize(numGammas);
	for( int i=0; i<numGammas; i++ ) {
		g_gcol.m_vGamma[i] = 1.0f;
		g_gcol.m_vVictories[i] = g_gcol.m_vParticipations[i] = g_gcol.m_vPresences[i] = 0;
	}
	
	g_gcol.m_vFeatureIndex.push_back(0);
	int iFeatures = 0;
	ss >> iFeatures;
	for( int i=0; i<iFeatures; i++ ) {
		int index;
		ss >> index;
		g_gcol.m_vFeatureIndex.push_back(index);
		string sName;
		ss >> sName;
		g_gcol.m_vFeatureName.push_back(sName);
	}	

	ostringstream oss;
	oss << "=== total read " << g_gcol.m_numFeatures << " features";
	cerr << progressColor(oss.str()) << endl;
	
	g_gcol.readGameCollection(cin);
	sendResultToServer("OK\n");
}

void Client::createThread()
{
	for( int i=0; i<NUM_THREAD; i++ ) {
		ThreadUnit* thread = new ThreadUnit(i,NUM_THREAD);
		m_vThreads.push_back(thread);
		m_threads.create_thread(boost::bind(&ThreadUnit::run,thread));
	}
}

string Client::waitForServerData()
{
	boost::asio::streambuf streambuf;
	boost::asio::read_until(m_socket,streambuf,'\n');
		
	boost::asio::streambuf::const_buffers_type bufs = streambuf.data();
	string sCommand(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs)+streambuf.size());
	
	ostringstream oss;
	oss << "<< receive command \"" << sCommand.substr(0,30) << "\"" << endl; 
	cerr << receieveColor(oss.str());
	
	return sCommand;
}

string Client::doCommand( string sCommand )
{
	if( sCommand.find("compute_victory")!=string::npos ) {
		g_type = TYPE_COMPUTEVIC;
		return mergeVictoryResult();
	} else if( sCommand.find("compute_loglikelihood")!=string::npos ) {
		g_type = TYPE_LOGLIKELIHOOD;
		return mergeLogLikelihoodResult();
	} else if( sCommand.find("compute_oneMMIteration")!=string::npos ) {
		g_type = TYPE_DOMM;
		m_doMMFeatureGroupIndex = atoi(sCommand.substr(sCommand.find("compute_oneMMIteration")+23).c_str());
		return mergeMMDenominatorResult();
	} else if( sCommand.find("update_Gammas")!=string::npos ) {
		g_type = TYPE_UPDATE_GAMMA;
		return updateMMGamma(sCommand.substr(sCommand.find("update_Gammas")+14));
	} else if( sCommand.find("exit")!=string::npos ) {
		g_type = TYPE_EXIT;
		return "";
	} else {
		cerr << errorColor("error type!") << endl;
		g_type = TYPE_ERROR;
		return "error type!\n";
	}
}

void Client::sendResultToServer( string sResult )
{
	if( sResult.length()!=boost::asio::write(m_socket,boost::asio::buffer(sResult.c_str(),sResult.length())) ) {
		cerr << errorColor("send error") << endl;
	}
	cerr << sendColor(">> send result to server") << endl << endl;
}

string Client::mergeVictoryResult()
{
	waitForThreadCompute();
	
	ostringstream oss;
	oss << g_gcol.m_vGame.size();
	
	for( int i=0; i<g_gcol.m_numFeatures; i++ ) {
		g_gcol.m_vVictories[i] = g_gcol.m_vParticipations[i] = g_gcol.m_vPresences[i] = 0;
		for( int j=0; j<NUM_THREAD; j++ ) {
			g_gcol.m_vVictories[i] += g_vVictories[j][i];
			g_gcol.m_vParticipations[i] += g_vParticipations[j][i];
			g_gcol.m_vPresences[i] += g_vPresences[j][i];
		}
		
		oss << ' ' << g_gcol.m_vVictories[i] << ' ' << g_gcol.m_vParticipations[i] << ' ' << g_gcol.m_vPresences[i];
	}
	
	oss << endl;

	return oss.str();
}

string Client::mergeLogLikelihoodResult()
{	
	waitForThreadCompute();
	
	double dResult = 0.0f;
	for( int i=0; i<NUM_THREAD; i++ ) { dResult += g_vLogLikelihood[i]; }

	ostringstream oss;
	oss << dResult << endl;
	return oss.str();
}

string Client::mergeMMDenominatorResult()
{
	for( int i=0; i<NUM_THREAD; i++ ) { m_vThreads[i]->setMMFeatureGroupIndex(m_doMMFeatureGroupIndex); }
	
	waitForThreadCompute();
	
	ostringstream oss;
	for( int i=0; i<g_gcol.m_numFeatures; i++ ) {
		double dDen = 0.0f;
		for( int j=0; j<NUM_THREAD; j++ ) { dDen += g_vDen[j][i]; }
		
		if( i>0 ) { oss << ' '; }
		oss << dDen;
	}
	oss << endl;

	return oss.str();
}

string Client::updateMMGamma( string sUpdate )
{
	int iFeature;
	stringstream ss(sUpdate);
	ss >> iFeature;

	// interval for this feature
	const int iMax = g_gcol.m_vFeatureIndex[iFeature+1];
	const int iMin = g_gcol.m_vFeatureIndex[iFeature];

	// update gammas
	for( int i=iMin; i<iMax; i++ ) {
		ss >> g_gcol.m_vGamma[i];
	}

	return "OK\n";
}

void Client::waitForThreadCompute()
{
	for( int i=0; i<NUM_THREAD; i++ ) { m_vThreads[i]->startRun(); }
	for( int i=0; i<NUM_THREAD; i++ ) { m_vThreads[i]->finishRun(); }
}

#endif
