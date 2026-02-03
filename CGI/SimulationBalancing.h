#ifndef SIMULATIONBALANCING_H
#define SIMULATIONBALANCING_H

#include "BasicType.h"
#include "SgfLoader.h"
#include "TBaseMasterSlave.h"
#include "TBaseServerClient.h"
#include "WeichiThreadState.h"
#include "WeichiPlayoutAgent.h"
#include "FileDirectoryExplorer.h"

class SBSharedData {
public:
	SpinLock m_lock;
	int m_simulation;
	double m_avgValue;
	bool m_bIsCalculateGradient;
	vector<WeichiMove> m_vMoves;

	static const int MAX_SIMULATION_TIMES = 512;
};

class SBSlave: public TBaseSlave<SBSharedData> {
private:
	WeichiThreadState m_state;
	WeichiPlayoutAgent m_agent;

	double m_dValue;
	Vector<double,MAX_NUM_PLAYOUT_FEATURES> m_vGradients;

public:
	SBSlave( int id, SBSharedData& sharedData )
		: TBaseSlave(id,sharedData)
		, m_agent(m_state)
	{}

	inline double getValue() { return m_dValue; }
	inline const Vector<double,MAX_NUM_PLAYOUT_FEATURES>& getGradient() const { return m_vGradients; }

private:
	virtual void initialize() { TBaseSlave::initialize(); }
	virtual bool isOver() { return false; }

	virtual void reset()
	{
		m_dValue = 0;
		m_vGradients.setAllAs(0.0f,MAX_NUM_PLAYOUT_FEATURES);

		m_state.resetThreadState();
		for( uint i=0; i<m_sharedData.m_vMoves.size(); i++ ) { m_state.play(m_sharedData.m_vMoves[i]); }
		m_state.m_rootFilter.startFilter();
		m_state.backup();
	}

	virtual void doSlaveJob()
	{
		Color turnColor = m_state.m_board.getToPlay();
		m_agent.m_bCalculateGradient = m_sharedData.m_bIsCalculateGradient;
		while( !isEnd() ) {
			m_state.startPlayout();
			if( m_sharedData.m_bIsCalculateGradient ) { m_agent.m_vGradients.setAllAs(0.0f,MAX_NUM_PLAYOUT_FEATURES); }

			WeichiPlayoutResult result = m_agent.run();
			if( !m_sharedData.m_bIsCalculateGradient ) { m_dValue += (result.getWinner()==turnColor); }

			m_state.endPlayout();
			m_state.rollback();

			if( m_sharedData.m_bIsCalculateGradient ) {
				double dResult = (result.getWinner()==turnColor)? 1: 0;
				for( uint i=0; i<MAX_NUM_PLAYOUT_FEATURES; i++ ) {
					m_vGradients[i] += (dResult-m_sharedData.m_avgValue) * m_agent.m_vGradients[i];
					//m_vGradients[i] += fabs(dResult-m_sharedData.m_avgValue) * m_agent.m_vGradients[i];
					//m_vGradients[i] += dResult * m_agent.m_vGradients[i];
					/*if( i==2066 ) {
						cerr << m_vGradients[i] << " " << (dResult-m_sharedData.m_avgValue) * m_agent.m_vGradients[i] << ", ";
						cerr << dResult << " " << m_sharedData.m_avgValue << " " << m_agent.m_vGradients[i] << endl;
					}*/
				}
			}
		}
	}

	bool isEnd()
	{
		m_sharedData.m_lock.lock();
		bool bIsEnd = (m_sharedData.m_simulation>=m_sharedData.MAX_SIMULATION_TIMES);
		if( !bIsEnd ) { m_sharedData.m_simulation++; }
		m_sharedData.m_lock.unlock();

		return bIsEnd;
	}
};

class SBMaster: public TBaseMaster<SBSharedData,SBSlave> {
private:
	double m_dValue;
	double m_dError;
	double m_dAnswerValue;
	Vector<double,MAX_NUM_PLAYOUT_FEATURES> m_vGradients;
	Vector<double,MAX_NUM_PLAYOUT_FEATURES> m_vAccmulateGradients;

	static const int MAX_MOVES = 250;

public:
	SBMaster()
		: TBaseMaster(Configure::NumThread)
	{
		initialize();
	}

