#ifndef TRAINING_H
#define TRAINING_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "BasicType.h"
#include "concepts.h"
#include "TrainMode.h"
#include "TrainShareData.h"
#include "TrainGenerator.h"
#include "WeichiThreadState.h"
#include "WeichiBoard.h"
#include "WeichiMoveFeatureHandler.h"

class Train {
private:
	fstream m_fTrainResult;
	TrainShareData m_shareData;
	boost::thread_group m_threads;
	Vector<TrainGenerator*,MAX_NUM_THREADS> m_vTrainGenerator;

public:
	Train() {}
	void start();

private:
	bool isNeedParallelMode();
	bool doParallelMode();
	bool initialThreadShareData();
	bool initializeDirectory();
	void summarizeThreadTrainResult();

	bool doNonParallelMode();
	void generateRadius3SymmetricIndexDB();
	void generateUctFeatureHeader();
	void generatePlayoutFeatureHeader();
};

#endif