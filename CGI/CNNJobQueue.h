#ifndef CNNJOBQUEUE_H
#define CNNJOBQUEUE_H

#include <vector>
#include <boost/thread.hpp>

#include "BaseJobQueue.h"
#include "NodePtr.h"
#include "Vector.h"
#include "WeichiMove.h"
#include "WeichiUctNode.h"

class CNNJob {
	typedef NodePtr<WeichiUctNode> UctNodePtr;

public:
	Vector<WeichiMove,MAX_TREE_DEPTH> m_vPaths;
	Vector<UctNodePtr, MAX_TREE_DEPTH> m_vPathPtr; // for removeVirtualLoss

	CNNJob() {}
	CNNJob( Vector<WeichiMove,MAX_TREE_DEPTH> vPaths, const Vector<UctNodePtr, MAX_TREE_DEPTH>& vPathPtr = Vector<UctNodePtr, MAX_TREE_DEPTH>() )
		: m_vPaths(vPaths), m_vPathPtr(vPathPtr)
	{}
};

class CNNJobQueue: public BaseJobQueue<CNNJob> {
	typedef NodePtr<WeichiUctNode> UctNodePtr;
private:
	boost::atomic<int> m_nUnfinishedJobs;

public:
	CNNJobQueue( int jobSize=(1<<17) )
		: BaseJobQueue(jobSize)
	{
		reset();
	}

	inline void reset()
	{
		BaseJobQueue::reset();
		resetUnfinishedJob();
	}

	inline bool addJob(const CNNJob& job)
	{
		addUnfinishedJob();
		return BaseJobQueue::addJob(job);
	}

	inline void reCollectJobWithPruneTree(WeichiUctNode* root)
	{
		boost::lock_guard<boost::mutex> lock(m_jobsMutex);

		CERR() << "Clean Job after prune tree, previous size = " << m_jobs.size();

		std::deque<CNNJob> newJobs;
		for( auto it=m_jobs.begin(); it!=m_jobs.end(); it++ ) {
			const CNNJob& job = *it;
			const Vector<WeichiMove,MAX_TREE_DEPTH>& vPaths = job.m_vPaths;
			vector<WeichiUctNode*> vPathPtr = traverseJobInTree(root, job);

			if (vPaths.size() == vPathPtr.size()) {
				CNNJob newJob;
				for (uint i = 0; i < vPaths.size(); i++) { newJob.m_vPaths.push_back(vPaths[i]); }
				newJobs.push_back(newJob);
			} else {
				// We should also clear virtual loss when clear the job
				for (uint i = 0; i < vPathPtr.size(); i++) { vPathPtr[i]->removeVirtualLoss(WeichiConfigure::virtual_loss_count); }
			}
		}

		m_jobs.swap(newJobs);
		m_nUnfinishedJobs = m_jobs.size();
		CERR() << ", new size = " << m_jobs.size() << endl;
	}

	inline void reCollectJobWithPlay(WeichiUctNode* root, WeichiMove move)
	{
		boost::lock_guard<boost::mutex> lock(m_jobsMutex);

		CERR() << "Clean Job with " << move.toGtpString() << ", previous size = " << m_jobs.size();

		std::deque<CNNJob> newJobs;
		for( auto it=m_jobs.begin(); it!=m_jobs.end(); it++ ) {
			const CNNJob& job = *it;
			const Vector<WeichiMove,MAX_TREE_DEPTH>& vPaths = job.m_vPaths;

			if (vPaths.size() < 1 || vPaths[0] != move) {
				// We should also clear virtual loss when clear the job
				vector<WeichiUctNode*> vPathPtr = traverseJobInTree(root, job);
				for (uint i = 0; i < vPathPtr.size(); i++) { vPathPtr[i]->removeVirtualLoss(WeichiConfigure::virtual_loss_count); }
			} else {
				CNNJob newJob;
				for (uint i = 1; i < vPaths.size(); i++) { newJob.m_vPaths.push_back(vPaths[i]); }
				newJobs.push_back(newJob);
			}
		}

		m_jobs.swap(newJobs);
		m_nUnfinishedJobs = m_jobs.size();
		CERR() << ", new size = " << m_jobs.size() << endl;
	}

	inline int getUnfinishedJob() const { return m_nUnfinishedJobs; }

	inline void removeUnfinishedJob(int job_num = 1)
	{
		boost::unique_lock<boost::mutex> lock(m_jobsMutex);
		m_nUnfinishedJobs -= job_num;
	}

private:
	inline void addUnfinishedJob(int job_num = 1)
	{
		boost::unique_lock<boost::mutex> lock(m_jobsMutex);
		m_nUnfinishedJobs += job_num;
	}

	inline void resetUnfinishedJob()
	{
		boost::unique_lock<boost::mutex> lock(m_jobsMutex);
		m_nUnfinishedJobs = 0;
	}

	inline vector<WeichiUctNode*> traverseJobInTree(WeichiUctNode* root, const CNNJob& job)
	{
		WeichiUctNode* pNode = root;
		vector<WeichiUctNode*> vPathPtr;
		const Vector<WeichiMove, MAX_TREE_DEPTH>& vPaths = job.m_vPaths;

		for (uint i = 0; i < vPaths.size(); i++) {
			bool bIsInTree = false;
			for (uint j = 0; j < pNode->getNumChildren(); j++) {
				if (pNode->getChild(j)->getMove() != vPaths[i]) { continue; }
				pNode = &(*(pNode->getChild(j)));
				bIsInTree = true;
				vPathPtr.push_back(pNode);
				break;
			}
			if (!bIsInTree) { break; }
		}

		return vPathPtr;
	}
};

#endif