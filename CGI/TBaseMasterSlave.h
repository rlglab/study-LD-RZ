#ifndef TBASEMASTERSLAVE_H
#define TBASEMASTERSLAVE_H

#include <vector>
#include <boost/thread/barrier.hpp>
#include <boost/thread/thread.hpp>
#include "Configure.h"

template<class _SharedData> class TBaseSlave {
protected:
	int m_id;
	uint m_seed;
	_SharedData& m_sharedData;

	boost::barrier m_barStart;
	boost::barrier m_barFinish;

public:
	TBaseSlave( int id, _SharedData& sharedData )
		: m_id(id), m_sharedData(sharedData), m_barStart(2), m_barFinish(2)
	{}

	void run()
	{
		initialize();

		while( !isOver() ) {
			m_barStart.wait();
			reset();
			doSlaveJob();
			m_barFinish.wait();
		}
	}
	void startRun() { m_barStart.wait(); }
	void finishRun() { m_barFinish.wait(); }

	inline int getID() const { return m_id; }
	inline void setTimeSeed( uint seed ) { m_seed = seed; }

protected:
	virtual void initialize()
	{
		Random::reset(m_seed);
	}

	virtual bool isOver()=0;
	virtual void reset()=0;
	virtual void doSlaveJob()=0;
};

template<class _SharedData, class _Slave> class TBaseMaster {
protected:
	uint m_seed;
	int m_nThread;
	_SharedData m_sharedData;
	std::vector<_Slave*> m_vSlaves;
	boost::thread_group m_threads;

public:
	TBaseMaster( int nThread )
		: m_nThread(nThread)
	{}

	virtual void run()
	{
		if( !initialize() ) { return; }
		for( int i=0; i<m_nThread; i++ ) { m_vSlaves[i]->startRun(); }
		for( int i=0; i<m_nThread; i++ ) { m_vSlaves[i]->finishRun(); }
		summarizeSlavesData();
	}

	virtual bool initialize()
	{
		m_seed = Configure::UseTimeSeed? static_cast<uint>(time(NULL)): Configure::RandomSeed;
		Random::reset(m_seed);

		for( int id=0; id<m_nThread; id++ ) {
			_Slave* slave = newSlave(id);
			slave->setTimeSeed(m_seed+id+1);
			m_threads.create_thread(boost::bind(&_Slave::run,slave));
			m_vSlaves.push_back(slave);
		}

		return true;
	}

	virtual _Slave* newSlave( int id )
	{
		return new _Slave(id,m_sharedData);
	}

	virtual void summarizeSlavesData()=0;

	inline uint getSeed() const { return m_seed; }
};

#endif