	void reset()
	{
		m_dError = 0.0f;
		m_vAccmulateGradients.setAllAs(0.0f,MAX_NUM_PLAYOUT_FEATURES);
	}

	void randomPickOnePosition( string sSgfString )
	{
		SgfLoader sgfLoader;
		sgfLoader.parseFromString(sSgfString,MAX_MOVES);

		m_sharedData.m_vMoves = sgfLoader.getPlayMove();
		m_sharedData.m_vMoves.resize(Random::nextInt(static_cast<int>(m_sharedData.m_vMoves.size())));
		m_dAnswerValue = (sgfLoader.getWinner()==AgainstColor(m_sharedData.m_vMoves.back().getColor()));
	}

	void calculateGradient( string sSgfString )
	{
		randomPickOnePosition(sSgfString);

		m_sharedData.m_simulation = 0;
		m_sharedData.m_bIsCalculateGradient = false;
		for( int i=0; i<m_nThread; i++ ) { m_vSlaves[i]->startRun(); }
		for( int i=0; i<m_nThread; i++ ) { m_vSlaves[i]->finishRun(); }
		summarizeSlavesData();

		m_sharedData.m_simulation = 0;
		m_sharedData.m_bIsCalculateGradient = true;
		for( int i=0; i<m_nThread; i++ ) { m_vSlaves[i]->startRun(); }
		for( int i=0; i<m_nThread; i++ ) { m_vSlaves[i]->finishRun(); }
		summarizeSlavesData();

		// calculate gradient
		m_dError += pow((m_dAnswerValue-m_dValue),static_cast<double>(2.0f));
		for( uint i=0; i<MAX_NUM_PLAYOUT_FEATURES; i++ ) {
			m_vAccmulateGradients[i] += (m_dAnswerValue-m_dValue) * m_vGradients[i];
		}
	}

	inline double getValue() const { return m_dValue; }
	inline double getError() const { return m_dError; }
	inline double getAnswerValue() const { return m_dAnswerValue; }
	inline const Vector<double,MAX_NUM_PLAYOUT_FEATURES>& getGradients() const { return m_vGradients; }
	inline const Vector<double,MAX_NUM_PLAYOUT_FEATURES>& getAccmulateGradients() const { return m_vAccmulateGradients; }

private:
	virtual bool initialize()
	{
		reset();
		Random::reset(Configure::RandomSeed);
		return TBaseMaster::initialize();
	}
	virtual SBSlave* newSlave( int id ) { return new SBSlave(id,m_sharedData); }

	virtual void summarizeSlavesData()
	{
		if( !m_sharedData.m_bIsCalculateGradient ) {
			m_sharedData.m_avgValue = 0;
			for( int i=0; i<m_nThread; i++ ) { m_sharedData.m_avgValue += m_vSlaves[i]->getValue(); }
			m_sharedData.m_avgValue /= m_sharedData.MAX_SIMULATION_TIMES;
			m_dValue = m_sharedData.m_avgValue;
		} else {
			m_vGradients.setAllAs(0.0f,MAX_NUM_PLAYOUT_FEATURES);
			for( uint i=0; i<MAX_NUM_PLAYOUT_FEATURES; i++ ) {
				for( int j=0; j<m_nThread; j++ ) { m_vGradients[i] += m_vSlaves[j]->getGradient()[i]; }
				m_vGradients[i] /= m_sharedData.MAX_SIMULATION_TIMES;
			}
		}
	}
};

class SBServer: public TBaseServer {
private:
	int m_iteration;
	int m_batch_size;
	FileDirectoryExplorer m_dirExplorer;

	static const int MAX_MOVES = 250;
	static const int MAX_BATCH_SIZE = 128;

	// parameters
	bool m_bUseSGD;
	double m_dBeta1;
	double m_dBeta2;
	double m_dAlpha;
	double m_dLambda;
	double m_dEpsilon;
	Vector<StatisticData,MAX_MOVES> m_vError;
	Vector<double,MAX_NUM_PLAYOUT_FEATURES> m_vM;
	Vector<double,MAX_NUM_PLAYOUT_FEATURES> m_vV;
	Vector<double,MAX_NUM_PLAYOUT_FEATURES> m_vTheta;
	Vector<double,MAX_NUM_PLAYOUT_FEATURES> m_vTheta0;

public:
	SBServer( int port, int numClinet )
		: TBaseServer(port,numClinet) {}

