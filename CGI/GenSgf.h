#ifndef GENSGF_H
#define GENSGF_H

#include "BasicType.h"
#include "SpinLock.h"
#include "SgfLoader.h"
#include "WeichiThreadState.h"
#include "TBaseMasterSlave.h"
#include "TBaseAsyncServerClient.h"
#include "BaseCNNPlayoutGenerator.h"
#include "FileDirectoryExplorer.h"

class GenSgfSCShareData {
public:
	SpinLock m_lock;
	fstream m_foutSgf;
	fstream m_abortSgf;
	FileDirectoryExplorer m_explorer;

	// log info
	uint m_numWorker;
	uint m_total_progress;
	string m_startTime;
	map<pair<string,string>,string> m_machineMap;
	map<pair<string,string>,int> m_machineProgress;

	static const int MAX_GAMES_IN_ONE_FILE = 1000000;
};

class GenSgfSession: public TBaseSession {
private:
	pair<string,string> m_sName;
	HashKey64 m_currentBoardKey;
	GenSgfSCShareData& m_sharedData;

	static const int MAX_MOVES = 150;

public:
	GenSgfSession( boost::asio::io_service& io_service, GenSgfSCShareData& shareData )
		: TBaseSession(io_service), m_sharedData(shareData)
	{
	}

	void displayProgress();

private:
	virtual void handle_read( std::string sResult );
	virtual void handle_write() {}
	virtual void handle_disconnect();

	void saveSgf( string sResult, fstream& fout, string sPostfix="" );
};

class GenSgfServer: public TBaseAsyncServer<GenSgfSession> {
private:
	GenSgfSCShareData m_shareData;

public:
	GenSgfServer( short port )
		: TBaseAsyncServer(port)
	{
	}

private:
	bool checkConfigure();

	virtual void initialize();
	virtual GenSgfSession* getNewSession();
	virtual void handle_accept( GenSgfSession* new_session );
};

class GenSgfClient: public TBaseAsyncClient {
public:
	GenSgfClient( string sIP, string sPort )
		: TBaseAsyncClient(sIP,sPort)
	{
	}

	void run();
};

class GenSgfMSSharedData {
public:
	SpinLock m_lock;
	string m_sCommand;
	GenSgfClient* m_client;
};

class GenSgfSlave: public TBaseSlave<GenSgfMSSharedData> {
private:
	class SlaveGame {
	public:
		bool m_bIsValid;
		bool m_bIsFinish;
		string m_sSgfLinkInfo;
		SgfLoader m_sgfLoader;
		uint m_random_move_number;
		WeichiThreadState m_state;

		static const int MAX_MOVES = 350;

		SlaveGame() { reset(); }
		inline void reset() {
			m_bIsValid = true;
			m_bIsFinish = false;
			m_state.resetThreadState();
			m_random_move_number = Random::nextInt(MAX_MOVES);
		}
		inline void loadGame( string sSgfString, bool bLoadAll ) {
			reset();
			m_sgfLoader.parseFromString(sSgfString);
			for( uint i=0; i<m_sgfLoader.getPreset().size(); i++ ) { m_state.play(m_sgfLoader.getPreset()[i]); }
			
			int playmove_size = static_cast<int>(m_sgfLoader.getPlayMove().size());
			int max_move = bLoadAll? playmove_size: Random::nextInt(playmove_size);
			for( int i=0; i<max_move; i++ ) { m_state.play(m_sgfLoader.getPlayMove()[i]); }

			m_random_move_number = static_cast<uint>(m_sgfLoader.getPreset().size()) + max_move;
		}
	};

	WeichiCNNSLNet* m_net;
	vector<SlaveGame*> m_vGames;
	boost::atomic<int> m_nFinish;
	DCNNNetCollection m_dcnnSLNetCollection;
	DCNNNetCollection m_dcnnRLNetCollection;

public:
	GenSgfSlave( int id, GenSgfMSSharedData& sharedData )
		: TBaseSlave(id,sharedData)
	{}

private:
	virtual void initialize();
	virtual bool isOver();
	virtual void reset();
	virtual void doSlaveJob();

	void randomChangeSLNet();
	void handle_endGame( int gameID );
	void handle_loadNewGame( int gameID );
	void handle_addFeature( int gameID );
	void handle_forward();
};

class GenSgfMaster: public TBaseMaster<GenSgfMSSharedData,GenSgfSlave> {
private:
	string m_sSgfString;
	string m_sStartTime;
	string m_sEndTime;

public:
	GenSgfMaster( string sIP, string sPort, int numThread )
		: TBaseMaster(numThread)
	{
		m_sharedData.m_client = new GenSgfClient(sIP,sPort);
	}

	void run();
	inline string getSgfResult() { return m_sSgfString; }
	virtual bool initialize();

	inline GenSgfMSSharedData& getSharedData() { return m_sharedData; }

private:
	virtual GenSgfSlave* newSlave( int id ) { return new GenSgfSlave(id,m_sharedData); }
	virtual void summarizeSlavesData() {}
};

#endif