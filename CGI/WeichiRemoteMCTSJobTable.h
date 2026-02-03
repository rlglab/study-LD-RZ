#ifndef __WEICHIREMOTEMCTSJOBTABLE_H__
#define __WEICHIREMOTEMCTSJOBTABLE_H__

#include "WeichiRemoteMCTSJob.h"
#include "WeichiBaseRemoteJobTable.h"

class WeichiRemoteMCTSJobTable : public WeichiBaseRemoteJobTable<WeichiRemoteMCTSJob>
{
public:
	uint produceNewJob( uint sessionId, string& serializedString )
	{
		boost::unique_lock<boost::recursive_mutex> lock( m_slotMutex );
		m_slotCV.wait( lock, [&] { return !m_emptySlots.empty(); } );
		uint newJobID = m_emptySlots.top();
		m_emptySlots.pop();
		m_outsourcingSlots.insert( newJobID );

		m_table[newJobID].clear();
		m_table[newJobID].produceJobContent();
		m_table[newJobID].setSessionId( sessionId );
		serializedString = m_table[newJobID].serializeCandidates();

		// empty job
		if ( m_table[newJobID].getJobContent().size() == 0 ) {
			m_emptySlots.push(newJobID);
			m_outsourcingSlots.erase(newJobID);
			newJobID = -1;
		}
		return newJobID;
	}
};

#endif
