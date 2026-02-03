#ifndef __WEICHIBASEREMOTEJOBTABLE_H__
#define __WEICHIBASEREMOTEJOBTABLE_H__

#include <stack>
#include "WeichiBaseRemoteJob.h"

template<class _RemoteJob>
class WeichiBaseRemoteJobTable
{
protected:
	static const uint MAX_JOBTABLE_SIZE = 8192;
	_RemoteJob m_table[MAX_JOBTABLE_SIZE];

	std::stack<uint> m_emptySlots;
	std::set<uint> m_outsourcingSlots;
	boost::recursive_mutex m_slotMutex;
	boost::condition_variable_any m_slotCV;

	std::deque<uint> m_updateQueue;
	boost::recursive_mutex m_updateQueueMutex;

	boost::shared_mutex m_sharedMutex;

public:
	WeichiBaseRemoteJobTable() { clear(); }
	~WeichiBaseRemoteJobTable() {}

	// call by master thread
	void clear()
	{
		boost::unique_lock<boost::shared_mutex> tableLock(m_sharedMutex);
		boost::lock(m_slotMutex, m_updateQueueMutex);
		boost::lock_guard<boost::recursive_mutex> slotLock(m_slotMutex, boost::adopt_lock);
		boost::lock_guard<boost::recursive_mutex> updateLock(m_updateQueueMutex, boost::adopt_lock);

		m_outsourcingSlots.clear();
		m_updateQueue = std::deque<uint>();

		// setup empty slots
		m_emptySlots = std::stack<uint>();
		for ( uint i=0; i<MAX_JOBTABLE_SIZE; ++i ) {
			m_emptySlots.push(i);
		}
	}
	
	// call by master thread
	void revert()
	{
		boost::unique_lock<boost::shared_mutex> tableLock(m_sharedMutex);
		boost::lock(m_slotMutex, m_updateQueueMutex);
		boost::lock_guard<boost::recursive_mutex> slotLock(m_slotMutex, boost::adopt_lock);
		boost::lock_guard<boost::recursive_mutex> updateLock(m_updateQueueMutex, boost::adopt_lock);

		// revert undone job to job queue
		for ( auto it=m_outsourcingSlots.rbegin(); it!=m_outsourcingSlots.rend(); ++it ) {
			uint jobID = *it;
			m_updateQueue.push_back(jobID);
			//CERR() << "revert m_outsourcingSlots jobID: " << jobID << ", " << getWeichiCNNNetTypeString(m_table[jobID].getCnnType()) << std::endl;
		}
		m_outsourcingSlots.clear();

		// revert updateQueue
		while ( !m_updateQueue.empty() ) {
			uint jobID = m_updateQueue.back();
			m_updateQueue.pop_back();
			revertJobWithoutCheckLock(jobID);
			//CERR() << "revert m_updateQueue jobID: " << jobID << ", " << getWeichiCNNNetTypeString(m_table[jobID].getCnnType()) << std::endl;
		}

		// setup empty slots
		m_emptySlots = std::stack<uint>();
		for ( uint i=0; i<MAX_JOBTABLE_SIZE; ++i ) {
			m_emptySlots.push(i);
		}
	}

	void revertJob(uint jobID)
	{
		boost::shared_lock<boost::shared_mutex> tableLock(m_sharedMutex, boost::try_to_lock);
		if ( !tableLock ) { return; }

		boost::lock_guard<boost::recursive_mutex> lock(m_slotMutex);
		revertJobWithoutCheckLock(jobID);
		//CERR() << "revertJob: " << jobID << endl;
	}

	// call by acceptor thread
	void pushUpdateJob(uint jobID, const string& sJobResult)
	{
		boost::shared_lock<boost::shared_mutex> tableLock(m_sharedMutex, boost::try_to_lock);
		if ( !tableLock ) { return; }

		boost::lock(m_slotMutex, m_updateQueueMutex);
		boost::lock_guard<boost::recursive_mutex> slotLock(m_slotMutex, boost::adopt_lock);
		boost::lock_guard<boost::recursive_mutex> updateLock(m_updateQueueMutex, boost::adopt_lock);

		m_table[jobID].setResult(sJobResult);

		bool bJobIDExist = (m_outsourcingSlots.erase(jobID) == 1);
		if ( bJobIDExist ) { m_updateQueue.push_back(jobID); }
	}

	// call by remote update thread
	// return: true if job update success or outdated, false otherwise
	bool updateOneJob(WeichiThreadState& state, WeichiUctAccessor& uctAccessor, ThreadSharedData<WeichiUctNode>& threadData, node_manage::NodeAllocator<WeichiUctNode>& nodeAllocator)
	{
		boost::shared_lock<boost::shared_mutex> tableLock(m_sharedMutex, boost::try_to_lock);
		if ( !tableLock ) { return false; }

		boost::unique_lock<boost::recursive_mutex> lock(m_updateQueueMutex);
		if ( m_updateQueue.size() == 0 ) { return false; }

		uint jobID = m_updateQueue.front();
		m_updateQueue.pop_front();
		lock.unlock();

		bool bSuccess = m_table[jobID].update(state, uctAccessor, threadData, nodeAllocator);
		if ( bSuccess ) { releaseJob(jobID); }
		else {
			boost::lock_guard<boost::recursive_mutex> lock(m_updateQueueMutex);
			m_updateQueue.push_front(jobID);
		}

		return bSuccess;
	}

protected:
	void revertJobWithoutCheckLock(uint jobID)
	{
		m_table[jobID].revert();
		m_outsourcingSlots.erase(jobID);
		m_emptySlots.push(jobID);
	}

	void releaseJob(uint jobID)
	{
		boost::lock_guard<boost::recursive_mutex> lock(m_slotMutex);
		m_emptySlots.push(jobID);
		m_slotCV.notify_one();
	}
};

#endif 