	void run()
	{
		initialize();
		waitAllClinet();

		m_iteration = 1;
		while( true ) {
			CERR() << "Iteration " << m_iteration << ": " << endl;
			broadcastTheta();
			calculateGradient();
			updateTheta();
			m_iteration++;
			CERR() << endl;
		}
	}

	bool initialize()
	{
		// set directory
		if( WeichiConfigure::caffe_train_dir!="" ) {
			m_dirExplorer.reset();
			m_dirExplorer.setRootDirectory(WeichiConfigure::caffe_train_dir);
			CERR() << "Open training directory " << WeichiConfigure::caffe_train_dir << endl;
		} else {
			CERR() << "Couldn't find training directory " << WeichiConfigure::caffe_train_dir << endl;
			return false;
		}

		// set parameters
		m_bUseSGD = true;
		m_dBeta1 = 0.9;
		m_dBeta2 = 0.999;
		m_dAlpha = 10;//0.01;
		m_dLambda = 0.0001;//0.0001;
		m_dEpsilon = 0.00000001;
		m_vM.setAllAs(0.0f,MAX_NUM_PLAYOUT_FEATURES);
		m_vV.setAllAs(0.0f,MAX_NUM_PLAYOUT_FEATURES);
		m_vError.setAllAs(StatisticData(),MAX_MOVES);
		m_vTheta.setAllAs(0.0f,MAX_NUM_PLAYOUT_FEATURES);
		m_vTheta0.setAllAs(0.0f,MAX_NUM_PLAYOUT_FEATURES);
		for( uint i=0; i<MAX_NUM_PLAYOUT_FEATURES; i++ ) {
			m_vTheta[i] = m_vTheta0[i] = log(WeichiGammaTable::getPlayoutFeatureValue(i));
		}

		return true;
	}

	void calculateGradient()
	{
		CERR() << "calculate gradient ... ";

		m_batch_size = 0;
		bool bIsEnd = false;
		while( !bIsEnd ) {
			bIsEnd = true;
			vector<bool> vIsSendToClient;
			vIsSendToClient.resize(m_vClient.size(),false);
			for( uint i=0; i<m_vClient.size(); i++ ) {
				string sData = getNewData();
				if( sData=="" ) { continue; }

				writeToClient(m_vClient[i],"calculate_gradient " + sData + "\n");
				vIsSendToClient[i] = true;
				bIsEnd = false;
			}

			for( uint i=0; i<m_vClient.size(); i++ ) {
				if( !vIsSendToClient[i] ) { continue; }
				readFromClient(m_vClient[i]);
			}
		}

		CERR() << " ==> done." << endl;
	}

	string getNewData()
	{
		if( m_batch_size>=MAX_BATCH_SIZE ) { return ""; }
		m_batch_size++;
		string sSgfString = m_dirExplorer.getNextSgfInformation().m_sSgfString;
		string sData = "";
		for( int i=0; i<sSgfString.length(); i++ ) {
			if( sSgfString[i]=='\n' ) { continue; }
			sData += sSgfString[i];
		}

		return sData;
	}

