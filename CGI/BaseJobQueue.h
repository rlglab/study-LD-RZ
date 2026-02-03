#ifndef BASEJOBQUEUE_H
#define BASEJOBQUEUE_H

#include <deque>

template<class _Job> class BaseJobQueue {
protected:
	std::deque<_Job> m_jobs;
	mutable boost::mutex m_jobsMutex;

	const int m_maxSize;

public:
	BaseJobQueue( int jobSize=(1<<17) )
		: m_maxSize(jobSize)
	{
		reset();
	}

	inline int getQueueSize() const
	{
		boost::lock_guard<boost::mutex> lock(m_jobsMutex);
		int size = static_cast<int>(m_jobs.size());

		return size;
	}

	inline void reset()
	{
		boost::lock_guard<boost::mutex> lock(m_jobsMutex);
		m_jobs.clear();
	}

	inline bool addJob(const _Job& job)
	{
		boost::unique_lock<boost::mutex> lock(m_jobsMutex);
		m_jobs.push_back(job);

		return true;
	}

	inline bool addJobToFront( const _Job& job )
	{
		// high priority job
		boost::lock_guard<boost::mutex> lock(m_jobsMutex);
		m_jobs.push_front(job);

		return true;
	}

	inline std::vector<_Job> getJob( int nJobs )
	{
		boost::lock_guard<boost::mutex> lock(m_jobsMutex);
		std::vector<_Job> vJobs;
		for( int i=0; i<nJobs; i++ ) {
			if (m_jobs.size() == 0) { break; }
			vJobs.push_back(m_jobs.front());
			m_jobs.pop_front();
		}

		return vJobs;
	}
};

#endif