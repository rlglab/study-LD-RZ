#ifndef __WEICHISLAVETHREAD_H__
#define __WEICHISLAVETHREAD_H__

#include <boost/bind.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

#include "SlaveThread.h"
#include "RLDCNN.h"
#include "WeichiWorker.h"

/*!
	@brief  SlaveThread (Compute Engine of MCTS)
	@author kwchen
*/

class WeichiSlaveThread : public SlaveThread<WeichiMove, WeichiPlayoutResult, WeichiThreadState, WeichiUctNode, WeichiUctAccessor, WeichiPlayoutAgent, WeichiStopCondition>
{
public:
	typedef NodePtr<WeichiUctNode> UctNodePtr ;

private:
	// for worker
	vector<WeichiCNNNetType> m_vNetCollectionType;
	vector<CNNJob> m_vJobs;
	uint m_netCollectionTypeIndex;
	uint m_sessionId;
	uint m_replySessionId;
	boost::atomic<bool> m_bJobsInitialized;

public:
	WeichiSlaveThread ( uint threadId, ThreadSharedData<WeichiUctNode>& threadData )
		: SlaveThread(threadId, threadData)
		, m_bJobsInitialized(false)
		, m_netCollectionTypeIndex(0)
		, m_sessionId(-1)
		, m_replySessionId(-2)
	{
	}

	void run () 
	{
		initialize();
		if ( m_state.isWorkerThread() ) {
			runWorker();
			return;
		}
		SlaveThread::run();
	}

	void setSessionId(uint sessionId) { m_sessionId = sessionId; }

	// for handle_job, must be called in worker mode
	void setupJobs( std::string jobContent )
	{
		boost::lock_guard<boost::mutex> lock(m_exclusiveMutex);
		istringstream iss(jobContent);
		uint numPaths;
		iss >> numPaths;
		m_vJobs.resize(numPaths);
		for ( uint i=0; i<numPaths; ++i ) {
			uint numMoves;
			iss >> numMoves;
			Color c = m_state.getRootTurn();
			m_vJobs[i].m_vPaths.clear();
			for ( uint j=0; j<numMoves; ++j ) {
				std::string pos_str;
				iss >> pos_str;
				WeichiMove m (c, pos_str);
				m_vJobs[i].m_vPaths.push_back(m);
				c = AgainstColor(c);
			}
		}
		m_bJobsInitialized = true;
	}

private:
	void doPlayouts () 
	{
		if( m_state.isCNNThread() ) { doCNNJobThreads(); }
		else if (m_state.isRemoteUpdateThread() ) { doRemoteUpdateThreads(); }
		else { doMCTSJobThreads(); }
	}

	void doMCTSJobThreads()
	{
		m_state.backup();
		m_uctAccessor.m_timer.reset();
		
		while ( !checkStopSearch() ) {
			
			if( m_threadId == 1 && WeichiConfigure::mcts_use_solver && m_threadData.root->isProved() ) {
				m_threadData.timeOver = true ;
				m_threadData.aborted = true ;				
				break ;
			}

			if ( m_threadId == 1 && Configure::period_report ) {
				float timeUsed = m_threadData.stopwatch.getRunSeconds() ;
				if ( timeUsed >= m_next_report_time ) {
					UctNodePtr& root = const_cast<UctNodePtr&>(m_threadData.root);
					uint sim_count = m_threadData.simulateCount;
					m_state.summarize(root, timeUsed, sim_count, true);
					m_next_report_time = timeUsed + Configure::period ;
				}
			}

			if ( Configure::ShowSimulationCount ) {
				if (m_threadData.simulateCount % 1000 == 0) {
					CERR() << m_threadData.simulateCount << "           \r";
				}
			}
			m_state.startSimulation(m_threadData.simulateCount) ;

			// TODO: forecast pool-full event

			// selection & expansion
			m_uctAccessor.selectNode();
			if ( m_nodeAllocator.hasAllocFail() ) {
				m_threadData.poolFull = true ;
				m_state.rollback();
				continue ;
			}

			// simulation 
			m_state.startPlayout();
			WeichiPlayoutResult result = m_playoutAgent.run();
			m_state.endPlayout();

			// back-propagation
			m_uctAccessor.update(result) ;

			m_state.endSimulation() ;
			//m_threadData.simulateCount++;

			m_state.rollback() ;
		}

		if ( m_threadData.timeOver )
			m_threadData.aborted = true ;
		m_state.rollback();
	}