	void updateTheta()
	{
		CERR() << "Update theta ... ";

		for( uint i=0; i<m_vClient.size(); i++ ) { writeToClient(m_vClient[i],"get_gradient \n"); }
		for( uint i=0; i<m_vClient.size(); i++ ) { readFromClient(m_vClient[i]); }

		// parse gradient
		double dError = 0.0f;
		Vector<double,MAX_NUM_PLAYOUT_FEATURES> vGradient;
		vGradient.setAllAs(0.0f,MAX_NUM_PLAYOUT_FEATURES);
		for( uint i=0; i<m_vClient.size(); i++ ) {
			double dResult = 0.0f;
			stringstream ss(m_vClient[i]->getResult());
			ss >> dResult;
			dError += dResult;
			for( uint j=0; j<MAX_NUM_PLAYOUT_FEATURES; j++ ) {
				ss >> dResult;
				vGradient[j] += dResult;
			}
		}
		dError /= MAX_BATCH_SIZE;
		for( uint j=0; j<MAX_NUM_PLAYOUT_FEATURES; j++ ) {
			vGradient[j] /= MAX_BATCH_SIZE;
		}

		CERR() << "MSE = " << dError << endl;
		CERR() << "=============" << endl;

		// update theta
		map<int,bool> vShow;
		vShow[426] = vShow[2051] = vShow[2066] = vShow[2087] = true;

		uint index = -1, index0 = -1;
		double dMaxDiff = 0, dMaxDiff0 = 0;
		StatisticData avgDiff,avgDiff0;
		double dL = 0.0f, dM = 0.0f, dV = 0.0f;
		double dBeta1T = pow(m_dBeta1,m_iteration);
		double dBeta2T = pow(m_dBeta2,m_iteration);
		for( uint i=0; i<MAX_NUM_PLAYOUT_FEATURES; i++ ) {
			if( vShow[i] ) { CERR() << "feature index: " << i << endl; }

			double dOldTheta = m_vTheta[i];
			double dTemp;

			dL = vGradient[i] + m_dLambda*(m_vTheta0[i]-m_vTheta[i]);

			if( vShow[i] && !m_bUseSGD ) {
				CERR() << "vL[i] = vGradient[i] + m_dLambda*(m_vTheta0[i]-m_vTheta[i]) => ";
				CERR() << dL << " = " << vGradient[i] << " + " << m_dLambda << "*(" << m_vTheta0[i] << "-" << m_vTheta[i] << ")" << endl;
			}

			dTemp = m_vM[i];
			m_vM[i] = m_dBeta1*m_vM[i] + (1-m_dBeta1)*dL;
			if( vShow[i] && !m_bUseSGD ) {
				CERR() << "m_vM[i] = m_dBeta1*m_vM[i] + (1-m_dBeta1)*vL[i] => ";
				CERR() << m_vM[i] << " = " << m_dBeta1 << "*" << dTemp << " + (1-" << m_dBeta1 << ")*" << dL << endl;
			}

			dTemp = m_vV[i];
			m_vV[i] = m_dBeta2*m_vV[i] + (1-m_dBeta2)*dL*dL;
			if( vShow[i] && !m_bUseSGD ) {
				CERR() << "m_vV[i] = m_dBeta2*m_vV[i] + (1-m_dBeta2)*vL[i]*vL[i] => ";
				CERR() << m_vV[i] << " = " << m_dBeta2 << "*" << dTemp << " + (1-" << m_dBeta2 << ")*" << dL << "*" << dL << endl;
			}

			dM = m_vM[i] / (1-dBeta1T);
			if( vShow[i] && !m_bUseSGD ) {
				CERR() << "vM[i] = m_vM[i] / (1-dBeta1T) => ";
				CERR() << dM << "= " << m_vM[i] << " / (1-" << dBeta1T << ")" << endl;
			}

			dV = m_vV[i] / (1-dBeta2T);
			if( vShow[i] && !m_bUseSGD ) {
				CERR() << "vV[i] = m_vV[i] / (1-dBeta2T) => ";
				CERR() << dV << "= " << m_vV[i] << " / (1-" << dBeta2T << ")" << endl;
			}

			dTemp = m_vTheta[i];
			if( m_bUseSGD ) { m_vTheta[i] = m_vTheta[i] + m_dAlpha*vGradient[i]; }
			else { m_vTheta[i] = m_vTheta[i] + m_dAlpha*dM/(sqrt(dV)+m_dEpsilon); }
			if( vShow[i] && !m_bUseSGD ) {
				CERR() << "m_vTheta[i] = m_vTheta[i] + m_dAlpha*vM[i]/(sqrt(vV[i])+m_dEpsilon) => ";
				CERR() << m_vTheta[i] << " = " << dTemp << " + " << m_dAlpha << "*" << dM << "/(" << sqrt(dV) << "+" << m_dEpsilon << ")" << endl;
			}

			if( vShow[i] && m_bUseSGD ) {
				CERR() << "m_vTheta[i] = m_vTheta[i] + m_dAlpha*vGradient[i] => ";
				CERR() << m_vTheta[i] << " = " << dTemp << " + " << m_dAlpha << "*" << vGradient[i] << endl;
			}

			if( vShow[i] ) { CERR() << "diff = " << m_vTheta[i] << " - " << dOldTheta << " = " << m_vTheta[i]-dOldTheta << endl; }

			avgDiff.add(fabs(m_vTheta[i]-dOldTheta));
			avgDiff0.add(fabs(m_vTheta[i]-m_vTheta0[i]));
			if( fabs(m_vTheta[i]-dOldTheta)>dMaxDiff ) {
				index = i;
				dMaxDiff = fabs(m_vTheta[i]-dOldTheta);
			}
			if( fabs(m_vTheta[i]-m_vTheta0[i])>dMaxDiff0 ) {
				index0 = i;
				dMaxDiff0 = fabs(m_vTheta[i]-m_vTheta0[i]);
			}
		}

		CERR() << "=============" << endl;
		CERR() << " ==> done." << endl;

		cerr << "avg diff = " << avgDiff.getMean();
		cerr << ", max diff = " << dMaxDiff << ", index = " << index << endl;
		cerr << "avg diff to 0 = " << avgDiff0.getMean();
		cerr << ", max diff to 0 = " << dMaxDiff0 << ", index = " << index0 << endl;

		storePlayoutFeatureValue();
	}

