#ifndef __WEICHIREMOTEMCTSJOB_H__
#define __WEICHIREMOTEMCTSJOB_H__

#include "WeichiBaseRemoteJob.h"

class WeichiRemoteMCTSJob : public WeichiBaseRemoteJob
{
private:
	// MCTS result
	vector<float> m_vResults;

public:
	WeichiRemoteMCTSJob() { clear(); }
	void clear() { WeichiBaseRemoteJob::clear(); }

	const vector<float>& getMCTSResult() const { return m_vResults; }
	void setMCTSResult( const vector<float>& vResults ) { m_vResults = vResults; }
	void setResult( const string& sJobResult )
	{
		// sJobResult to MCTSResult
		m_vResults.resize( m_vJobs.size() );
		istringstream iss( sJobResult );
		for ( uint i = 0; i < m_vJobs.size(); ++i ) {
			float playoutResult;
			iss >> playoutResult;
			m_vResults[i] = playoutResult;
		}
	}

	void produceJobContent();
	bool update(WeichiThreadState& state, WeichiUctAccessor& uctAccessor, ThreadSharedData<WeichiUctNode>& threadData, node_manage::NodeAllocator<WeichiUctNode>& nodeAllocator);
	void revert();
};

#endif 