	void doCNNJobThreads()
	{
		m_state.backup();

		while ( !checkStopSearch() ) {
			m_uctAccessor.doUctCNNJob();
			m_threadData.simulateCount = WeichiGlobalInfo::getTreeInfo().m_nDCNNJob;

			if( m_nodeAllocator.hasAllocFail() ) {
				m_threadData.poolFull = true ;
				m_state.rollback();
				continue;
			}
		}

		if ( m_threadData.timeOver )
			m_threadData.aborted = true ;
		m_state.rollback();
	}

	// ==================== worker functions ====================

	// for reply_start and reply_job, must be called in worker mode
	std::string nextJobHeaderString()
	{
		ostringstream oss;
		oss << m_threadId << ' ' << m_sessionId << ' ';
		++m_netCollectionTypeIndex;
		if ( m_netCollectionTypeIndex >= m_vNetCollectionType.size() ) { m_netCollectionTypeIndex = 0; }

		m_vNetCollectionType = m_state.m_dcnnNetCollection.getNetCollectionType();
		WeichiCNNNetType type = m_vNetCollectionType[m_netCollectionTypeIndex];
		oss << "GPU " << getWeichiCNNNetTypeString(type);
		oss << ' ' << m_state.m_dcnnNetCollection.getNet(type)->getNumJobPerForward();
		return oss.str();
	}

	void doCNNJobWorkerThreads()
	{
		// reply_start
		if ( m_sessionId != m_replySessionId ) {
			WeichiWorker::write("reply_start " + nextJobHeaderString());
			m_replySessionId = m_sessionId;
			return;
		}

		// reply_job
		m_state.backup();

		while ( !m_bJobsInitialized );
		WeichiCNNNetType currentCnnType = m_vNetCollectionType[m_netCollectionTypeIndex];
		std::string sJobResult;
		if ( m_vJobs.size()>0 ) { sJobResult = m_uctAccessor.runRemoteJob(currentCnnType, m_vJobs); }
		else { boost::this_thread::sleep_for(boost::chrono::microseconds(1)); }
		ostringstream oss;
		oss << "reply_job " << nextJobHeaderString() << ' '
			<< getWeichiCNNNetTypeString(currentCnnType) << ' ' << sJobResult;
		
		m_bJobsInitialized = false;
		WeichiWorker::write(oss.str());
		m_state.rollback();
	}

	void runWorker()
	{
		while ( true ) {
			waitComputeStart();
			if ( isQuit() ) break ;
			boost::lock_guard<boost::mutex> lock(m_exclusiveMutex);

			m_uctAccessor.setRoot(m_threadData.root);

			if ( m_bStateChanged ) {
				/// if state changed, backup-state should be renew
				m_bStateChanged = false ;

				// start doing root filter (add by 5D)
				m_state.m_rootFilter.startFilter();
			}
			if ( m_state.isCNNThread() ) { doCNNJobWorkerThreads(); }
			// else MCTS thread do nothing
			// no finishCompute()
		}
	}

	// ==================== server functions ====================

	void doRemoteUpdateThreads()
	{
		m_state.backup();

		while ( !checkStopSearch() ) {
			bool bHasDoneJob = remoteCNNUpdate();
			if ( !bHasDoneJob ) {
				boost::this_thread::sleep_for(boost::chrono::microseconds(1));
			} else {
				m_threadData.simulateCount = WeichiGlobalInfo::getTreeInfo().m_nDCNNJob;
			}
		}

		if ( m_threadData.timeOver )
			m_threadData.aborted = true ;
		m_state.rollback();
	}

	bool remoteCNNUpdate()
	{
		return WeichiGlobalInfo::getRemoteInfo().m_cnnJobTable.updateOneJob(m_state, m_uctAccessor, m_threadData, m_nodeAllocator);
	}
};

#endif 