	void storePlayoutFeatureValue()
	{
		if( m_iteration%100!=0 ) { return; }

		string sDirectory = WeichiConfigure::db_dir + "playout_sb_iter" + ToString(m_iteration) + ".db";
		ofstream fout(sDirectory.c_str(),ios::out|ios::binary);

		double dPlayoutFeatureValue[MAX_NUM_PLAYOUT_FEATURES];
		for( uint i=0; i<MAX_NUM_PLAYOUT_FEATURES; i++ ) { dPlayoutFeatureValue[i] = exp(m_vTheta[i]); }
		fout.write(reinterpret_cast<char*>(dPlayoutFeatureValue), sizeof(double)*MAX_NUM_PLAYOUT_FEATURES);
		fout.close();
	}

	void broadcastTheta()
	{
		CERR() << "broadcast theta ... ";

		string sTheta = "";
		for( uint i=0; i<m_vTheta.size(); i++ ) { sTheta += ToString(m_vTheta[i]) + " "; }
		for( uint i=0; i<m_vClient.size(); i++ ) { writeToClient(m_vClient[i],"update_theta " + sTheta + "\n"); }
		for( uint i=0; i<m_vClient.size(); i++ ) { readFromClient(m_vClient[i]); }

		CERR() << " ==> done." << endl;
	}
};

class SBClient: public TBaseClient {
private:
	SBMaster sbMaster;

public:
	SBClient( string sIP, string sPort, int numThread )
		: TBaseClient(sIP,sPort,numThread) {}

	void run()
	{
		connectToServer();
		
		sbMaster.reset();
		while( true ) {
			string sCommand = readFromServer();
			string sResult = "done\n";

			CERR() << " receive \"" << sCommand.substr(0,sCommand.find(" ")) << "\"";

			if( sCommand.find("get_gradient")!=string::npos ) { sResult = getGradient(); }
			else if( sCommand.find("calculate_gradient")!=string::npos ) { sbMaster.calculateGradient(sCommand.substr(sCommand.find(" "))); }
			else if( sCommand.find("update_theta")!=string::npos ) { updateTheta(sCommand.substr(sCommand.find("update_theta")+13)); }
			else if( sCommand.find("exit")!=string::npos ) { break; }

			CERR() << " ==> done." << endl;

			writeToServer(sResult);
		}
	}

private:
	string getGradient()
	{
		string sResult = ToString(sbMaster.getError());
		const Vector<double,MAX_NUM_PLAYOUT_FEATURES>& vGradients = sbMaster.getAccmulateGradients();
		for( uint i=0; i<vGradients.size(); i++ ) {
			sResult += " " + ToString(vGradients[i]);
		}
		sResult += '\n';

		sbMaster.reset();

		return sResult;
	}

	void updateTheta( string sCommand )
	{
		double dTheta = 0.0f;
		stringstream ss(sCommand);
		for( uint i=0; i<MAX_NUM_PLAYOUT_FEATURES; i++ ) {
			ss >> dTheta;
			WeichiGammaTable::setPlayoutFeatureValue(i,exp(dTheta));
		}
	}
};

#endif