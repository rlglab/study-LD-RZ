#ifndef TRAINSHAREDATA_H
#define TRAINSHAREDATA_H

#include "SpinLock.h"
#include "TrainMode.h"
#include "FileDirectoryExplorer.h"

class TrainShareData {
public:
	uint m_numSample;
	uint m_numSimulation;
	TrainMode m_mode;
	SpinLock m_writeLock;
	fstream m_fTrainResult;		
	FileDirectoryExplorer m_fileExplorer;

	// for unique opening
	map<unsigned long long, bool> m_uniqueMap;
};

#endif