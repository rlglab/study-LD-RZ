#ifndef TCAFFE_H
#define TCAFFE_H

#include "BasicType.h"
#include "SgfLoader.h"
#include "TBaseMasterSlave.h"
#include "WeichiThreadState.h"
#include "WeichiCNNTrainNet.h"
#include "FileDirectoryExplorer.h"

class TCaffeSharedData {
public:
	fstream m_logFile;
	bool m_bIsTraining;
	bool m_bUpdateFirstBatch;
	boost::mutex m_mutex;
	WeichiCNNTrainNet* m_cnnTrainNet;
	FileDirectoryExplorer m_trainDirExplorer;
	FileDirectoryExplorer m_testDirExplorer;
};

class TCaffeSlave: public TBaseSlave<TCaffeSharedData> {
private:
	int m_batch_pos;
	float* m_label;
	WeichiThreadState m_state;

	static const int MAX_MOVES = 400;

public:
	TCaffeSlave( int id, TCaffeSharedData& sharedData )
		: TBaseSlave(id,sharedData), m_label(nullptr)
	{}

	~TCaffeSlave()
	{
		if( m_label!=nullptr ) { delete[] m_label; }
	}

private:
	virtual void initialize();
	virtual bool isOver();
	virtual void reset();
	virtual void doSlaveJob();
	
	SgfInformation getNextSgf();
	void calculateFeatures();
	void calculateVNLabel(float *label, const SgfLoader& sgfLoader, Color turnColor);
	void calculateBVLabel(float *label, const SgfLoader& sgfLoader, Color turnColor, const Territory& territory);
	int pickRandomMove( const SgfLoader &sgfLoader );
	int pickOpeningMove(const SgfLoader &sgfLoader);
	SgfInformation pickRandomSgf();
	bool resetFileDirectory();
};

class TCaffeMaster: public TBaseMaster<TCaffeSharedData,TCaffeSlave> {	
public:
	TCaffeMaster()
		: TBaseMaster(Configure::NumThread)
	{}

	void train();

private:
	virtual bool initialize();
	virtual TCaffeSlave* newSlave( int id );
	virtual void summarizeSlavesData();
	void updateCNNNet();
};

#endif