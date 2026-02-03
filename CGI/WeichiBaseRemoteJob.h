#ifndef __WEICHIBASEREMOTEJOB_H__
#define __WEICHIBASEREMOTEJOB_H__

#include <string>
#include <boost/thread/mutex.hpp>
#include "strops.h"
#include "ThreadSharedData.h"
#include "CNNJobQueue.h"

class WeichiThreadState;
class WeichiUctAccessor;

class WeichiBaseRemoteJob
{
protected:
	// Job mutex
	mutable boost::mutex m_mutex;

	// Job content
	uint m_sessionId;
	vector<CNNJob> m_vJobs;

public:
	WeichiBaseRemoteJob() { clear(); }
	void clear() { boost::lock_guard<boost::mutex> lock(m_mutex); m_sessionId = -1; m_vJobs.clear(); }

	boost::mutex& getMutex() const { return m_mutex; }

	const uint getSessionId() const { return m_sessionId; }
	void setSessionId( uint sessionId ) { m_sessionId = sessionId; }

	const vector<CNNJob>& getJobContent() const { return m_vJobs; }
	void setJobContent( const vector<CNNJob>& vJobs ) { m_vJobs = vJobs; }

	std::string serializeCandidates() const
	{
		std::string serializedString;

		serializedString += ToString(m_vJobs.size());
		for ( uint i=0; i<m_vJobs.size(); ++i ) {
			serializedString += " " + ToString(m_vJobs[i].m_vPaths.size());
			for ( uint j=0; j<m_vJobs[i].m_vPaths.size(); ++j ) {
				WeichiMove move = m_vJobs[i].m_vPaths[j];
				serializedString += " " + move.toGtpString();
			}
		}
		return serializedString;
	}

	//virtual void produceJobContent() = 0;
	virtual void setResult(const string& sJobResult) = 0;
	virtual bool update(WeichiThreadState& state, WeichiUctAccessor& uctAccessor, ThreadSharedData<WeichiUctNode>& threadData, node_manage::NodeAllocator<WeichiUctNode>& nodeAllocator) = 0;
	virtual void revert() = 0;
};

#endif 
