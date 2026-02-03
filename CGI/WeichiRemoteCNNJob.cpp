#include "WeichiRemoteCNNJob.h"
#include "WeichiGlobalInfo.h"
#include "WeichiThreadState.h"
#include "WeichiUctAccessor.h"

void WeichiRemoteCNNJob::produceJobContent( WeichiCNNNetType type, uint batchSize )
{
	setCnnType( type );
	/*if( type==CNN_NET_SL && WeichiGlobalInfo::getCNNSLJobQueue().getQueueSize()<static_cast<int>(batchSize) && WeichiGlobalInfo::getCNNBVVNJobQueue().getQueueSize()>static_cast<int>(batchSize) ) { return; }

	while ( m_vJobs.size() < batchSize ) {
		vector<CNNJob> vNewJobs;
		switch ( type ) {
		case CNN_NET_SL: vNewJobs = WeichiGlobalInfo::getCNNSLJobQueue().getJob( batchSize - static_cast<uint>(m_vJobs.size()) ); break;
		case CNN_NET_BV_VN: vNewJobs = WeichiGlobalInfo::getCNNBVVNJobQueue().getJob( batchSize - static_cast<uint>(m_vJobs.size()) ); break;
		default: CERR() << "sendGpuJob: error type" << std::endl; break;
		}
		if ( vNewJobs.size() == 0 ) { break; }
		m_vJobs.insert( m_vJobs.end(), vNewJobs.begin(), vNewJobs.end() );
	}*/
	m_vJobs = WeichiGlobalInfo::getCNNJobQueue().getJob(batchSize);
}

bool WeichiRemoteCNNJob::update(WeichiThreadState& state, WeichiUctAccessor& uctAccessor, ThreadSharedData<WeichiUctNode>& threadData, node_manage::NodeAllocator<WeichiUctNode>& nodeAllocator)
{
	if ( WeichiGlobalInfo::getRemoteInfo().m_jobSessionId == getSessionId() ) {
		uctAccessor.updateAll(*this);
		state.rollback();
		if ( nodeAllocator.hasAllocFail() ) {
			threadData.poolFull = true;
			return false;
		}
	} else {
		CERR() << "abandon CNN job" << std::endl;
	}

	return true;
}

void WeichiRemoteCNNJob::revert()
{
	const vector<CNNJob>& vPaths = getJobContent();
	for ( auto it=vPaths.rbegin(); it!=vPaths.rend(); ++it ) {
		const CNNJob& job = *it;
		switch( getCnnType() ) {
		case CNN_NET:
			WeichiGlobalInfo::getCNNJobQueue().addJobToFront(job);
			/*for (int i = 0; i < job.m_vPaths.size(); ++i ) {
				CERR() << job.m_vPaths[i].toGtpString() << ' ';
			}
			CERR() << endl;*/
			break;
		default: CERR() << "revertJob: unknown cnnType" << std::endl; break;
		}
	}
	clear();
} 
