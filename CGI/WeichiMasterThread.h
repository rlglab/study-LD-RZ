#ifndef __WEICHIMASTERTHREAD_H__
#define __WEICHIMASTERTHREAD_H__

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "MasterThread.h"
#include "WeichiAcceptor.h"
#include "WeichiWorker.h"

class WeichiMasterThread : public MasterThread<WeichiMove, WeichiThreadState, WeichiSlaveThread, WeichiUctNode, WeichiMoveDecisionMaker, WeichiRootShifter, WeichiPrunePolicy, WeichiStopCondition>
{
	typedef NodePtr<WeichiUctNode> UctNodePtr ;

private:
	// acceptor threads
	WeichiAcceptor m_acceptor;
	boost::thread_group m_acceptorThreads;

	uint m_numRemoteUpdateThread;

public:
	WeichiMasterThread ( uint numThread = 1 )
		: MasterThread (numThread)
		, m_acceptor (Configure::server_port)
		, m_numRemoteUpdateThread(4)
	{
		if ( arguments::mode == "cnnserver" ) {
			// test code, 2 thread run io_service
			m_acceptorThreads.create_thread( boost::bind(&WeichiAcceptor::run, &m_acceptor) );
			//m_acceptorThreads.create_thread( boost::bind(&WeichiAcceptor::run, &m_acceptor) );

			// create remote update threads (thread id > MAX_NUM_THREADS)
			for ( uint i=0 ; i<m_numRemoteUpdateThread ; ++i ) {
				WeichiSlaveThread* slaveThread = new WeichiSlaveThread ( MAX_NUM_THREADS+i+1, m_threadData ) ;
				m_vSlaveThreads.push_back(slaveThread) ;
				m_threads.create_thread( boost::bind ( &WeichiSlaveThread::run, slaveThread ) ) ;
			}
		}
	}

	~WeichiMasterThread()
	{
	}

	bool play( WeichiMove move )
	{
		// recollect CNN job queue
		WeichiGlobalInfo::getCNNJobQueue().reCollectJobWithPlay(&(*m_pRoot), move);
		WeichiGlobalInfo::getSearchInfo().m_moveCache.cleanHashTable();

		bool bPlaySuccess = MasterThread::play(move);
		if ( arguments::mode == "cnnserver" && bPlaySuccess ) {
			ostringstream oss;
			oss << "play " << toChar(move.getColor()) << ' ' << move.toGtpString();
			m_acceptor.broadcast(oss.str());
		}
		return bPlaySuccess;
	}

	void newGame()
	{
		if ( arguments::mode == "cnnserver" ) {
			m_acceptor.broadcast("clear_board");
		}
		MasterThread::newGame();

		WeichiGlobalInfo::getRemoteInfo().m_cnnJobTable.clear();
		WeichiGlobalInfo::getCNNJobQueue().reset();
		WeichiGlobalInfo::getSearchInfo().m_moveCache.clear();
	}

	WeichiMove genmove()
	{
		// recalculate gpu num
		WeichiConfigure::TotalGpuNum = 0;
		for ( uint i=1; i<m_vSlaveThreads.size(); ++i ) {
			if ( m_vSlaveThreads[i]->getState().isCNNThread() ) { ++WeichiConfigure::TotalGpuNum; }
		}
		m_acceptor.recalculateNumThread();
		WeichiConfigure::TotalGpuNum += m_acceptor.getTotalNumGpuThread();
		CERR() << "GPU num: " << WeichiConfigure::TotalGpuNum << std::endl;
		sendStart();
		WeichiMove m = MasterThread::genmove();
		sendStop();
		return m;
	}

	void ponder_start()
	{
		WeichiGlobalInfo::get()->cleanSummaryInfo();
		sendStart();
		MasterThread::ponder_start();
	}

	void ponder_stop()
	{
		MasterThread::ponder_stop();
		sendStop();
	}

	void pruneTree (bool after_play)
	{
		sendStop();
		MasterThread::pruneTree(after_play);
		sendStart();
	}

	void sendStart()
	{
		if ( arguments::mode == "cnnserver" ) {
			ostringstream oss;
			oss << "start " << WeichiGlobalInfo::getRemoteInfo().m_jobSessionId << ' ' << WeichiDynamicKomi::Internal_komi;
			m_acceptor.broadcast(oss.str());
		}
	}

	void sendStop()
	{
		if ( arguments::mode == "cnnserver" ) {
			m_acceptor.broadcast("stop");
			WeichiGlobalInfo::getRemoteInfo().m_jobSessionId++;
			WeichiGlobalInfo::getRemoteInfo().m_cnnJobTable.revert();
		}
	}

	void broadcast( const std::string& msg, bool bBroadcastGPU=true )
	{
		m_acceptor.broadcast(msg, bBroadcastGPU);
	}

	// must be called in worker mode
	void replyStart( uint sessionId )
	{
		if ( arguments::mode == "cnnworker" ) {
			// skip thread 1, i start from 1
			for ( uint i=1; i<m_vSlaveThreads.size(); ++i ) {
				m_vSlaveThreads[i]->setSessionId(sessionId);
				m_vSlaveThreads[i]->startCompute();
			}
		} else {
			CERR() << "replyStart error: must be called in cnnworker mode" << std::endl;
		}
	}

	// must be called in worker mode
	void setupJobs( uint threadId, std::string jobContent )
	{
		if ( 1 <= threadId && threadId <= m_vSlaveThreads.size() ) {
			m_vSlaveThreads[threadId-1]->setupJobs(jobContent);
		} else {
			CERR() << "Error: wrong thread ID" << std::endl;
		}
	}

	// must be called in worker mode
	void startCompute ( uint threadId )
	{
		if ( 1 <= threadId && threadId <= m_vSlaveThreads.size() ) {
			m_vSlaveThreads[threadId-1]->startCompute();
		} else {
			CERR() << "Error: wrong thread ID" << std::endl;
		}
	}
};


#endif  
