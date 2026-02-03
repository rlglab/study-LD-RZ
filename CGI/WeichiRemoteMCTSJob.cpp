#include "WeichiRemoteMCTSJob.h"
#include "WeichiGlobalInfo.h"
#include "WeichiThreadState.h"
#include "WeichiUctAccessor.h"

void WeichiRemoteMCTSJob::produceJobContent()
{
	/*const uint cpuBatchSize = 16;
	while ( m_vJobs.size() < cpuBatchSize ) {
		vector<CNNJob> vNewJobs;
		vNewJobs = WeichiGlobalInfo::getMCTSJobQueue().getJob( cpuBatchSize - static_cast<uint>(m_vJobs.size()) );
		if ( vNewJobs.size() == 0 ) { break; }
		m_vJobs.insert( m_vJobs.end(), vNewJobs.begin(), vNewJobs.end() );
	}*/
}

bool WeichiRemoteMCTSJob::update(WeichiThreadState& state, WeichiUctAccessor& uctAccessor, ThreadSharedData<WeichiUctNode>& threadData, node_manage::NodeAllocator<WeichiUctNode>& nodeAllocator)
{
	/*if ( WeichiGlobalInfo::getRemoteInfo().m_jobSessionId == getSessionId() ) {
		for ( uint i = 0; i < m_vJobs.size(); ++i ) {
			state.m_path = m_vJobs[i].m_vPathPtr;
			for ( uint j = 0; j < m_vJobs[i].m_vPaths.size(); ++j ) {
				state.play(m_vJobs[i].m_vPaths[j]);
			}
			uctAccessor.update(WeichiPlayoutResult(m_vResults[i]));
			threadData.simulateCount++;
			state.rollback();
		}
	} else {
		CERR() << "abandon MCTS job" << std::endl;
	}*/

	return true;
}

void WeichiRemoteMCTSJob::revert()
{
	/*const vector<CNNJob>& vPaths = getJobContent();
	for ( auto it=vPaths.rbegin(); it!=vPaths.rend(); ++it ) {
		const CNNJob& job = *it;
		WeichiGlobalInfo::getMCTSJobQueue().addJobToFront(job); break;
	}
	clear();*/
} 
