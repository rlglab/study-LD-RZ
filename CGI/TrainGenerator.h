#ifndef GENERATOR_H
#define GENERATOR_H

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/thread/barrier.hpp>

#include <vector>
#include "WeichiMove.h"
#include "WeichiThreadState.h"
#include "SpinLock.h"
#include "TrainShareData.h"
#include "MovePredictorStatistic.h"
#include "SgfLoader.h"
#include "FileDirectoryExplorer.h"
#include "BaseCNNPlayoutGenerator.h"

using namespace std;

class TrainGenerator {
private:
	struct RadiusPatternEntry {
		uint win;
		uint total;
		Vector<uint,MAX_RADIUS_SIZE> vIndex;
	};

	boost::barrier m_barStart;
	boost::barrier m_barFinish;

	uint m_id;
	uint m_gpu_device;
	uint m_numMoves;
	uint m_iSgfCounter;
	SgfInformation m_sgfInformation;
	SgfLoader m_sgfLoader;
	WeichiThreadState* m_state;
	TrainShareData& m_shareData;
	Territory m_territory;
	BaseCNNPlayoutGenerator *m_cnnGenerator;

	// radius pattern
	vector< map< HashKey64, RadiusPatternEntry > > m_vRadiusPatternCollection;
	
	// Move predictor statistic
	MovePredictorStatistic m_movePredictorStatistic;

	// board evaluation error
	vector<int> m_vNumEvaluation;
	vector<double> m_vSquareError;

	// CNN candidate generator
	vector< vector<CandidateEntry> > m_vRotateCandidates;
	vector<uint> m_vRotateAnswer;

	// for lifedeath judgement
	map<string, string> mLifeDeathAnswers;
	int m_nTotalProblems;
	int m_nCorrect;
	string m_testSubject;
	int m_testPos;

	class LogData{
	public:
		string m_sFilename;
		string m_sResultInfo;
		int m_iFileLine;
		int m_moveNumber;
		Color m_turnColor;
		Color m_winnerColor;
		uint m_movePosition;
		double m_komi;
		Territory gameResultTerritory;
		vector< Vector<Color,MAX_NUM_GRIDS> > m_vPlayoutTerritory;
	};
	vector<LogData> m_vLogData;
	// repetition sgf
	map< string,vector<HashKey64> > m_mMoveThresholdHashkeys;

	static const int RADIUS_PATTERN_COLLECTION_INTERVAL = 300;
	static const int REPETITION_POSITION_THRESHOLD = 100;

public:
	TrainGenerator( TrainShareData& trainShareData, uint id, uint gpu_device )
		: m_shareData(trainShareData), m_barStart(2), m_barFinish(2), m_id(id), m_gpu_device(gpu_device)
	{
		m_numMoves = 0;
		m_iSgfCounter = 0;
		m_state = new WeichiThreadState;
		m_nTotalProblems = 0;
		m_nCorrect = 0;
	}

	void run();
	void startRun() { m_barStart.wait(); }
	void finishRun() { m_barFinish.wait(); }
	void summarizeNumberMoves( const TrainGenerator& rhs );
	void writeRadiusPatternCollection();
	void summarizeMovePrediction( const TrainGenerator& rhs );
	void outputPredictionRateFile( string sFileName );
	void summarizeRepeatedSgfData( const TrainGenerator& rhs );
	void outputEvaluationError( string sFileName );
	void summarizeEvaluationError( const TrainGenerator& rhs );
	void outputRepeatedSgfFile();
	void outputDCNNRotationResult( string sFileName );

private:
	inline void writeLock() { m_shareData.m_writeLock.lock(); }
	inline void writeUnlock() { m_shareData.m_writeLock.unlock(); }

	void initialize();
	void playOneGame( string sFileName );
	bool generateOneMove( uint moveNumber, SgfLoader& sgfLoader );

	// for radius pattern
	void collectRadiusPattern( const WeichiMove& winMove );

	// for UCT & playout & CNN features
	void collectUCTFeatures( const WeichiMove& winMove );
	void collectPlayoutFeatures( const WeichiMove& winMove );
	vector<uint> translateUCTFeatureToFeatureTeam( WeichiMoveFeature moveFeature, bool bIsWinMove );
	void writeFeatureTeam( ostringstream& oss, vector<uint> vTeam, uint pos, bool bIsWinMove );

	// for move prediction
	void addUpUctMovePredictorStatistic( const WeichiMove& winMove, uint moveNumber );
	void addUpPlayoutMovePredictorStatistic( const WeichiMove& winMove, uint moveNumber );

	// for CNN rotate candidate
	void collectDCNNRotateCandidate( const vector<WeichiMove>& vMoves, uint moveNumber );

	// for DCNN log
	void logNetRecord();
	void logFTLPolicy();
	void setLADBitBoard(string sSgfPos, WeichiBitBoard& bmStone);
	void forwardNet();	
	vector<uint> randomMoveNumber();

	void writeDevelopmentCommand(SgfLoader& sgfLoader);		
	void setLifeDeathConfig(string comment) ;
	// for collecting human game opening
	void collectHumanGameOpening(SgfLoader& sgfLoader);
	void addRegionFromSplitBoard();			
	void mergeToRawSgf();
	void checkSgf();
	void generateJson();

	// for others tool
	void findSgfAttribute( SgfLoader& sgfLoader );
	bool isLowRankSgf( SgfLoader& sgfLoader );
	bool isHandicapSgf( SgfLoader& sgfLoader );	
	bool collectRepeatedSgf();
	map<string,string> m_LookupSgfs;
	map<string, string> m_maskSgfs;
};

#endif