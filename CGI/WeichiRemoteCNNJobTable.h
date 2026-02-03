#ifndef __WEICHIREMOTECNNJOBTABLE_H__
#define __WEICHIREMOTECNNJOBTABLE_H__

#include "WeichiRemoteCNNJob.h"
#include "WeichiBaseRemoteJobTable.h"

class WeichiRemoteCNNJobTable : public WeichiBaseRemoteJobTable<WeichiRemoteCNNJob>
{
public:
	uint produceNewJob( uint sessionId, WeichiCNNNetType type, uint batchSize, string& serializedString )
	{
		boost::unique_lock<boost::recursive_mutex> lock( m_slotMutex );
		m_slotCV.wait( lock, [&] { return !m_emptySlots.empty(); } );
		uint newJobID = m_emptySlots.top();
		m_emptySlots.pop();
		m_outsourcingSlots.insert( newJobID );

		m_table[newJobID].clear();
		m_table[newJobID].produceJobContent(type, batchSize);
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
